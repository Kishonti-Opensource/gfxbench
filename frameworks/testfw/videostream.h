/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VIDEOSTREAM_H_
#define VIDEOSTREAM_H_

#include "ng/log.h"
#include <string>


class VideoStream
{
public:
    VideoStream()
    : width_(0)
    , height_(0)
    {
        static const float identity[] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        for(int i = 0; i < 16; ++i) texmat_[i] = identity[i];
    }
    virtual ~VideoStream() { NGLOG_INFO("VideoStream destructor"); }
    virtual void updateTexImage() { NGLOG_INFO("empty updateTexImage impl"); }
    virtual std::string decoderString() const { return ""; }
    void setTransformMatrix(float mtx[16]) { memcpy(texmat_, mtx, sizeof(texmat_)); }
    const float *transformMatrix() const { return texmat_; }
    std::string name() { return name_; }
    void setName(const std::string &name) { name_ = name; }
    void setSize(int width, int height) { width_ = width, height_ = height; }
    int width() { return width_; }
    int height() { return height_; }
    std::string format() const { return format_; }
    void setFormat(const std::string &format) { format_ = format; }
private:
    float texmat_[16];
    std::string name_;
    std::string format_;
    int width_;
    int height_;
};

#endif  // VIDEOSTREAM_H_
