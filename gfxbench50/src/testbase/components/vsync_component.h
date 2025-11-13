/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VSYNC_COMPONENT_H
#define VSYNC_COMPONENT_H

#include "test_component.h"
#include <kcl_base.h>

namespace GLB
{

class TimeComponent;

class VSyncComponent : public TestComponent
{
public:
	static const char *NAME;

	VSyncComponent(TestBaseGFX *test);
	virtual ~VSyncComponent();

	virtual bool Init() override;
	virtual void EndFrame() override;
    virtual void CreateResult(tfw::ResultGroup &result_group) override;

private:
	TimeComponent *m_time_component;

	KCL::uint32 m_vsync_frames_accum_vsync;
	KCL::uint32 m_vsync_time_accum;
	KCL::uint32 m_vsync_frames_accum;
	KCL::uint32 m_vsync_limit_count;
};

}

#endif
