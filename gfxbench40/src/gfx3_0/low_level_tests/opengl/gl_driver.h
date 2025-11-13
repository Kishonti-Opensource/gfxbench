/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "driver_base.h"
#include "kcl_base.h"


class CPUOverheadTest : public CPUOverheadTest_Base
{
private:
	static const int m_stride = 5 * sizeof(float); // distance between the beginning of vertices
	static const int BufferCount = 4;
	KCL::uint32 m_vertexBuffers[BufferCount];
	KCL::uint32 m_indexBuffers[BufferCount];

	float m_screenWidth;
	float m_screenHeight;

	KCL::uint32 m_shader;
	int m_uniRoration;
	int m_uniPosition;
	int m_uniMatrixSize;
	int m_uniScale;
	int m_uniColor0;
	int m_uniColor1;
	int m_uniColor2;
	int m_uniColor3;
	int m_uniScreenResolution;

public:
	CPUOverheadTest(const GlobalTestEnvironment* const gte);
	~CPUOverheadTest();

protected:
	
	virtual KCL::KCL_Status init ();
	virtual bool render ();
	
	virtual void FreeResources();
};
