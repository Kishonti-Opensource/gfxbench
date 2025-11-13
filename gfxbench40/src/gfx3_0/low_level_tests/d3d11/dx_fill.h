/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DX_FILL_H
#define DX_FILL_H

#include "new_low_level_base_dx.h"

class CompressedFillTest : public LowLevelBase
{
private:
	__declspec(align(16)) struct ConstantBuffer
	{
		float AspectRatio;
		float Rotation;
	};

	KCL::ConstantBuffer* m_constantBuffer;
	KCL::Texture *m_colorTexture;
	KCL::Texture *m_lightTexture;
	Shader* m_fillShader;
	int m_score;

	double m_transferredTexels;
	int m_displayedElementCount;
	int m_elementCountStep;
	int m_testStage;

public:
	CompressedFillTest(const GlobalTestEnvironment* const gte);
	~CompressedFillTest();

	/* override */ virtual const char* getUom() const { return "MTexels/sec"; }
	/* override */ virtual bool isWarmup() const { return false; }
	/* override */ virtual KCL::uint32 indexCount() const { return 0; }
	/* override */ virtual float getScore() const { return m_score; }

	/* override */ virtual KCL::KCL_Status init();
	/* override */ virtual bool animate(const int time);
	/* override */ virtual void resetEnv();
	/* override */ virtual bool render();
	/* override */ virtual void FreeResources();
};

#endif