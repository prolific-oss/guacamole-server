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

#include "video-timeline.h"

#include <CUnit/CUnit.h>

#include <stddef.h>
#include <stdint.h>

void test_video_timeline__uses_exact_rational_30_fps_boundaries() {

    /* Given a timeline whose timestamps do not fall on integer milliseconds. */
    guac_timestamp origin = 1000;

    /* When each timestamp is converted from the shared origin. */
    uint64_t first = guacenc_video_frame_index(origin, origin);
    uint64_t half_second = guacenc_video_frame_index(origin, 1500);
    uint64_t irregular = guacenc_video_frame_index(origin, 1545);
    uint64_t real_dump_end = guacenc_video_frame_index(origin, 340508);

    /* Then each result uses floor(elapsed * 30 / 1000) without accumulated loss. */
    CU_ASSERT_EQUAL(first, 0);
    CU_ASSERT_EQUAL(half_second, 15);
    CU_ASSERT_EQUAL(irregular, 16);
    CU_ASSERT_EQUAL(real_dump_end, 10185);

}

void test_video_timeline__is_independent_of_intermediate_event_cadence() {

    /* Given irregular events that previously accumulated truncated milliseconds. */
    const guac_timestamp timestamps[] = {
        1000, 1101, 1202, 1303, 1404, 1505, 1606, 1707, 1808, 1909, 2000
    };
    const size_t timestamp_count = sizeof(timestamps) / sizeof(timestamps[0]);

    /* When frame deltas are accumulated from absolute frame indices. */
    uint64_t accumulated_frames = 0;
    uint64_t previous_frame = 0;
    for (size_t index = 1; index < timestamp_count; index++) {
        uint64_t frame = guacenc_video_frame_index(timestamps[0],
                timestamps[index]);
        accumulated_frames += frame - previous_frame;
        previous_frame = frame;
    }

    /* Then the result equals the direct endpoint calculation. */
    CU_ASSERT_EQUAL(accumulated_frames,
            guacenc_video_frame_index(timestamps[0],
                timestamps[timestamp_count - 1]));
    CU_ASSERT_EQUAL(accumulated_frames, 30);

}

void test_video_timeline__preserves_frame_count_across_window_boundaries() {

    /* Given the shared origin and two boundaries from a split recording. */
    guac_timestamp origin = 1000;
    guac_timestamp boundary = 191988;
    guac_timestamp end = 340508;

    /* When each half-open window advances on the same absolute frame grid. */
    uint64_t first_window_frames =
        guacenc_video_frame_index(origin, boundary)
        - guacenc_video_frame_index(origin, origin);
    uint64_t final_window_frames =
        guacenc_video_frame_index(origin, end)
        - guacenc_video_frame_index(origin, boundary);
    uint64_t final_prepared_frame = 1;

    /* Then the windows contain exactly the monolithic frame count. */
    CU_ASSERT_EQUAL(first_window_frames + final_window_frames
            + final_prepared_frame,
            guacenc_video_frame_index(origin, end) + 1);
    CU_ASSERT_EQUAL(first_window_frames + final_window_frames
            + final_prepared_frame, 10186);

}
