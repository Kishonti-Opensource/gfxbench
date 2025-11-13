/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "fill_base.h"


CompressedFillTest_Base::CompressedFillTest_Base(const GlobalTestEnvironment * const gte) : TestBase(gte),
	m_colorTexture(NULL),
	m_lightTexture(NULL),
	m_score(0),
    m_displayedElementCount(64),
    m_elementCountStep(32),
    m_testStage(0)
{

}


CompressedFillTest_Base::~CompressedFillTest_Base()
{

}


bool CompressedFillTest_Base::animate(const int time)
{
	SetAnimationTime(time);
	
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


KCL::KCL_Status CompressedFillTest_Base::init()
{
	std::string imageFolder;
	if (m_settings->GetTextureType() == "ETC1to565")
	{
		imageFolder = "fill/images_ETC1";
		KCL::Texture::SetDecodeDetails(KCL::Image_ETC1, KCL::Image_RGB565);
	}
	else if (m_settings->GetTextureType() == "ETC1to888")
	{
		imageFolder = "fill/images_ETC1";
		KCL::Texture::SetDecodeDetails(KCL::Image_ETC1, KCL::Image_RGB888);
	}
	else
	{
		KCL::Texture::SetDecodeDetails(KCL::ImageTypeAny, KCL::ImageTypeAny);
		imageFolder = "fill/images_" + m_settings->GetTextureType();
	}

	KCL::Image colorImg;
	if (!colorImg.load((imageFolder + std::string("/Monoscope.pvr")).c_str()))
	{
		INFO("missing file: %s", "Monoscope.pvr" );
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	m_colorTexture = CreateTexture(&colorImg);		
	m_colorTexture->setMinFilter(KCL::TextureFilter_Linear);
	m_colorTexture->setMagFilter(KCL::TextureFilter_Linear);
	m_colorTexture->setMipFilter(KCL::TextureFilter_NotApplicable);
	m_colorTexture->setWrapS(KCL::TextureWrap_Repeat);
	m_colorTexture->setWrapT(KCL::TextureWrap_Repeat);
	m_colorTexture->commit();

	KCL::Image lightImg;
	if (!lightImg.load((imageFolder + std::string("/LowLevelLightMap.pvr")).c_str()))
	{
		INFO("missing file: %s", "LowLevelLightMap.pvr");
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	m_lightTexture = CreateTexture(&lightImg);		
	m_lightTexture->setMinFilter(KCL::TextureFilter_Linear);
	m_lightTexture->setMagFilter(KCL::TextureFilter_Linear);
	m_lightTexture->setMipFilter(KCL::TextureFilter_NotApplicable);
	m_lightTexture->setWrapS(KCL::TextureWrap_Repeat);
	m_lightTexture->setWrapT(KCL::TextureWrap_Repeat);
	m_lightTexture->commit();

	return KCL::KCL_TESTERROR_NOERROR ;
}


void CompressedFillTest_Base::FreeResources()
{
	if (m_colorTexture)
	{
		delete m_colorTexture;
		m_colorTexture = NULL;
	}

	if (m_lightTexture)
	{
		delete m_lightTexture;
		m_lightTexture = NULL;
	}
}

