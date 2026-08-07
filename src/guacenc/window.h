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

#ifndef GUACENC_WINDOW_H
#define GUACENC_WINDOW_H

#include <guacamole/timestamp.h>

#include <stdint.h>

/**
 * A half-open range of accepted display sync events to encode. Instructions
 * before the range are replayed to reconstruct display state, but do not
 * produce video frames. The event at end_sync_index is not included.
 */
typedef struct guacenc_window {

    /**
     * Zero-based index of the first accepted display sync event to encode.
     */
    uint64_t start_sync_index;

    /**
     * Zero-based index immediately after the last display sync event to
     * encode, or UINT64_MAX if the window extends through end-of-file.
     */
    uint64_t end_sync_index;

    /**
     * Timestamp of the first accepted display sync event in the complete
     * recording. All windows use this timestamp to preserve one global frame
     * grid.
     */
    guac_timestamp timeline_origin;

} guacenc_window;

#endif
