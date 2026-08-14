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

#ifndef GUACENC_VIDEO_TIMELINE_H
#define GUACENC_VIDEO_TIMELINE_H

#include <guacamole/timestamp.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * The framerate at which video should be encoded, in frames per second.
 */
#define GUACENC_VIDEO_FRAMERATE 30

/**
 * Returns the absolute CFR frame index corresponding to the given timestamp.
 * The calculation uses the shared timeline origin directly, avoiding the
 * cumulative truncation that results from repeatedly adding 1000 / FPS
 * milliseconds.
 *
 * @param origin
 *     The timestamp corresponding to frame zero.
 *
 * @param timestamp
 *     The timestamp to convert. This must not precede origin.
 *
 * @param frame_index
 *     Storage for the zero-based frame index containing timestamp.
 *
 * @return
 *     true if timestamp was valid and frame_index was written, false if
 *     timestamp precedes origin or frame_index is NULL.
 */
bool guacenc_video_frame_index(guac_timestamp origin,
        guac_timestamp timestamp, uint64_t* frame_index);

#endif
