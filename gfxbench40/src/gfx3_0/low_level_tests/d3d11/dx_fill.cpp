/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dx_fill.h"

CompressedFillTest::CompressedFillTest(const GlobalTestEnvironment* const gte) :
LowLevelBase(gte),
m_constantBuffer(NULL),
m_colorTexture(NULL),
m_lightTexture(NULL),
m_fillShader(NULL),
m_displayedElementCount(64),
m_elementCountStep(32),
m_testStage(0)
{
}

CompressedFillTest::~CompressedFillTest()
{
	FreeResources();
}

KCL::KCL_Status CompressedFillTest::init()
{
	m_score = 0;
	KCL::KCL_Status  result = LowLevelBase::init();
	if (result != KCL::KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	m_fillShader = Shader::CreateShader("LowLevel_Fill.vs", "LowLevel_Fill.fs", NULL, result);

	std::string imageFolder;
	if (m_settings->GetTextureType() == "ETC1to565")
	{
		imageFolder = "fill/images_ETC1";
	}
	else
	{
		imageFolder = "fill/images_" + m_settings->GetTextureType();
	}

	if( (m_colorTexture = TextureFactory().CreateAndSetup(KCL::Texture_2D, (imageFolder + std::string("/Monoscope.pvr")).c_str())) == NULL)
	{
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	m_colorTexture->setMinFilter(KCL::TextureFilter_Linear);
	m_colorTexture->setMagFilter(KCL::TextureFilter_Linear);
	m_colorTexture->setMipFilter(KCL::TextureFilter_Linear);
	m_colorTexture->setWrapS(KCL::TextureWrap_Repeat);
	m_colorTexture->setWrapT(KCL::TextureWrap_Repeat);
	m_colorTexture->commit();

	if( (m_lightTexture = TextureFactory().CreateAndSetup(KCL::Texture_2D, (imageFolder + std::string("/LowLevelLightMap.pvr")).c_str())) == NULL)
	{
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	m_lightTexture->setMinFilter(KCL::TextureFilter_Linear);
	m_lightTexture->setMagFilter(KCL::TextureFilter_Linear);
	m_lightTexture->setMipFilter(KCL::TextureFilter_Linear);
	m_lightTexture->setWrapS(KCL::TextureWrap_Repeat);
	m_lightTexture->setWrapT(KCL::TextureWrap_Repeat);
	m_lightTexture->commit();

	m_constantBuffer = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBuffer>();
	m_constantBuffer->commit();

	return KCL::KCL_TESTERROR_NOERROR;
}

bool CompressedFillTest::animate(const int time)
{
	SetAnimationTime(time);
	float sint = 0.75f + cosf(time * 0.0031415926535897932384626433832795f) * 0.25f;

	static KCL::uint32 startFrame = 0;
	static double startTime = 0;

	double elapsedTime = 0.001f *  (KCL::g_os->GetTimeMilliSec() - startTime);

	switch (m_testStage)
	{
	case 0:
		startTime = KCL::g_os->GetTimeMilliSec();
		startFrame = m_frames;
		m_testStage = 1;
		return true;

		// Adjust number of displayed items
	case 1:
	{
		if (elapsedTime < 0.2f)
		{
			return true;
		}

		int frameCount = m_frames - startFrame;
		float fps = frameCount / elapsedTime;
		startTime = KCL::g_os->GetTimeMilliSec();
		startFrame = m_frames;

		// Spend at most half the play time with adjustment
		if (time > m_settings->m_play_time * 0.5f)
		{
			m_testStage = 2;
			return true;
		}

		if (fps > 30)
		{
			m_elementCountStep = m_elementCountStep > 0 ? m_elementCountStep : (-m_elementCountStep / 2);
			if (!m_elementCountStep)
			{
				m_elementCountStep = 16;
			}

			m_displayedElementCount += m_elementCountStep;
		}
		else if (fps < 25)
		{
			m_elementCountStep = m_elementCountStep < 0 ? (m_elementCountStep / 2) : (-m_elementCountStep / 2);
			if (!m_elementCountStep)
			{
				m_elementCountStep = -16;
			}

			m_displayedElementCount += m_elementCountStep;
			if (m_displayedElementCount < 10)
			{
				m_displayedElementCount = 10;
			}
		}
		else
		{
			m_testStage = 2;
		}

		return true;
	}

		// Number of items is set, wait for fps settle...
	case 2:
	{
		// Wait for frame rate to settle
		if (elapsedTime > 1.0)
		{
			startTime = KCL::g_os->GetTimeMilliSec();
			startFrame = m_frames;
			m_transferredTexels = 0;
			m_testStage = 3;
		}

		return true;
	}

	case 3:
	{
		// Perform measurement
		m_score = m_transferredTexels / elapsedTime / (1024 * 1024);
		return time < m_settings->m_play_time;
	}

	default:
		return time < m_settings->m_play_time;
	}
}

void CompressedFillTest::resetEnv()
{
	LowLevelBase::resetEnv();
	setDepthState(Depth_Disabled);
	m_constantBuffer->bind(0);
	m_fillShader->Bind();
	m_colorTexture->bind(0);
	m_lightTexture->bind(1);
}

bool CompressedFillTest::render()
{
	const float exposureTime = 150;
	for (int i = 0; i < m_displayedElementCount; i++)
	{
		ConstantBuffer* buf = (ConstantBuffer*)m_constantBuffer->map();
		buf->AspectRatio = m_aspectRatio;
		buf->Rotation = (m_time + exposureTime * i / m_displayedElementCount) * 0.0003;
		m_constantBuffer->unmap();

		setBlendState(Blend_ConstantAlpha, 1.0f / (1 + i));
		drawTriangles(m_quadVertices, m_quadIndices);
	}

	const unsigned int texelsPerScreen = m_settings->m_viewport_width * m_settings->m_viewport_height * 4;
	m_transferredTexels += texelsPerScreen * m_displayedElementCount;
	return false;
}

void CompressedFillTest::FreeResources()
{
	SAFE_DELETE(m_constantBuffer);
	SAFE_DELETE(m_colorTexture);
	SAFE_DELETE(m_lightTexture);
}

