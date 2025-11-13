/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "test_base.h"
#include "kcl_image.h"
#include "kcl_math3d.h"
#include "kcl_texture.h"

class UITest_Base : public GLB::TestBase
{
private:
	float m_score;
	int m_elementCountStep;
	int m_testStage;

public:
	UITest_Base(const GlobalTestEnvironment * const gte) ;
	virtual ~UITest_Base() ;

protected:
	class TestUIElement
	{
	public:
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		float m_positionX;
		float m_positionY;
		const KCL::Image* m_image;
		KCL::Texture* m_texture;

		TestUIElement(const KCL::Image* image, KCL::Texture* texture, float positionX, float positionY);
		~TestUIElement();
		inline void LoadTexture()	{ m_texture->commit(); }
		inline void ReleaseTexture()	{ m_texture->release(); }
	};

	KCL::Vector3D m_clearColor;
	double m_transferredBytes;
	int m_displayedElementCount;

	std::vector<TestUIElement*> m_uiElements;

	void createItems(int screenWidth, int screenHeight);
	
	virtual KCL::Texture* createTexture(KCL::Image* image) = 0 ;

	KCL::Image* generateImage(int screenWidth, int screenHeight, int idx);
	virtual bool animate(const int time);
	virtual float getScore() const { return m_score; }

};

