/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_VK_GENERATOR__
#define __KSL_VK_GENERATOR__

#include "../gl/ksl_gl_generator_base.h"
#include "ksl_vk_translator.h"
#include "ksl_vk_nodes.h"

class KSLVKGenerator : public KSLGLGeneratorBase
{
public:
	KSLVKGenerator(bool vk_use_subpass);
	virtual ~KSLVKGenerator();

	virtual bool Generate();
	virtual void SetTranslator(KSLTranslator* translator);

protected:

	bool VisitVulkanInterfaceNode(KSLVulkanInterfaceNode* vin);
	virtual bool VisitGlobalSpaceNode(KSLASTNode* n);
	virtual bool VisitVariableExpression(KSLVariableExpressionNode *ve);
	virtual bool VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce);

	virtual std::string TypeToString(KSLType type);
	virtual std::string TranslateAttribQualifer(uint32_t qualifier);

	void PrintHeader();

	KSLVKTranslator *m_vk_translator;
	bool m_vk_use_subpass;
};



#endif //__KSL_VK_GENERATOR__
