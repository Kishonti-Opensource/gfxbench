/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_D3D_GENERATOR__
#define __KSL_D3D_GENERATOR__

#include "ksl_generator.h"
#include "ksl_d3d_nodes.h"
#include "ksl_d3d_translator.h"


const std::string KSL_D3D_SAMPLER_SUFFIX = "__ksl_sampler__";


class KSLD3DGenerator : public KSLGenerator
{
public:
	KSLD3DGenerator();
	virtual ~KSLD3DGenerator();

	virtual bool Generate();
	virtual void SetTranslator(KSLTranslator* translator);

	virtual bool VisitVariableDefinitions(KSLVariableDefinitionsNode *vardef);
	virtual bool VisitFunction(KSLFunctionNode* function);
	virtual bool VisitGlobalSpaceNode(KSLASTNode* n);
	virtual bool VisitImageDefinition(KSLImageDefinitionNode* idn);
	bool VisitD3DUniformInterfaceNode(KSLD3DUniformInterfaceNode* d3duin);
	bool VisitD3DInputOutputInterfaceNode(KSLD3DInputOutputIntefaceNode* d3dioin);
	bool VisitD3DMainFunctionNode(KSLFunctionNode* d3dmfn);

	// statements
	virtual bool VisitStatementOrVariableDefinitions(KSLASTNode *n);
	virtual bool VisitReturnStatement(KSLReturnStatementNode* rs);
	virtual bool VisitForStatement(KSLForStatementNode *fs);

	// expressions
	virtual bool VisitVariableExpression(KSLVariableExpressionNode *ve);
	virtual bool VisitBinaryExpression(KSLBinaryExpressionNode *be);
	virtual bool VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce);
	virtual bool VisitConstructorExpression(KSLConstructorExpressionNode *ce);
	virtual bool VisitSelectorExpression(KSLSelectorExpressionNode *se);
	virtual bool VisitLiteralExpression(KSLLiteralExpressionNode *le);

	virtual std::string TypeToString(KSLType type);
	virtual std::string StorageQualifierToString(KSLStorageQualifier qualifier);

private:

	void PrintSamplingMethods();

	bool Print_texture(KSLFunctionCallExpressionNode *fce);
	bool Print_textureGather(KSLFunctionCallExpressionNode* fce);
	bool Print_texelFetch(KSLFunctionCallExpressionNode* fce);
	bool Print_textureLod(KSLFunctionCallExpressionNode *fce);
	bool Print_textureLodOffset(KSLFunctionCallExpressionNode *fce);

	bool PrintTextureAccess(KSLFunctionCallExpressionNode* fce);
	bool PrintSubpassLoad(KSLFunctionCallExpressionNode* fce);
	bool PrintBarrier(KSLFunctionCallExpressionNode* fce);
	bool PrintImageStore(KSLFunctionCallExpressionNode* fce);

	std::string GetSamplerStateType(const KSLType &type);

	KSLD3DTranslator* m_d3d_translator;

	bool m_visiting_main;
	bool m_visiting_global_vdn;
	
	uint32_t m_next_srv_binding;
	uint32_t m_next_uav_binding;
};


#endif //__KSL_D3D_GENERATOR__

