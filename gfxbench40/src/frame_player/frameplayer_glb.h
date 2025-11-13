/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FRAMEPLAYER_GLB_H
#define FRAMEPLAYER_GLB_H

#include <test_base.h>
#include <kcl_base.h>

namespace GLB {

	template<class t>
	class FramePlayerGLB: public TestBase
	{
		t *m_t;
	public:
		FramePlayerGLB( const TestDescriptor &ts): TestBase(ts)
		{
			m_t = new t();
		}
		virtual ~FramePlayerGLB() { FreeResources(); }
		virtual void FreeResources() {}
		virtual float getScore () const { return m_frames*1000.0f/getElapsedTime(); }
		virtual const char* getUom() const { return "fps"; }
		virtual bool isWarmup() const { return false; }
		virtual KCL::uint32 indexCount() const { return 0; }
		virtual bool animate (const int time) { return time <= m_settings->m_play_time; }
	private:
		virtual KCL::KCL_Status init() { m_t->init(); return KCL::KCL_TESTERROR_NOERROR; }
		virtual bool render() { m_t->render(); return true; }
	protected:
	};
}

#endif
