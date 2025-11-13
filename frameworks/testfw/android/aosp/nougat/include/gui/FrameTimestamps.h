/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_GUI_FRAMETIMESTAMPS_H
#define ANDROID_GUI_FRAMETIMESTAMPS_H

#include <utils/Timers.h>
#include <utils/Flattenable.h>

namespace android {

struct FrameTimestamps : public LightFlattenablePod<FrameTimestamps> {
    FrameTimestamps() :
        frameNumber(0),
        postedTime(0),
        acquireTime(0),
        refreshStartTime(0),
        glCompositionDoneTime(0),
        displayRetireTime(0),
        releaseTime(0) {}

    uint64_t frameNumber;
    nsecs_t postedTime;
    nsecs_t acquireTime;
    nsecs_t refreshStartTime;
    nsecs_t glCompositionDoneTime;
    nsecs_t displayRetireTime;
    nsecs_t releaseTime;
};

} // namespace android
#endif
