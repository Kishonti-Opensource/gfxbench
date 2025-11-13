/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_GL_GENERATOR__
#define __KSL_GL_GENERATOR__

#include "ksl_gl_generator_base.h"

class KSLGLGenerator : public KSLGLGeneratorBase
{
public:
	KSLGLGenerator();
	virtual ~KSLGLGenerator();

	virtual bool VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce);

};


#endif //__KSL_GL_GENERATOR__

