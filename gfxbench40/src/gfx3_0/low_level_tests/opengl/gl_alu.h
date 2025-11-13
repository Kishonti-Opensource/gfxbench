/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "alu_base.h"
#include "kcl_base.h"
#include "kcl_math3d.h"

class ALUTest : public ALUTest_Base
{
private:
	static const int m_stride = 5 * sizeof(float); // distance between the beginning of vertices
	KCL::uint32 m_vertexBuffer;
	KCL::uint32 m_indexBuffer;
	KCL::uint32 m_shader;
	KCL::Vector2D m_aspectRatio;

    bool m_isRotated;

	int m_uniTimeLocation;
	int m_uniAspectRatio;
	int m_uniLightDirLocation;
	int m_uniPositionLocation;
	int m_uniOrientationLocation;

public:
	ALUTest(const GlobalTestEnvironment* const gte);
	virtual ~ALUTest();

protected:
	virtual const char* getUom() const { return "frames"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	
	virtual KCL::KCL_Status init ();
	virtual bool render ();
	
	virtual void FreeResources();

	bool m_vectorized;
};

class ALUTestVectorized : public ALUTest
{
public:
	ALUTestVectorized(const GlobalTestEnvironment* const gte) :ALUTest(gte) { ALUTest::m_vectorized = true; };
};
