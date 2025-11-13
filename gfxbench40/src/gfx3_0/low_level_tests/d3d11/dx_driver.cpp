/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dx_driver.h"

CPUOverheadTest::CPUOverheadTest(const GlobalTestEnvironment* const gte) :
LowLevelBase(gte),
m_cpuShader(NULL)
{
	for (int i = 0; i < BufferCount; i++)
	{
		m_vertexBuffers[i] = NULL;
		m_indexBuffers[i] = NULL;
	}
}

CPUOverheadTest::~CPUOverheadTest()
{
	FreeResources();
}

KCL::KCL_Status CPUOverheadTest::init()
{
	m_score = 0;
	KCL::KCL_Status  result = LowLevelBase::init();
	if (result != KCL::KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	m_cpuShader = Shader::CreateShader("LowLevel_CpuOverhead.vs", "LowLevel_CpuOverhead.fs", NULL, result);

	m_constantBuffer = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBuffer>();
	m_constantBuffer->commit();

	int seed = 123;
	for (int i = 0; i < BufferCount; i++)
	{
		int vertexCount = i + 3;
		int triangleCount = vertexCount - 2;

		BillboardVertex* vertices = new BillboardVertex[vertexCount];
		BillboardVertex* vertPtr = vertices;

		// Generate vertices
		float theta = 0.0f;
		float thetaStep = 6.283185307179586476925286766559f / vertexCount;
		for (int j = 0; j < vertexCount; j++, theta += thetaStep, vertPtr++)
		{
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);

			vertPtr->Position.set(cosTheta, sinTheta, KCL::Math::randomf(&seed));
			vertPtr->TexCoord.set(cosTheta * 0.5f + 0.5f, sinTheta * 0.5f + 0.5f);
			vertPtr->Color.set(0.0, 0.0, 0.0, 1.0);//cosTheta, sinTheta, 1.0);
		}

		m_vertexBuffers[i] = KCL::VertexBuffer::factory->CreateBuffer<BillboardVertex>(vertices, vertexCount);
		m_vertexBuffers[i]->commit();
		delete[] vertices;

		int indexCount = triangleCount * 3;
		KCL::uint16* indices = new KCL::uint16[indexCount];
		KCL::uint16* indPtr = indices;
		for (int j = 1; j <= triangleCount; j++)
		{
			*(indPtr++) = 0;
			*(indPtr++) = j;
			*(indPtr++) = j + 1;
		}

		m_indexBuffers[i] = KCL::IndexBuffer::factory->CreateBuffer(indices, indexCount);
		m_indexBuffers[i]->commit();
		delete indices;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}

bool CPUOverheadTest::animate(const int time)
{
	SetAnimationTime(time);

	int score = m_frames;
	if (score > m_score)
	{
		m_score = score;
	}

	float sint = cosf(m_time * 0.0015707963267948966192313216916398f);
	m_clearColor.set(0.25f, 0.75f - 0.25f * sint, 0.75f + 0.25f * sint);
	return time < m_settings->m_play_time;
}

void CPUOverheadTest::resetEnv()
{
	LowLevelBase::resetEnv();
	setDepthState(Depth_Disabled);
	setBlendState(Blend_Overwrite);
	m_constantBuffer->bind(0);
	m_cpuShader->Bind();
}

bool CPUOverheadTest::render()
{
	float rotation = 0.003 * m_time;

	int seed = 0;
	const int ColumnCount = 50;
	const int RowCount = 50;
	int instanceIdx = 0;
	for (int y = 0; y < RowCount; y++)
	{
		for (int x = 0; x < ColumnCount; x++)
		{
			float r0 = KCL::Math::randomf(&seed);
			float r1 = KCL::Math::randomf(&seed);
			float r2 = KCL::Math::randomf(&seed);
			float r3 = KCL::Math::randomf(&seed);
			int stateIdx = KCL::Math::randomf(&seed) * 140;
			instanceIdx++;
			seed += x + y;

			ConstantBuffer* buf = (ConstantBuffer*)m_constantBuffer->map();
			buf->Position.set(x + 0.5f * (y & 1), y);
			buf->MatrixSize.set(ColumnCount, RowCount);

			float r = rotation * (0.2f + r2) + r0;
			float cr = cos(rotation);
			float sr = sin(rotation);
			buf->Rotation.set(cr, -sr, sr, cr);
			buf->Scale.set(r1 + 0.5f, r2 + 0.5f);
			buf->ScreenResolution.set(m_screenWidth, m_screenHeight);
			buf->Color0.set(r0, r1, r2, r3);
			buf->Color1.set(r1, r2, r3, r0);
			buf->Color2.set(r2, r3, r0, r1);
			buf->Color3.set(r3, r0, r1, r2);
			m_constantBuffer->unmap();

			DepthState depthState = (DepthState)(instanceIdx / 100 % 6);
			setDepthState(depthState);

			BlendState blendState = (BlendState)(instanceIdx / 10 % 5);
			setBlendState(blendState);

			int bufferIdx = stateIdx % BufferCount;
			drawTriangles(m_vertexBuffers[bufferIdx], m_indexBuffers[bufferIdx]);
		}
	}

	return false;
}

void CPUOverheadTest::FreeResources()
{
	SAFE_DELETE(m_constantBuffer);
	for (int i = 0; i < BufferCount; i++)
	{
		SAFE_DELETE(m_vertexBuffers[i]);
		SAFE_DELETE(m_indexBuffers[i]);
	}
}