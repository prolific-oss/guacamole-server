/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "encode.h"
#include "guacenc.h"
#include "log.h"
#include "parse.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum guacenc_option {
    GUACENC_OPTION_START_SYNC_INDEX = 0x100,
    GUACENC_OPTION_END_SYNC_INDEX,
    GUACENC_OPTION_TIMELINE_ORIGIN
};

int main(int argc, char* argv[]) {

    int i;

    /* Load defaults */
    bool force = false;
    int width = GUACENC_DEFAULT_WIDTH;
    int height = GUACENC_DEFAULT_HEIGHT;
    int bitrate = GUACENC_DEFAULT_BITRATE;

    /* Event window defaults */
    guacenc_window window = {
        .start_sync_index = 0,
        .end_sync_index = UINT64_MAX,
        .timeline_origin = 0
    };
    bool start_sync_index_set = false;
    bool end_sync_index_set = false;
    bool timeline_origin_set = false;

    static const struct option long_options[] = {
        {"start-sync-index", required_argument, NULL,
            GUACENC_OPTION_START_SYNC_INDEX},
        {"end-sync-index", required_argument, NULL,
            GUACENC_OPTION_END_SYNC_INDEX},
        {"timeline-origin", required_argument, NULL,
            GUACENC_OPTION_TIMELINE_ORIGIN},
        {NULL, 0, NULL, 0}
    };

    /* Parse arguments */
    int opt;
    while ((opt = getopt_long(argc, argv, "s:r:f",
                    long_options, NULL)) != -1) {

        /* -s: Dimensions (WIDTHxHEIGHT) */
        if (opt == 's') {
            if (guacenc_parse_dimensions(optarg, &width, &height)) {
                guacenc_log(GUAC_LOG_ERROR, "Invalid dimensions.");
                goto invalid_options;
            }
        }

        /* -r: Bitrate (bits per second) */
        else if (opt == 'r') {
            if (guacenc_parse_int(optarg, &bitrate)) {
                guacenc_log(GUAC_LOG_ERROR, "Invalid bitrate.");
                goto invalid_options;
            }
        }

        /* -f: Force */
        else if (opt == 'f')
            force = true;

        /* --start-sync-index: First accepted display sync event to encode */
        else if (opt == GUACENC_OPTION_START_SYNC_INDEX) {
            if (guacenc_parse_uint64(optarg, &window.start_sync_index)) {
                guacenc_log(GUAC_LOG_ERROR, "Invalid start sync index.");
                goto invalid_options;
            }
            start_sync_index_set = true;
        }

        /* --end-sync-index: First accepted display sync event to omit */
        else if (opt == GUACENC_OPTION_END_SYNC_INDEX) {
            if (guacenc_parse_uint64(optarg, &window.end_sync_index)) {
                guacenc_log(GUAC_LOG_ERROR, "Invalid end sync index.");
                goto invalid_options;
            }
            end_sync_index_set = true;
        }

        /* --timeline-origin: First accepted display sync timestamp */
        else if (opt == GUACENC_OPTION_TIMELINE_ORIGIN) {
            if (guacenc_parse_nonnegative_timestamp(optarg,
                        &window.timeline_origin)) {
                guacenc_log(GUAC_LOG_ERROR, "Invalid timeline origin.");
                goto invalid_options;
            }
            timeline_origin_set = true;
        }

        /* Invalid option */
        else {
            goto invalid_options;
        }

    }

    /* Validate optional event window */
    bool window_requested = start_sync_index_set
                          || end_sync_index_set
                          || timeline_origin_set;
    if (window_requested && !timeline_origin_set) {
        guacenc_log(GUAC_LOG_ERROR, "--timeline-origin is required when "
                "encoding an event window.");
        goto invalid_options;
    }
    if (timeline_origin_set
            && !start_sync_index_set
            && !end_sync_index_set) {
        guacenc_log(GUAC_LOG_ERROR, "--timeline-origin requires "
                "--start-sync-index or --end-sync-index.");
        goto invalid_options;
    }
    if (window.end_sync_index <= window.start_sync_index) {
        guacenc_log(GUAC_LOG_ERROR, "End sync index must be greater than "
                "start sync index.");
        goto invalid_options;
    }

    /* Log start */
    guacenc_log(GUAC_LOG_INFO, "Guacamole video encoder (guacenc) "
            "version " VERSION);

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 10, 100)
    /* Prepare libavcodec */
    avcodec_register_all();
#endif

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif

    /* Track number of overall failures */
    int total_files = argc - optind;
    int failures = 0;

    /* Abort if no files given */
    if (total_files <= 0) {
        guacenc_log(GUAC_LOG_INFO, "No input files specified. Nothing to do.");
        return 0;
    }

    guacenc_log(GUAC_LOG_INFO, "%i input file(s) provided.", total_files);

    guacenc_log(GUAC_LOG_INFO, "Video will be encoded at %ix%i "
            "and %i bps.", width, height, bitrate);

    if (window_requested)
        guacenc_log(GUAC_LOG_INFO, "Encoding display sync events "
                "[%" PRIu64 ", %" PRIu64 ") on timeline origin %" PRId64 ".",
                window.start_sync_index, window.end_sync_index,
                (int64_t) window.timeline_origin);

    /* Encode all input files */
    for (i = optind; i < argc; i++) {

        /* Get current filename */
        const char* path = argv[i];

        /* Generate output filename */
        char out_path[4096];
        int len = snprintf(out_path, sizeof(out_path), "%s.m4v", path);

        /* Do not write if filename exceeds maximum length */
        if (len >= sizeof(out_path)) {
            guacenc_log(GUAC_LOG_ERROR, "Cannot write output file for \"%s\": "
                    "Name too long", path);
            continue;
        }

        /* Attempt encoding, log granular success/failure at debug level */
        if (guacenc_encode(path, out_path, "mpeg4",
                    width, height, bitrate, force,
                    window_requested ? &window : NULL)) {
            failures++;
            guacenc_log(GUAC_LOG_DEBUG,
                    "%s was NOT successfully encoded.", path);
        }
        else
            guacenc_log(GUAC_LOG_DEBUG, "%s was successfully encoded.", path);

    }

    /* Warn if at least one file failed */
    if (failures != 0)
        guacenc_log(GUAC_LOG_WARNING, "Encoding failed for %i of %i file(s).",
                failures, total_files);

    /* Notify of success */
    else
        guacenc_log(GUAC_LOG_INFO, "All files encoded successfully.");

    /* Encoding complete */
    return 0;

    /* Display usage and exit with error if options are invalid */
invalid_options:

    fprintf(stderr, "USAGE: %s"
            " [-s WIDTHxHEIGHT]"
            " [-r BITRATE]"
            " [-f]"
            " [--start-sync-index INDEX]"
            " [--end-sync-index INDEX]"
            " [--timeline-origin TIMESTAMP]"
            " [FILE]...\n", argv[0]);

    return 1;

}
