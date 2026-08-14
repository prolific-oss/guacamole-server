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

#include <assert.h>
#include <stdint.h>

uint64_t guacenc_video_frame_index(guac_timestamp origin,
        guac_timestamp timestamp) {

    /* Timestamps are validated by the display before reaching the encoder. */
    assert(timestamp >= origin);

    /*
     * Split whole seconds from the millisecond remainder before multiplying.
     * This preserves exact rational FPS arithmetic without overflowing for
     * realistic long-running Guacamole timestamps.
     */
    uint64_t elapsed = (uint64_t) (timestamp - origin);
    return (elapsed / 1000) * GUACENC_VIDEO_FRAMERATE
        + (elapsed % 1000) * GUACENC_VIDEO_FRAMERATE / 1000;

}
