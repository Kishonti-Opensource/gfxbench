/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dx_blending.h"

UITest::UITest(const GlobalTestEnvironment* const gte) :
LowLevelBase(gte),
m_testStage(0),
m_uiShader(NULL),
m_score(0),
m_transferredBytes(0),
m_displayedElementCount(64),
m_elementCountStep(128)
{
}

UITest::~UITest()
{
	FreeResources();
}

KCL::Image* UITest::generateImage(int screenWidth, int screenHeight, int idx)
{
	// Prepare buffers with single colors
	static const KCL::uint32 colorCount = 12;
	static const KCL::uint32 colors[] =
	{
		0x0000ff,	// Red
		0x007fff,	// Orange
		0x00ffff,	// Yellow
		0x00ff7f,	// 

		0x00ff00,	// Green
		0x7fff00,	//  
		0xffff00,	// Cyan
		0xff7f00,	// 

		0xff0000,	// Blue
		0xff007f,	// Purple
		0xff00ff,	// Magenta
		0x7f00ff,	// 
	};

	// Calculate pseudo-random numbers - we want the scene to be reproducible
	KCL::uint32 color = colors[idx % colorCount];

	float relativeWidth = sinf(idx * 2) * 0.25f + 0.5f;
	float relativeHeight = sinf(idx * 3) * 0.25f + 0.5f;
	int width = max(relativeWidth * screenWidth, 1);
	int height = max(relativeHeight * screenHeight, 1);

	int cornerRadius = max((width < height ? width : height) / 3, 1);

	KCL::Image* image = new KCL::Image(width, height, KCL::Image_RGBA8888);
	KCL::uint32* ptr = (KCL::uint32*)image->getData();

	for (int y = 0; y < height; y++)
	{
		int yEdgeDist = (y < height / 2) ? y : (height - 1 - y);
		yEdgeDist = yEdgeDist > cornerRadius ? 0 : (cornerRadius - yEdgeDist);

		for (int x = 0; x < width; x++, ptr++)
		{
			int xEdgeDist = (x < width / 2) ? x : (width - 1 - x);
			xEdgeDist = xEdgeDist > cornerRadius ? 0 : (cornerRadius - xEdgeDist);

			float edgeDist = sqrtf(xEdgeDist * xEdgeDist + yEdgeDist * yEdgeDist);

			if (edgeDist < 0.75f * cornerRadius - 2)
			{
				if (!(x & 0xf) || !(y & 0xf))
				{
					*ptr = 0xff000000 | color;
				}
				else if (!(x & 0x7) || !(y & 0x7))
				{
					*ptr = 0xcf000000 | color;
				}
				else
				{
					int alpha = 160 + (96 * edgeDist / cornerRadius);
					*ptr = (alpha << 24) | color;
				}
			}
			else if (edgeDist <= 0.75f * cornerRadius + 2)
			{
				// White border
				*ptr = 0xffffffff;
			}
			else if (edgeDist < cornerRadius)
			{
				// Shadow
				int alpha = (1.0 - (edgeDist / cornerRadius)) * 220;
				*ptr = alpha << 24;
			}
			else
			{
				*ptr = 0x00000000;
			}
		}
	}

	return image;
}

UITest::TestUIElement::TestUIElement(const KCL::Image* image, float positionX, float positionY) :
m_width(image->getWidth()),
m_height(image->getHeight()),
m_positionX(positionX),
m_positionY(positionY)
{
	m_texture = new DXB::DXBTexture(image); // TextureFactory().CreateTexture(image);
	m_texture->setMipLevels(1);
	m_texture->setWrapS(KCL::TextureWrap_Clamp);
	m_texture->setWrapT(KCL::TextureWrap_Clamp);
	m_texture->setMinFilter(KCL::TextureFilter_Nearest);
	m_texture->setMagFilter(KCL::TextureFilter_Nearest);
	m_texture->setMipFilter(KCL::TextureFilter_NotApplicable);
	m_texture->commit();
}

UITest::TestUIElement::~TestUIElement()
{
	SAFE_DELETE(m_texture)
}

KCL::KCL_Status UITest::init()
{
	KCL::KCL_Status  result = LowLevelBase::init();
	if (result != KCL::KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	m_constantBuffer = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBuffer>();
	m_constantBuffer->commit();

	int totalSize = 0;
	int idx = 0;

	while (totalSize < (64 << 20) && idx < 2000)
	{
		KCL::Image* image = generateImage(m_screenWidth, m_screenHeight, idx);
		float relativeWidth = (float)image->getWidth() * m_invScreenWidth;
		float relativeHeight = (float)image->getHeight() * m_invScreenHeight;

		float positionX = sinf(idx * 5) * (1.0 - 1.2f * relativeWidth);
		float positionY = cosf(idx * 7) * (1.0 - 1.2f * relativeHeight);

		TestUIElement* element = new TestUIElement(image, positionX, positionY);
		m_uiElements.push_back(element);
		idx++;
		totalSize += image->getDataSize();

		delete image;
	}

	m_uiShader = Shader::CreateShader("LowLevel_UI.vs", "LowLevel_UI.fs", NULL, result);

	return KCL::KCL_TESTERROR_NOERROR;
}

bool UITest::animate(const int time)
{
	SetAnimationTime(time);
	float sint = 0.75f + cosf(time * 0.0031415926535897932384626433832795f) * 0.25f;

	static KCL::uint32 startFrame = 0;
	static double startTime;

	double elapsedTime = 0.001f *  (KCL::g_os->GetTimeMilliSec() - startTime);

	switch (m_testStage)
	{
	case 0:
		m_clearColor.set(sint, 0, 0);
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
		m_clearColor.set(sint, 0, 0);

		// Spend at most half the play time with adjustment
		if (time > m_settings->m_play_time * 0.5f)
		{
			m_testStage = 2;
			return true;
		}

		if (fps > 25)
		{
			m_elementCountStep = m_elementCountStep > 0 ? m_elementCountStep : (-m_elementCountStep / 2);
			if (!m_elementCountStep)
			{
				m_elementCountStep = 16;
			}

			m_displayedElementCount += m_elementCountStep;
		}
		else if (fps < 20)
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
		m_clearColor.set(sint, sint * (elapsedTime / 2), 0);
		if (elapsedTime > 1.0)
		{
			startTime = KCL::g_os->GetTimeMilliSec();
			startFrame = m_frames;
			m_transferredBytes = 0;
			m_testStage = 3;
		}

		return true;
	}

	case 3:
	{
		// Perform measurement
		m_clearColor.set(0, sint, 0);
		m_score = m_transferredBytes / elapsedTime / (1024 * 1024);
		return time < m_settings->m_play_time;
	}

	default:
		return time < m_settings->m_play_time;
	}
}

void UITest::resetEnv()
{
	LowLevelBase::resetEnv();
	setBlendState(Blend_Alpha);
	setDepthState(Depth_Disabled);
	m_uiShader->Bind();
	m_constantBuffer->bind(0);
}

bool UITest::render()
{
	for (int i = 0; i < m_displayedElementCount; i++)
	{
		TestUIElement* element = m_uiElements[i % m_uiElements.size()];
		element->getTexture()->bind(0);

		float param = m_time * 0.0001 * (5 + (i & 11));
		float offsetX = 0.2f * m_invScreenWidth * element->m_width * cos(param);
		float offsetY = 0.2f * m_invScreenHeight * element->m_height * sin(param);

		ConstantBuffer* buf = (ConstantBuffer*)m_constantBuffer->map();
		buf->Scale.set(m_invScreenWidth * element->m_width, m_invScreenHeight * element->m_height);
		buf->Offset.set(offsetX + element->m_positionX, offsetY + element->m_positionY);
		m_constantBuffer->unmap();

		drawTriangles(m_quadVertices, m_quadIndices);

		m_transferredBytes += (element->m_width * element->m_height) << 2;
	}

	return false;
}

void UITest::FreeResources()
{
	SAFE_DELETE(m_constantBuffer);

	for (std::vector<TestUIElement*>::iterator it = m_uiElements.begin(); it != m_uiElements.end(); ++it)
	{
		TestUIElement* element = *it;
		delete element;
	}

	m_uiElements.clear();
}
