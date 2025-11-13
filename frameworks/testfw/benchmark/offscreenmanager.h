/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef OFFSCREEN_MANAGER_H_
#define OFFSCREEN_MANAGER_H_

#include <vector>
#include <string>
#include <stdlib.h>
#include <stdint.h>

class OffscreenManager
{
public:
    static const unsigned int SWAP_BUFFER_INTERVAL = 5000;

    static const unsigned int OFFSCR_RENDER_TIMER = 100;
    static const unsigned int SAMPLE_W = 8;
    static const unsigned int SAMPLE_H = 8;
    static const unsigned int SAMPLE_NUM_X = 8;
    static const unsigned int SAMPLE_NUM_Y = 8;
    static const unsigned int SAMPLE_C = SAMPLE_NUM_X * SAMPLE_NUM_Y;
    static const unsigned int MOSAIC_WIDTH = SAMPLE_NUM_X * SAMPLE_W;
    static const unsigned int MOSAIC_HEIGHT = SAMPLE_NUM_Y * SAMPLE_H;
    static const unsigned int SCRATCH_MIN_DIMSZ = 64;
    static const unsigned int SCRATCH_WIDTH = SAMPLE_W < SCRATCH_MIN_DIMSZ ? SCRATCH_MIN_DIMSZ : SAMPLE_W;
    static const unsigned int SCRATCH_HEIGHT = SAMPLE_H < SCRATCH_MIN_DIMSZ ? SCRATCH_MIN_DIMSZ : SAMPLE_H;

    //these will go onscreen
    static const unsigned int ONSCR_SAMPLE_W = 48; //480270, 100 * 4827 sample
    static const unsigned int ONSCR_SAMPLE_H = 27;
    static const unsigned int ONSCR_SAMPLE_NUM_X = 10;
    static const unsigned int ONSCR_SAMPLE_NUM_Y = 10;
    static const unsigned int ONSCR_SAMPLE_C = ONSCR_SAMPLE_NUM_X * ONSCR_SAMPLE_NUM_Y;
    static const unsigned int ONSCR_MOSAIC_WIDTH = ONSCR_SAMPLE_NUM_X * ONSCR_SAMPLE_W;
    static const unsigned int ONSCR_MOSAIC_HEIGHT = ONSCR_SAMPLE_NUM_Y * ONSCR_SAMPLE_H;

    static const unsigned int FINISH_FBO_SIZE = 1;

    enum ManagerMode
    {
        OnscreenMode = 0,
        OffscreenMode,
    };

    static OffscreenManager* Create();
    virtual ~OffscreenManager();

    int Init( ManagerMode mode, unsigned int onscreen_width, unsigned int onscreen_height, unsigned int play_time, unsigned int color_bpp, unsigned int depth_bpp, unsigned int default_fbo);
    void PreRender() const;
    bool PostRender(unsigned int time, unsigned int frame, int current_viewport_width, int current_viewport_height);
    void Clear();

    virtual bool RenderLastFrames(int current_viewport_width, int current_viewport_height);
    virtual void FinishRendering();
    unsigned int GetDefaultFBO() const
    {
        if (m_mode == OnscreenMode)
            return m_globalFBO;
        else
            return m_offscreen_fbo[m_current_fbo_index]->m_fbo;
    }
    bool SwapBufferNeeded()
    {
        return m_swap_buffer_needed;
    }

    int32_t width() const;
    int32_t height() const;

private:
    class FBO
    {
    public:
        enum ColorMode
        {
            RGB888_Linear,
            RGB565_Linear
        };
        enum DepthMode
        {
            DEPTH_NONE = 0,
            DEPTH_16_RB,
            DEPTH_24_RB
        };

        FBO(unsigned int width, unsigned int height, ColorMode color_mode, DepthMode depth_mode, const char *m_debug_label = NULL);
        ~FBO();

        unsigned int m_fbo;
        unsigned int m_color_texture;
        unsigned int m_depth_renderbuffer;
        unsigned int m_width;
        unsigned int m_height;
        std::string m_debug_label;
    };

    OffscreenManager();
    OffscreenManager(const OffscreenManager &);
    OffscreenManager& operator=(const OffscreenManager &);

    enum TextureCoordinatesMode { USE_MOSAIC = 0, USE_SCRATCH, USE_FULL, USE_FULL_NEAREST };

    virtual void createOffscrProg();
    unsigned int initShader(unsigned int shaderType, const char *shaderSource);
    void InitCommon(const char *srcVert, const char *srcVert2, const char *srcFrag);
    void deleteOffscrProg();
    virtual void renderRectWithTexture(unsigned int textureId, TextureCoordinatesMode textureCoordinatesMode) const; // use only in PostRender!!!
    void calcSampleTexCoords(size_t idx, float &X_L, float &Y_D, float &X_R, float &Y_U);

    void save_sample_time(int t)
    {
        m_sample_times[m_mosaic_idx] = t;
    }

    void clear_saved_sample_data();
    void setMosaicViewPort() const;

    void renderToScratch() const;               // use only in PostRender!!!
    void renderScratchToMosaic() const;         // use only in PostRender!!!
    void renderScratchToBackScreen() const;     // use only in PostRender!!!

    void renderToOnscrMosaic();                 // use only in PostRender!!!
    void renderOnscrMosaicToBackScreen(const bool isRotated) const; // use only in PostRender!!!
    void renderOffscrToBackScreen(const bool isRotated) const;
    void setOnscrSampleViewPort() const;
    void setOnscrMosaicViewPort(const bool isRotated) const;
    void init_onscr_mosaic_coords();

    ManagerMode m_mode;

    bool m_virtual_resolution;

    bool m_swap_buffer_needed;

    int m_onscreen_width;
    int m_onscreen_height;

    int m_width;
    int m_height;
    unsigned int m_globalFBO;

    FBO *m_offscreen_fbo[2];
    FBO *m_scratch_fbo[2];
    int m_current_fbo_index;
    FBO *m_mosaic_fbo;

    FBO *m_finish_fbo;

    unsigned int m_last_swap_time;

    unsigned int m_offscreen_vertex_shader;
    unsigned int m_offscreen_fragment_shader;
    unsigned int m_offscreen_program;
    unsigned int m_offscreen_vbo;
    unsigned int m_offscreen_ebo;
    int m_offscr_text_unif_loc;

    unsigned int m_next_slot_time_interval; // when the delta time is GEQ to this, we render scratch into mosaic, save current time, and increment mosaic_idx
    unsigned int m_next_slot_previous_time; // the previous time when we rendered scratch into mosaic
    unsigned int m_mosaic_idx;

    unsigned int m_sample_times[SAMPLE_C]; // (this is used for save)
    unsigned int m_sample_coords_x[SAMPLE_C]; // x coord of i. sample in the offscreen fbo (this is used for save)
    unsigned int m_sample_coords_y[SAMPLE_C]; // y coord of i. sample in the offscreen fbo (this is used for save)
    unsigned int m_mosaic_coords_x[SAMPLE_C]; // x coord for where to put i. sample in mosaic (setting viewport) AND from where to read from mosaic (this is used for save)
    unsigned int m_mosaic_coords_y[SAMPLE_C]; // y coord for where to put i. sample in mosaic (setting viewport) AND from where to read from mosaic (this is used for save)

    FBO *m_onscr_mosaic_fbo;
    unsigned int m_onscr_mosaic_idx;
    unsigned int m_onscr_mosaic_x[2]; // x coord for where to put onscreen mosaic in back buffer  (setting viewport)
    unsigned int m_onscr_mosaic_y[2]; // y coord for where to put onscreen mosaic in back buffer  (setting viewport)
    unsigned int m_onscr_mosaic_coords_x[ONSCR_SAMPLE_C]; // x coord for where to put i. onscreen sample in onscreen mosaic (setting viewport)
    unsigned int m_onscr_mosaic_coords_y[ONSCR_SAMPLE_C]; // y coord for where to put i. onscreen sample in onscreen mosaic (setting viewport)

    unsigned int m_onscr_mosaic_program[2];
    unsigned int m_onscr_mosaic_text_unif_loc[2];
    unsigned int m_onscr_mosaic_viewport_width [2];
    unsigned int m_onscr_mosaic_viewport_height[2];

    unsigned int m_last_refresh_msec;
    unsigned int m_refresh_msec;

    virtual bool PostRender_original(unsigned int time, unsigned int frame, int current_viewport_width, int current_viewport_height);

};

#endif
