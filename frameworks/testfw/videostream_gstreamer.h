/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "videostream.h"
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

namespace tfw
{

class GStreamerVideoStream : public VideoStream
{
public:
    GStreamerVideoStream();
    ~GStreamerVideoStream();
    void openFile(const std::string &file);
    void openPipeline(const std::string &pipeline);
    void setLoopEnabled(bool loop);
    void close();
    void updateTexImage() override;
    std::string decoderString() const override;
private:
    void connectAppSrc();
    void connectDecodeBin();
    void connectAppSink();
    void reset();
    void setState(GstState state);
    void processCaps(GstCaps *caps);
    static void needData(GstAppSrc *src, guint length, gpointer userData);
    static void enoughData(GstAppSrc *src, gpointer userData);
    static gboolean seekData(GstAppSrc *src, guint64 offset, gpointer userData);
    static void onElementAdded(GstElement *element, GstElement *added, gpointer userData);
    GstElement *pipeline_;
    GstElement *vsrc_;
    GstElement *vsink_;
    uint32_t tex_[3];
    bool loop_;
    std::string data_;
    guint offset_;
    std::vector<std::string> elements_;
};

}

