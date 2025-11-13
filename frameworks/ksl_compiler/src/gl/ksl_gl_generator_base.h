/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_GL_GENERATOR_BASE__
#define __KSL_GL_GENERATOR_BASE__

#include "../ksl_generator.h"

class KSLGLGeneratorBase : public KSLGenerator
{
public:
	KSLGLGeneratorBase();
	virtual ~KSLGLGeneratorBase();

	void Clear();

protected:

	bool PrintDefinitionLayout(const std::set<uint32_t> &bool_attribs,const std::map<uint32_t, uint32_t> int_attribs);
	bool PrintBufferDefinition(KSLVariableDefinitionsNode *vardef);
	virtual bool VisitVariableDefinitions(KSLVariableDefinitionsNode *vardef);
	virtual bool VisitImageDefinition(KSLImageDefinitionNode* idn);
	virtual bool VisitNumThreads(KSLNumThreadNode* ntn);
	virtual bool VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce);

	// expressions
	virtual bool VisitConstructorExpression(KSLConstructorExpressionNode *ce);
	virtual bool VisitLiteralExpression(KSLLiteralExpressionNode *le);

	std::string PrecisionToPrefix(KSLPrecision p);
	virtual std::string TypeToString(KSLType type);
	virtual std::string TypeToStringWithOutPrecision(KSLType type);

	virtual std::string TranslateAttribQualifer(uint32_t qualifier);
};



#endif //__KSL_GL_GENERATOR_BASE__

