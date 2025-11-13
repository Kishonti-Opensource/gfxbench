/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dx_alu.h"

ALUTest::ALUTest(const GlobalTestEnvironment* const gte) :
LowLevelBase(gte),
m_aluShader(0),
m_score(0)
{
}

ALUTest::~ALUTest()
{
	FreeResources();
}

bool ALUTest::animate(const int time)
{
	float sint = 0.75f + cosf(time * 0.0031415926535897932384626433832795f) * 0.25f;
	m_clearColor.set(sint, 0, 0);

	SetAnimationTime(time);
	if (m_frames > m_score)
	{
		m_score = m_frames;
	}

	return time < m_settings->m_play_time;
}

KCL::KCL_Status ALUTest::init()
{
	KCL::KCL_Status  result = LowLevelBase::init();
	if (result != KCL::KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	m_aluShader = Shader::CreateShader("LowLevel_ALU.vs", "LowLevel_ALU.fs", NULL, result);

	m_constantBuffer = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBuffer>();
	m_constantBuffer->commit();

	return KCL::KCL_TESTERROR_NOERROR;
}

void ALUTest::resetEnv()
{
	LowLevelBase::resetEnv();
	setBlendState(Blend_Overwrite);
	setDepthState(Depth_Disabled);
	m_aluShader->Bind();
	m_constantBuffer->bind(0);
}

bool ALUTest::render()
{
	float lightTime = m_time * 0.0003;
	float flyTime = m_time * 0.0005;

	ConstantBuffer* buf = (ConstantBuffer*)m_constantBuffer->map();
	buf->AspectRatio = m_aspectRatio;
	buf->Time = m_time;
	buf->EyePosition.set(10 * sin(flyTime), sin(flyTime * 0.5) * 2 + 2.1, -flyTime * 10);
	buf->LightDir.set(sin(lightTime), 0.5 + 0.5 * cos(lightTime), 0.5 - 0.5 * cos(lightTime));
	buf->LightDir.normalize();
	KCL::Matrix4x4 yaw, pitch, roll, orientation;
	KCL::Matrix4x4::RotateY(yaw, cos(flyTime) * 28);
	KCL::Matrix4x4::RotateX(pitch, cos(flyTime * 0.5) * 28);
	KCL::Matrix4x4::RotateZ(roll, sin(flyTime) * 28);
	buf->Orientation = pitch * roll * yaw;
	buf->Orientation.transpose();
	m_constantBuffer->unmap();

	drawTriangles(m_quadVertices, m_quadIndices);

	return false;
}

void ALUTest::FreeResources()
{
	SAFE_DELETE(m_constantBuffer);
}


