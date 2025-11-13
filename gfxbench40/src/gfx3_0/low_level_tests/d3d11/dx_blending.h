/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DX_BLENDING_H
#define DX_BLENDING_H

#include "new_low_level_base_dx.h"

class UITest : public LowLevelBase
{
private:
	__declspec(align(16)) struct ConstantBuffer
	{
		KCL::Vector2D Scale;
		KCL::Vector2D Offset;
	};

	struct TestUIElement
	{
	public:
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		float m_positionX;
		float m_positionY;
		KCL::Texture* m_texture;

		TestUIElement(const KCL::Image* image, float positionX, float positionY);
		~TestUIElement();

		inline KCL::Texture* getTexture() const	{ return m_texture; }
	};

	KCL::ConstantBuffer* m_constantBuffer;
	std::vector<TestUIElement*> m_uiElements;
	Shader* m_uiShader;

	double m_transferredBytes;
	float m_score;

	int m_displayedElementCount;
	int m_elementCountStep;
	int m_testStage;

public:
	UITest(const GlobalTestEnvironment* const gte);
	~UITest();

	/* override */ virtual const char* getUom() const { return "MB/s"; }
	/* override */ virtual bool isWarmup() const { return false; }
	/* override */ virtual KCL::uint32 indexCount() const { return 0; }
	/* override */ virtual float getScore() const { return m_score; }

	/* override */ virtual KCL::KCL_Status init();
	/* override */ virtual bool animate(const int time);
	/* override */ virtual void resetEnv();
	/* override */ virtual bool render();
	/* override */ virtual void FreeResources();

protected:
	KCL::Image* generateImage(int screenWidth, int screenHeight, int idx);
};


#endif