/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DX_ALU_H
#define DX_ALU_H

#include "new_low_level_base_dx.h"

class ALUTest : public LowLevelBase
{
private:
	__declspec(align(16)) struct ConstantBuffer
	{
		KCL::Matrix4x4 Orientation;
		KCL::Vector3D EyePosition;
		float AspectRatio;
		KCL::Vector3D LightDir;
		float Time;
	};

	KCL::ConstantBuffer* m_constantBuffer;
	Shader* m_aluShader;
	KCL::uint32 m_score;

public:
	ALUTest(const GlobalTestEnvironment* const gte);
	~ALUTest();

	/* override */ virtual const char* getUom() const { return "frames"; }
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