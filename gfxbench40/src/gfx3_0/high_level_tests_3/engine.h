/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TEST2_H
#define TEST2_H

#include "test_base.h"

class GlobatTestEnviroment;
class KRL_Scene;

class Engine2 : public GLB::TestBase
{
public:
	Engine2 (const GlobalTestEnvironment* const gte);

	virtual ~Engine2 ();

	void FreeResources();

	inline bool isWarmup() const 
	{ 
		return false;
	}

	const char* getUom() const 
	{
		if(m_settings->m_frame_step_time > -1)
		{
			return "msec";
		}
		return "frames";
	}

	virtual void onCheckboxChanged(int id, bool state);
    virtual void onSliderChanged(int id, float value);

	virtual KCL::KCL_Status init ();

	virtual bool animate (const int time);

	virtual bool render ();

	virtual float getScore () const;

	//virtual float getFrames () const;

	virtual KCL::uint32 indexCount() const;

	virtual bool isLowLevel() const;

	virtual bool resize(int width, int height);

	virtual KCL::uint32 getRenderedVerticesCount() const;

	virtual KCL::uint32 getRenderedTrianglesCount() const;

	virtual KCL::uint32 getDrawCalls() const;

	virtual KCL::int32 getUsedTextureCount() const;

	virtual KCL::int32 getSamplesPassed() const;

    virtual float getPixelCoverage() const;

	virtual KCL::int32 getInstructionCount() const;

    int updateInput(const int time);
	
	KRL_Scene *m_scene;
};

#endif

