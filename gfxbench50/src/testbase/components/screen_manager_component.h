/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SCREEN_MANAGER_COMPONENT_H
#define SCREEN_MANAGER_COMPONENT_H

#include "test_component.h"
#include "kcl_base.h"
#include "time_component.h"


namespace GLB
{


class ScreenManagerComponent : public TestComponent
{
public:
	static const char *NAME;

	ScreenManagerComponent(TestBaseGFX *test, KCL::uint32 native_width, KCL::uint32 native_height)
		: TestComponent(test, NAME)
		, m_native_width(native_width)
		, m_native_height(native_height)
		, m_swapbuffer_needed(true)
		, m_command_buffer(0)
		, m_is_warmup(false)
	{
	}

	virtual ~ScreenManagerComponent()
	{
	}

	void SetCommandBuffer(KCL::uint32 command_buffer) { m_command_buffer = command_buffer; }
	virtual KCL::uint32 GetActiveBackbufferId() const = 0 ;
	virtual void GetTestSize(KCL::uint32 &width, KCL::uint32 &height) const = 0;
	virtual void FinishTest() = 0;
	virtual void FinishRender() = 0;
	virtual void WaitFinish(bool needs_readpixel) = 0;
	virtual const std::vector<KCL::uint32> &GetBackbuffers() const { return m_backbuffers; }
	virtual bool NeedSwapbuffer() const { return m_swapbuffer_needed; };
	virtual void Warmup() { }
	void ClearScreen();

	static ScreenManagerComponent* Create(TestBaseGFX* test, const TestDescriptor &td, KCL::uint32 native_width, KCL::uint32 native_height);

protected:	
	std::vector<KCL::uint32> m_backbuffers;
	KCL::uint32 m_native_width, m_native_height;
	bool m_swapbuffer_needed;
	KCL::uint32 m_command_buffer;
	bool m_is_warmup;
};


}


#endif // SCREEN_MANAGER_COMPONENT_H

