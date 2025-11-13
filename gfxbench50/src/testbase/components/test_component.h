/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TEST_COMPONENT_H
#define TEST_COMPONENT_H

#include <string>

namespace tfw
{
	class ResultGroup;
}

namespace GLB
{

class TestBaseGFX;
class TestDescriptor;

class TestComponent
{
public:
	TestComponent(TestBaseGFX *test, const char *name);
	virtual ~TestComponent();

	virtual bool Init() { return true; }

	/*
	virtual void BeforeAnimate() { }
	virtual void AfterAnimate()	{ }

	virtual void BeforeRender()	{ }
	*/
	virtual void AfterRender() { }

	virtual void BeginFrame() { }
	virtual void EndFrame() { }

	virtual void BeginTest() { }

	virtual void CreateResult(tfw::ResultGroup &result_group) { }

	const std::string &GetName() const;
	TestDescriptor &GetTestDescriptor() const;

protected:
	std::string m_name;
	TestBaseGFX *m_test;

private:
	TestComponent(const TestComponent &copy);
};

}

#endif

