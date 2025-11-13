/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define NOMINMAX
#include "videostream_gstreamer.h"
#include <fstream>
#include <memory>
#include <algorithm>
#include <gst/app/gstappsink.h>
#include <gst/gstversion.h>
#ifdef HAVE_EPOXY
  #include <epoxy/egl.h>
  #include <epoxy/gl.h>
  #ifdef _WIN32
    #include <epoxy/wgl.h>
  #elif defined HAVE_X11
    #include <epoxy/glx.h>
  #endif
#else
  #ifdef _WIN32
    #include <Windows.h>
    #include <gl/GL.h>
    #pragma comment(lib, "opengl32.lib")
  #else
    #include <EGL/egl.h>
    #include <GLES2/gl2.h>
  #endif
#endif
#include "ng/require.h"
#include "ng/macro_utils.h"

NG_TABLE_START(TBL_STATE_CHANGE_RET)
    NG_TABLE_ITEM0(GST_STATE_CHANGE_FAILURE)
    NG_TABLE_ITEM0(GST_STATE_CHANGE_SUCCESS)
    NG_TABLE_ITEM0(GST_STATE_CHANGE_ASYNC)
    NG_TABLE_ITEM0(GST_STATE_CHANGE_NO_PREROLL)
NG_TABLE_END(TBL_STATE_CHANGE_RET)

NG_TABLE_START(TBL_STATE)
    NG_TABLE_ITEM0(GST_STATE_VOID_PENDING)
    NG_TABLE_ITEM0(GST_STATE_NULL)
    NG_TABLE_ITEM0(GST_STATE_READY)
    NG_TABLE_ITEM0(GST_STATE_PAUSED)
    NG_TABLE_ITEM0(GST_STATE_PLAYING)
NG_TABLE_END(TBL_STATE)

NG_TABLE_START(TBL_STREAM_STATUS)
    NG_TABLE_ITEM0(GST_STREAM_STATUS_TYPE_CREATE)
    NG_TABLE_ITEM0(GST_STREAM_STATUS_TYPE_ENTER)
    NG_TABLE_ITEM0(GST_STREAM_STATUS_TYPE_LEAVE)
    NG_TABLE_ITEM0(GST_STREAM_STATUS_TYPE_DESTROY)
    NG_TABLE_ITEM0(GST_STREAM_STATUS_TYPE_START)
    NG_TABLE_ITEM0(GST_STREAM_STATUS_TYPE_PAUSE)
    NG_TABLE_ITEM0(GST_STREAM_STATUS_TYPE_STOP)
NG_TABLE_END(TBL_STREAM_STATUS)

using namespace tfw;

namespace
{
    GLuint createTexture()
    {
        GLuint texId = 0;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        return texId;
    }



    void uploadGL(GLuint tex, int32_t width, int32_t height, GLenum format, const void *data)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    }



    void setElementState(GstElement *element, GstState request)
    {
        GstStateChangeReturn ret;
        (void)ret;
        GstState state, pending;
        ret = gst_element_get_state (element, &state, &pending, 5 * GST_SECOND);
        NGLOG_TRACE("pre: ret: %s, state: %s, pending: %s", TBL_STATE_CHANGE_RET(ret), TBL_STATE(state), TBL_STATE(pending));
        gst_element_set_state(element, request);
        ret = gst_element_get_state (element, &state, &pending, 5 * GST_SECOND);
        NGLOG_TRACE("post: ret: %s, state: %s, pending: %s", TBL_STATE_CHANGE_RET(ret), TBL_STATE(state), TBL_STATE(pending));
    }
}



gboolean bus_watch(GstBus *bus, GstMessage *msg, gpointer data)
{
    const gchar *name = gst_message_type_get_name(GST_MESSAGE_TYPE(msg));
    (void)name;
    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_ERROR:
            {
                GError *err = NULL;
                gchar *dbg_info = NULL;

                gst_message_parse_error (msg, &err, &dbg_info);
                NGLOG_ERROR ("msg-error: %s: %s, dbg: %s", GST_OBJECT_NAME (msg->src), err->message, (dbg_info) ? dbg_info : "none");
                g_error_free (err);
                g_free (dbg_info);
                break;
            }
        case GST_MESSAGE_BUFFERING:
            {
                gint percent = 0;
                gst_message_parse_buffering(msg, &percent);
                NGLOG_TRACE("msg-buffering: %s", percent);
                break;
            }
        case GST_MESSAGE_STATE_CHANGED:
            {
                GstState old_state, new_state;

                gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);
                NGLOG_TRACE ("msg-state-change: %s: from %s to %s",
                        GST_OBJECT_NAME (msg->src),
                        gst_element_state_get_name (old_state),
                        gst_element_state_get_name (new_state));
                break;
            }
        case GST_MESSAGE_TAG:
            {
                GstTagList *tags = NULL;

                gst_message_parse_tag (msg, &tags);
                gchar *str = gst_tag_list_to_string(tags);
                NGLOG_TRACE ("msg-tag: %s: %s", GST_OBJECT_NAME (msg->src), str);
                g_free(str);
                gst_tag_list_unref (tags);
                break;
            }
        case GST_MESSAGE_STREAM_STATUS:
            {
                GstStreamStatusType type;
                GstElement *owner;
                gst_message_parse_stream_status (msg, &type, &owner);
                NGLOG_TRACE("stream-status: %s: %s", TBL_STREAM_STATUS(type).c_str(), GST_OBJECT_NAME(owner));
                break;
            }
        default:
            NGLOG_TRACE("msg type: %s", name);
            break;
    }
    return false;
}



GStreamerVideoStream::GStreamerVideoStream()
: pipeline_(0)
, vsink_(0)
, loop_(false)
, offset_(0)
{
    tex_[0] = 0;
    tex_[1] = 0;
    float vfliptc[] = {
        1, 0,0,0,
        0,-1,0,0,
        0, 0,1,0,
        0, 1,0,1,
    };
    setTransformMatrix(vfliptc);
}



GStreamerVideoStream::~GStreamerVideoStream()
{
    close();
}



void GStreamerVideoStream::openFile(const std::string &file)
{
    std::ifstream stream(file.c_str(), std::ios::binary);
    assert(stream.is_open());
    std::ostringstream oss;
    oss << stream.rdbuf();
    data_ = oss.str();
    openPipeline("appsrc name=vsrc ! decodebin name=decoder ! appsink name=vsink sync=false");
}



void GStreamerVideoStream::openPipeline(const std::string &pipeline)
{
    NGLOG_TRACE("opening gstreamer: %s", pipeline);
    GError *err = NULL;
    gst_init(0, NULL);
	pipeline_ = gst_parse_launch (pipeline.c_str(), &err);
    if (err)
    {
        NGLOG_ERROR("GStreamer: %s", err->message);
        return;
    }
    requireex(pipeline_ != 0);

    GstBus *bus = gst_element_get_bus(pipeline_);
    gst_bus_add_watch(bus, bus_watch, this);
    connectAppSrc();
    connectDecodeBin();
    connectAppSink();

    tex_[0] = createTexture();
    tex_[1] = createTexture();
    tex_[2] = createTexture();
    setState(GST_STATE_PLAYING);
#if GST_CHECK_VERSION(1,0,0)
    std::unique_ptr<GstSample, void(*)(GstSample*)> sample(
            gst_app_sink_pull_preroll(GST_APP_SINK(vsink_)), &gst_sample_unref);
    processCaps(gst_sample_get_caps(sample.get()));
#else
    std::unique_ptr<GstBuffer, void(*)(GstBuffer*)> sample(
            gst_app_sink_pull_preroll(GST_APP_SINK(vsink_)), &gst_buffer_unref);
    processCaps(gst_buffer_get_caps(sample.get()));
#endif
}



void GStreamerVideoStream::connectAppSrc()
{
    vsrc_ = gst_bin_get_by_name(GST_BIN(pipeline_), "vsrc");
    requireex(vsrc_ != 0);
    GstAppSrc *src = GST_APP_SRC(vsrc_);
    requireex(src != 0);
    static GstAppSrcCallbacks callbacks = { &needData, &enoughData, &seekData };
    gst_app_src_set_callbacks(src, &callbacks, this, nullptr);
    gst_app_src_set_stream_type(src, GST_APP_STREAM_TYPE_RANDOM_ACCESS);
}



void GStreamerVideoStream::needData(GstAppSrc *src, guint length, gpointer userData)
{
    GStreamerVideoStream *self = static_cast<GStreamerVideoStream*>(userData);
    guint maxSize = (guint)self->data_.size() - self->offset_;
    guint size = std::min(length, maxSize);

    GstBuffer *buffer = gst_buffer_new_wrapped_full(GST_MEMORY_FLAG_READONLY,
            const_cast<char*>(self->data_.c_str()), self->data_.size(),
            self->offset_, size, nullptr, nullptr);
    self->offset_ += size;
    gst_app_src_push_buffer(src, buffer);
    if (size == maxSize) {
        gst_app_src_end_of_stream(src);
    }
}



void GStreamerVideoStream::enoughData(GstAppSrc *src, gpointer userData)
{
}



gboolean GStreamerVideoStream::seekData(GstAppSrc *src, guint64 offset, gpointer userData)
{
    GStreamerVideoStream *self = static_cast<GStreamerVideoStream*>(userData);
    self->offset_ = static_cast<guint>(offset);
    return TRUE;
}



void GStreamerVideoStream::onElementAdded(GstElement *element, GstElement *added, gpointer userData)
{
    GStreamerVideoStream *self = static_cast<GStreamerVideoStream*>(userData);
    std::shared_ptr<char> name(gst_element_get_name(added), &g_free);
    self->elements_.push_back(name.get());
}



void GStreamerVideoStream::connectDecodeBin()
{
    GstElement* decoder = gst_bin_get_by_name(GST_BIN(pipeline_), "decoder");
    g_signal_connect(decoder, "element-added", G_CALLBACK(onElementAdded), this);
}



void GStreamerVideoStream::connectAppSink()
{
    vsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "vsink");
    gst_app_sink_set_max_buffers(GST_APP_SINK(vsink_), 1);
    requireex(vsink_ != 0);
    GstStructure *nv12 = gst_structure_new("video/x-raw",
        "format", G_TYPE_STRING, "NV12",
        "width", GST_TYPE_INT_RANGE, 64, 3840,
        "height", GST_TYPE_INT_RANGE, 64, 2160,
        NULL);
    GstStructure *i420 = gst_structure_new("video/x-raw",
        "format", G_TYPE_STRING, "I420",
        "width", GST_TYPE_INT_RANGE, 64, 3840,
        "height", GST_TYPE_INT_RANGE, 64, 2160,
        NULL);

    GstCaps *caps = gst_caps_new_full(nv12, i420, NULL);
    gst_app_sink_set_caps(GST_APP_SINK(vsink_), caps);
    NGLOG_TRACE("caps: %s is-fixed: %s", gst_caps_to_string(caps), gst_caps_is_fixed(caps));
    gst_caps_unref(caps);
}



void GStreamerVideoStream::close()
{
    if (pipeline_)
    {
        setState(GST_STATE_NULL);
        gst_object_unref (pipeline_);
        pipeline_ = NULL;
        vsink_ = NULL;
    }
    glDeleteTextures(3, tex_);
}



void GStreamerVideoStream::setLoopEnabled(bool loop)
{
    loop_ = loop;
}



void GStreamerVideoStream::setState(GstState state)
{
    setElementState(pipeline_, state);
}



void GStreamerVideoStream::processCaps(GstCaps *caps) {
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    setFormat(gst_structure_get_string(structure, "format"));
    int width;
    gst_structure_get_int(structure, "width", &width);
    int height;
    gst_structure_get_int(structure, "height", &height);
    setSize(width, height);
}



void GStreamerVideoStream::reset()
{
    if (!gst_element_seek_simple(pipeline_,
                GST_FORMAT_TIME,
                GstSeekFlags(GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_KEY_UNIT),
                0))
    {
        NGLOG_WARN("Seek failed!\n");
    }
#if GST_CHECK_VERSION(1,0,0)
    GstSample *sample = gst_app_sink_pull_preroll(GST_APP_SINK(vsink_));
    if (sample) gst_sample_unref(sample);
#else
    GstBuffer *sample = gst_app_sink_pull_preroll(GST_APP_SINK(vsink_));
    if (sample) gst_buffer_unref(sample);
#endif
}



void GStreamerVideoStream::updateTexImage()
{
    GstBus *bus = gst_element_get_bus(pipeline_);
    GstMessage *msg = gst_bus_pop(bus);
    while (msg)
    {
        bus_watch(bus, msg, this);
        gst_message_unref(msg);
        msg = gst_bus_pop(bus);
    }
    if(gst_app_sink_is_eos(GST_APP_SINK(vsink_)))
    {
        NGLOG_TRACE("Stream got EOS");
        if (loop_)
        {
            reset();
        }
        else
        {
            return;
        }
    }
#if GST_CHECK_VERSION(1,0,0)
    std::unique_ptr<GstSample, void(*)(GstSample*)> sample(
            gst_app_sink_pull_sample(GST_APP_SINK(vsink_)), &gst_sample_unref);
    if (sample == nullptr) return;
    GstBuffer *buffer = gst_sample_get_buffer(sample.get());
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);
    uint8_t *data = map.data;
#else
    std::unique_ptr<GstBuffer, void(*)(GstBuffer*)> sample(
            gst_app_sink_pull_buffer(GST_APP_SINK(vsink_)), &gst_buffer_unref);
    if (sample == nullptr) return;
    uint8_t *data = buffer->data;
#endif

    //NGLOG_TRACE("uploading data for texture: Y: %s UV: %s (%sx%s)", tex_[0], tex_[1], width, height);
    if (format() == "I420")
    {
        glActiveTexture(GL_TEXTURE0);
        uploadGL(tex_[0], width(), height(), GL_LUMINANCE, data);
        glActiveTexture(GL_TEXTURE1);
        uploadGL(tex_[1], width()/2, height()/2, GL_LUMINANCE, data + width()*height());
        glActiveTexture(GL_TEXTURE2);
        uploadGL(tex_[2], width()/2, height()/2, GL_LUMINANCE, data + width()*height() + width()*height()/4);

    }
    else if (format() == "NV12")
    {
        glActiveTexture(GL_TEXTURE0);
        uploadGL(tex_[0], width(), height(), GL_LUMINANCE, data);
        glActiveTexture(GL_TEXTURE1);
        uploadGL(tex_[1], width()/2, height()/2, GL_LUMINANCE_ALPHA, data + width()*height());
    }
#if GST_CHECK_VERSION(1,0,0)
    gst_buffer_unmap (buffer, &map);
#endif
}



std::string GStreamerVideoStream::decoderString() const
{
    std::string result;
    if (elements_.empty()) {
        return result;
    }

    const char * const HARDWARE_DECODERS[] = { "vaapi", "vdpau", "dxva", "xvba", "vda", "omx" };
    size_t HARDWARE_DECODER_COUNT = sizeof(HARDWARE_DECODERS) / sizeof(HARDWARE_DECODERS[0]);

    bool hardwareDecode = false;
    result = elements_.front();
    for (size_t i = 1; i < elements_.size(); ++i) {
        result += " ! " + elements_.at(i);
        for (size_t j = 0; j < HARDWARE_DECODER_COUNT; ++j) {
            if (elements_.at(i).find(HARDWARE_DECODERS[j]) != std::string::npos) {
                hardwareDecode = true;
            }
        }
    }
    result += (hardwareDecode ? " (HARDWARE VIDEO DECODE)" : " (SOFTWARE VIDEO DECODE)");
    return result;
}
