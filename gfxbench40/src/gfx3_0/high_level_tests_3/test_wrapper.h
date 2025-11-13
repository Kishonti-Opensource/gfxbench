/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTWRAPPER_H
#define TESTWRAPPER_H

#include "test_base.h"

class TestWrapper : public GLB::TestBase
{
protected:
	GLB::TestBase* m_wrappedTest;

public:
	TestWrapper(const GlobalTestEnvironment* const gte, TestBase* wrappedTest);

	virtual ~TestWrapper(void);

	virtual bool animate(const int time);
	virtual void setLoadingProgressPtr(float* ptr) { if (m_wrappedTest) m_wrappedTest->setLoadingProgressPtr(ptr); }
	virtual bool render0(const char* screenshotName = NULL);
	virtual void finishTest();

	bool isWarmup() const;
	KCL::uint32 indexCount() const;
	void resetEnv();
	bool resize(int width, int height);
	virtual KCL::KCL_Status init0();
	bool isLowLevel() const;

	inline TestBase* getWrappedTest() const	{ return m_wrappedTest; }

	void FreeResources();
	void initEffects(int a, const char *const* b);

protected:
	KCL::KCL_Status init() { return KCL::KCL_TESTERROR_NOERROR; }
	bool render() { return false; }
	
	inline void updateInternalSettings()
	{
		if (m_wrappedTest)
		{
			copySettings(this, m_wrappedTest);
		}
	}

	inline void updateWrapperSettings()
	{
		if (m_wrappedTest)
		{
			copySettings(m_wrappedTest, this);
		}
	}

	static void copySettings(TestBase *src, TestBase *dst);
};

template <class T>
class TestWrapperA : public TestWrapper
{
public:
	TestWrapperA(const GlobalTestEnvironment* const gte)
	{
		TestWrapper(gte, new T(gte));
	}
};

#endif

