/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__OFFSCRMAN_H__
#define __XX__OFFSCRMAN_H__


#include <vector>
#include <stdlib.h>
#include <string>
#include <kcl_os.h>
#include "../gfxbench/global_test_environment.h"

namespace GLB
{
	class FBO;
}

enum OffscrMethod
{
	OM_ORIGINAL = 0,
	OM_HYBRID,
};


class TestDescriptor;

class OffscreenManager
{
public:
	static const KCL::uint32 OFFSCR_RENDER_TIMER = 100;
	static const KCL::uint32 SAMPLE_W = 8;
	static const KCL::uint32 SAMPLE_H = 8;
	static const KCL::uint32 SAMPLE_NUM_X = 8;
	static const KCL::uint32 SAMPLE_NUM_Y = 8;
	static const KCL::uint32 SAMPLE_C = SAMPLE_NUM_X * SAMPLE_NUM_Y;
	static const KCL::uint32 MOSAIC_WIDTH = SAMPLE_NUM_X * SAMPLE_W;
	static const KCL::uint32 MOSAIC_HEIGHT = SAMPLE_NUM_Y * SAMPLE_H;
	static const KCL::uint32 SCRATCH_MIN_DIMSZ = 64;
	static const KCL::uint32 SCRATCH_WIDTH = SAMPLE_W < SCRATCH_MIN_DIMSZ ? SCRATCH_MIN_DIMSZ : SAMPLE_W;
	static const KCL::uint32 SCRATCH_HEIGHT = SAMPLE_H < SCRATCH_MIN_DIMSZ ? SCRATCH_MIN_DIMSZ : SAMPLE_H;

	//these will go onscreen
	static const KCL::uint32 ONSCR_SAMPLE_W = 48; //480270, 100 * 4827 sample
	static const KCL::uint32 ONSCR_SAMPLE_H = 27;
	static const KCL::uint32 ONSCR_SAMPLE_NUM_X = 10;
	static const KCL::uint32 ONSCR_SAMPLE_NUM_Y = 10;
	static const KCL::uint32 ONSCR_SAMPLE_C = ONSCR_SAMPLE_NUM_X * ONSCR_SAMPLE_NUM_Y;
	static const KCL::uint32 ONSCR_MOSAIC_WIDTH = ONSCR_SAMPLE_NUM_X * ONSCR_SAMPLE_W;
	static const KCL::uint32 ONSCR_MOSAIC_HEIGHT = ONSCR_SAMPLE_NUM_Y * ONSCR_SAMPLE_H;

	static OffscreenManager* Create(const GlobalTestEnvironment* const gte, int w, int h);
	virtual ~OffscreenManager();

	int Init( unsigned int onscreen_width, unsigned int onscreen_height, const TestDescriptor &td);
	void PreRender() const;
	void PostRender(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer);
	void Clear();
	void ResetGlobalFBO(int current_viewport_width, int current_viewport_height) const;
	void SaveForTesting() const;

    virtual void RenderCurrentMosaic();
    virtual void RenderLastFrames(int current_viewport_width, int current_viewport_height);
	bool IsSwapBufferNeeded()
	{
		return m_onscr_mosaic_idx == ONSCR_SAMPLE_C;
	}

protected:
	OffscreenManager( int w, int h);
	OffscreenManager(const OffscreenManager &);
	OffscreenManager& operator=(const OffscreenManager &);

	enum TextureCoordinatesMode { USE_MOSAIC = 0, USE_SCRATCH, USE_FULL, USE_FULL_NEAREST };

	virtual void createOffscrProg();
    void InitCommon(const char *srcVert, const char *srcVert2, const char *srcFrag);
	void deleteOffscrProg();
	virtual void renderRectWithTexture(KCL::uint32 textureId, TextureCoordinatesMode textureCoordinatesMode) const; // use only in PostRender!!!
	void calcSampleTexCoords(size_t idx, float &X_L, float &Y_D, float &X_R, float &Y_U);
	
	void save_sample_time(int t)
	{
		m_sample_times[m_mosaic_idx] = t;
	}

	void clear_saved_sample_data();
	void setMosaicViewPort() const;

	void renderToScratch() const;		        // use only in PostRender!!!
	void renderScratchToMosaic() const;	        // use only in PostRender!!!
	void renderScratchToBackScreen() const;     // use only in PostRender!!!

	void renderToOnscrMosaic();				    // use only in PostRender!!!
	void renderOnscrMosaicToBackScreen(const bool isRotated) const; // use only in PostRender!!!
    void renderOffscrToBackScreen(const bool isRotated) const;
	void setOnscrSampleViewPort() const;
	void setOnscrMosaicViewPort(const bool isRotated) const;
	void init_onscr_mosaic_coords();

    bool m_virtual_resolution;

    int m_onscreen_width;
    int m_onscreen_height;

	int m_width;
	int m_height;
	GLB::FBO* m_globalFBO;

	GLB::FBO *m_offscreen_fbo[2];
	GLB::FBO *m_scratch_fbo[2];
	int m_current_fbo_index;
	GLB::FBO *m_mosaic_fbo;

	KCL::uint32 m_offscreen_vertex_shader;
	KCL::uint32 m_offscreen_fragment_shader;
	KCL::uint32 m_offscreen_program;
	KCL::uint32 m_offscreen_vbo;
	KCL::uint32 m_offscreen_ebo;
	int m_offscr_text_unif_loc;
	
	KCL::uint32 m_next_slot_time_interval; // when the delta time is GEQ to this, we render scratch into mosaic, save current time, and increment mosaic_idx
	KCL::uint32 m_next_slot_previous_time; // the previous time when we rendered scratch into mosaic
	KCL::uint32 m_mosaic_idx;

	KCL::uint32 m_sample_times[SAMPLE_C]; // (this is used for save)
	KCL::uint32 m_sample_coords_x[SAMPLE_C]; // x coord of i. sample in the offscreen fbo (this is used for save)
	KCL::uint32 m_sample_coords_y[SAMPLE_C]; // y coord of i. sample in the offscreen fbo (this is used for save)
	KCL::uint32 m_mosaic_coords_x[SAMPLE_C]; // x coord for where to put i. sample in mosaic (setting viewport) AND from where to read from mosaic (this is used for save)
	KCL::uint32 m_mosaic_coords_y[SAMPLE_C]; // y coord for where to put i. sample in mosaic (setting viewport) AND from where to read from mosaic (this is used for save)

	GLB::FBO *m_onscr_mosaic_fbo;
	KCL::uint32 m_onscr_mosaic_idx;
	KCL::uint32 m_onscr_mosaic_x[2]; // x coord for where to put onscreen mosaic in back buffer  (setting viewport)
	KCL::uint32 m_onscr_mosaic_y[2]; // y coord for where to put onscreen mosaic in back buffer  (setting viewport)
	KCL::uint32 m_onscr_mosaic_coords_x[ONSCR_SAMPLE_C]; // x coord for where to put i. onscreen sample in onscreen mosaic (setting viewport)
	KCL::uint32 m_onscr_mosaic_coords_y[ONSCR_SAMPLE_C]; // y coord for where to put i. onscreen sample in onscreen mosaic (setting viewport)
	
	KCL::uint32 m_onscr_mosaic_program[2];
	KCL::uint32 m_onscr_mosaic_text_unif_loc[2];
	KCL::uint32 m_onscr_mosaic_viewport_width [2];
	KCL::uint32 m_onscr_mosaic_viewport_height[2];
	
	KCL::uint32 m_hybrid_onscreen_width;
	KCL::uint32 m_hybrid_onscreen_height;

	int m_offscreen_default_viewport_width;
	int m_offscreen_default_viewport_height;

	OffscrMethod m_method;
	KCL::uint32 m_last_refresh_msec;
	KCL::uint32 m_refresh_msec;

	virtual void PostRender_original(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer);
	virtual void PostRender_hybrid(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer);

};

class OffscreenManager_CoreProfileVAO : public OffscreenManager
{
	friend class OffscreenManager;
public:
    /*virtual*/ ~OffscreenManager_CoreProfileVAO();
    /*virtual*/ void RenderCurrentMosaic();
    /*virtual*/ void RenderLastFrames(int current_viewport_width, int current_viewport_height);
protected:
    OffscreenManager_CoreProfileVAO( int w, int h);
    /*virtual*/ void createOffscrProg();

    /*virtual*/ void renderRectWithTexture(KCL::uint32 textureId, TextureCoordinatesMode textureCoordinatesMode) const; // use only in PostRender!!!
    /*virtual*/ void PostRender_original(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer);
    /*virtual*/ /*no impl*/ void PostRender_hybrid(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer) {}

    KCL::uint32 m_VAOs[2+SAMPLE_C]; //one VAO for each mosaic piece, plus 2 extra ones
};

#endif //__XX__OFFSCRMAN_H__
