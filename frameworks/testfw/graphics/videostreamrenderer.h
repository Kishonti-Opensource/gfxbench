/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VIDEOSTREAMRENDERER_H_
#define VIDEOSTREAMRENDERER_H_

#include <string>
#include <stdint.h>

class VideoStream;

namespace tfw
{


class VideoStreamRenderer
{
public:
    VideoStreamRenderer();
    void setupGL(VideoStream *stream);
    void drawGL(VideoStream *stream);
    void releaseGL();

private:
    std::string getInfoLog () const;
    void compileShader(uint32_t shader);
    uint32_t prog_, vs_, fs_;
};


}

#endif  // VIDEOSTREAMRENDERER_H_
