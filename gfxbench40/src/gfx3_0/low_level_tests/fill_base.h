/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __FILL_TEST_BASE__
#define __FILL_TEST_BASE__

#include "test_base.h"
#include "kcl_texture.h"

class CompressedFillTest_Base : public GLB::TestBase
{
public:
	CompressedFillTest_Base(const GlobalTestEnvironment * const gte) ;
	virtual ~CompressedFillTest_Base() ;

protected:
	virtual const char* getUom() const { return "MTexels/s"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	virtual float getScore () const { return m_score; }

	virtual KCL::Texture* CreateTexture(KCL::Image* img) = 0 ;

	virtual void FreeResources();

	virtual KCL::KCL_Status init ();
	virtual bool animate (const int time);

	KCL::Texture *m_colorTexture;
	KCL::Texture *m_lightTexture;

	int m_score;
	double m_transferredTexels;

	int m_displayedElementCount;
	int m_elementCountStep;
	int m_testStage;
private:

};


#endif // __FILL_TEST_BASE__