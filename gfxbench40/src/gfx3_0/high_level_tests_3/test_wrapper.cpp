/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "test_wrapper.h"


TestWrapper::TestWrapper(const GlobalTestEnvironment* const gte, TestBase* wrappedTest) : 
	TestBase(gte),
	m_wrappedTest(wrappedTest)
{
}

TestWrapper::~TestWrapper(void)
{
	if (m_wrappedTest)
	{
		delete m_wrappedTest;
		m_wrappedTest = NULL;
	}
}

bool TestWrapper::animate(const int time)
{
	if (!m_wrappedTest)
	{
		return false;
	}
	
	updateInternalSettings();
	bool result = m_wrappedTest->animate(time);
	updateWrapperSettings();

	return result;
}

bool TestWrapper::render0(const char* screenshotName)
{
	if (!m_wrappedTest)
	{
		return false;
	}

	updateInternalSettings();
	bool result = m_wrappedTest->render0(screenshotName);
	updateWrapperSettings();

	return result;
}


void TestWrapper::finishTest()
{
	if (!m_wrappedTest)
	{
		return ;
	}

	m_wrappedTest->finishTest();
}


bool TestWrapper::isWarmup() const
{
	return m_wrappedTest ? m_wrappedTest->isWarmup() : false;
}

KCL::uint32 TestWrapper::indexCount() const
{
	return m_wrappedTest ? m_wrappedTest->indexCount() : 0;
}

bool TestWrapper::isLowLevel() const
{
	return m_wrappedTest ? m_wrappedTest->isLowLevel() : false;
}

KCL::KCL_Status TestWrapper::init0()
{
	KCL::KCL_Status error =  m_wrappedTest->init0();
	SetRuntimeError(error);
	return error;
}

void TestWrapper::resetEnv()
{
	if (m_wrappedTest)
	{
		m_wrappedTest->resetEnv();
	}
}

void TestWrapper::FreeResources()
{
	if (getWrappedTest()) getWrappedTest()->FreeResources();
}

void TestWrapper::initEffects (int a, const char *const* b)
{
	if (getWrappedTest()) getWrappedTest()->initEffects(a,b);
}


bool TestWrapper::resize(int width, int height)
{
	return m_wrappedTest ? m_wrappedTest->resize(width, height) : true;
}

void TestWrapper::copySettings(TestBase *src, TestBase *dst)
{
	dst->SetAnimationTime(src->GetAnimationTime());
	dst->SetRuntimeError(src->GetRuntimeError());
	if (dst->getFrames()>src->getFrames()) dst->ResetFrameCounter();
	while (dst->getFrames()<src->getFrames()) dst->IncFrameCounter();
}