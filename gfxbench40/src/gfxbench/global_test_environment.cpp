/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  global_test_enviroment.cpp
//  GFXBench
//
//  Created by Hapák József on 07/08/14.
//
//

#include "global_test_environment.h"

GlobalTestEnvironment::GlobalTestEnvironment(GraphicsContext* graphicscontext) : m_graphicscontext(graphicscontext), m_ts(0)
{
    
}

GlobalTestEnvironment::~GlobalTestEnvironment()
{
    
}

bool GlobalTestEnvironment::IsGraphicsContextGLorES() const
{
    return (m_graphicscontext->type() == GraphicsContext::GLES) || (m_graphicscontext->type() == GraphicsContext::OPENGL) ;
}

bool GlobalTestEnvironment::IsGraphicsContextMetal() const
{
    return (m_graphicscontext->type() == GraphicsContext::METAL) ;
}

GraphicsContext* GlobalTestEnvironment::GetGraphicsContext() const
{
    return m_graphicscontext ;
}

TestDescriptor* GlobalTestEnvironment::GetTestDescriptor() const
{
	return m_ts ;
}

void GlobalTestEnvironment::SetTestDescriptor(TestDescriptor* ts)
{
	m_ts = ts ;
}

bool GlobalTestEnvironment::IsEngine(std::string engine) const 
{
	return engine == m_ts->m_engine ;
}

void GlobalTestEnvironment::SetTestId(const std::string &test_id)
{
	m_test_id = test_id;
}

const std::string &GlobalTestEnvironment::GetTestId() const
{
	return m_test_id;
}
