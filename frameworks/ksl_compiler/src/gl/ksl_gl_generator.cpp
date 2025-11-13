/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_gl_generator.h"


KSLGLGenerator::KSLGLGenerator()
{

}


KSLGLGenerator::~KSLGLGenerator()
{

}


bool KSLGLGenerator::VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "subpassLoad")
	{
		m_result << "texelFetch(";
		VisitExpression(fce->arguments[0]);
		m_result << ", ivec2(gl_FragCoord.xy), 0)";
		return true;
	}

	return KSLGLGeneratorBase::VisitFunctionCallExpression(fce);
}

