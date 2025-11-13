/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DX_DRIVER_H
#define DX_DRIVER_H

#include "new_low_level_base_dx.h"

class CPUOverheadTest : public LowLevelBase
{
private:
	__declspec(align(16)) struct ConstantBuffer
	{
		KCL::Vector2D Position;
		KCL::Vector2D Scale;
		KCL::Vector2D ScreenResolution;
		KCL::Vector2D MatrixSize;

		KCL::Vector4D Color0;
		KCL::Vector4D Color1;
		KCL::Vector4D Color2;
		KCL::Vector4D Color3;

		KCL::Vector4D Rotation;
	};

	static const int BufferCount = 4;
	KCL::ConstantBuffer* m_constantBuffer;
	KCL::VertexBuffer* m_vertexBuffers[BufferCount];
	KCL::IndexBuffer* m_indexBuffers[BufferCount];

	int m_score;
	Shader* m_cpuShader;

public:
	CPUOverheadTest(const GlobalTestEnvironment* const gte);
	~CPUOverheadTest();

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