/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "test_component.h"

#include "test_base_gfx.h"
#include "test_descriptor.h"

using namespace GLB;

TestComponent::TestComponent(TestBaseGFX *test, const char *name)
{
	m_test = test;
	m_name = name;
}


TestComponent::~TestComponent()
{

}


const std::string &TestComponent::GetName() const
{
	return m_name;
}


TestDescriptor &TestComponent::GetTestDescriptor() const
{
	return m_test->GetTestDescriptor();
}
