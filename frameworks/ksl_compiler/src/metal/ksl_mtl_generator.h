/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_MTL_GENERATOR__
#define __KSL_MTL_GENERATOR__

#include "ksl_generator.h"

#include "ksl_mtl_nodes.h"
#include "ksl_mtl_translator.h"


const std::string KSL_METAL_SAMPLER_SUFFIX = "__ksl_sampler__";
const std::string KSL_METAL_GLOBAL_SUFFIX = "__ksl_global__";

const std::string KSL_SHADOW_SAMPLER_NAME = "__ksl_shadow_sampler__";

const std::string KSL_METAL_SAMPLE_2D_ARRAY_NAME = "__ksl_sample_2d_array__";
const std::string KSL_METAL_SAMPLE_2D_DEPTH_NAME = "__ksl_sample_2d_depth__";
const std::string KSL_METAL_SAMPLE_2D_DEPTH_ARRAY_NAME = "__ksl_sample_2d_depth_array__";

const std::string KSL_METAL_SAMPLE_2D_ARRAY_LOD_NAME = "__ksl_sample_2d_array_lod__";
const std::string KSL_METAL_SAMPLE_2D_DEPTH_LOD_NAME = "__ksl_sample_2d_depth_lod__";
const std::string KSL_METAL_SAMPLE_2D_DEPTH_ARRAY_LOD_NAME = "__ksl_sample_2d_depth_array_lod__";

const std::string KSL_METAL_SAMPLE_CUBE_DEPTH_NAME = "__ksl_sample_cube_depth__";


class KSLMetalGenerator : public KSLGenerator
{
public:
	KSLMetalGenerator(bool mtl_use_subpass, uint32_t target_api);
	~KSLMetalGenerator();

	bool VisitMetalUniformInterfaceNode(KSLMetalUniformInterfaceNode* muin);
	bool VisitMetalInputOutputInterfaceNode(KSLMetalInputOutputIntefaceNode* mioin);
	bool VisitMetalMainFunctionNode(KSLMetalMainFunctionNode* mmfn);
	virtual bool VisitVariableExpression(KSLVariableExpressionNode *ve);
	virtual bool VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce);
	virtual bool VisitFunction(KSLFunctionNode* function);

	virtual bool VisitControlStatement(KSLControlStatementNode* cs);
	virtual bool VisitGlobalSpaceNode(KSLASTNode* n);
	virtual bool VisitReturnStatement(KSLReturnStatementNode* rs);

	virtual bool Generate();
	virtual void SetTranslator(KSLTranslator* translator);

	virtual std::string TypeToString(KSLType type);

	bool m_mtl_subpass_as_framebuffer_fetch;

private:
	
	bool Print_texture(KSLFunctionCallExpressionNode* fce);
	bool Print_textureGather(KSLFunctionCallExpressionNode* fce);
	bool Print_textureLod(KSLFunctionCallExpressionNode* fce);
	bool Print_textureLodOffset(KSLFunctionCallExpressionNode* fce);
    bool Print_texelFetch(KSLFunctionCallExpressionNode* fce);

	bool PrintTextureAccess(KSLFunctionCallExpressionNode* fce);
	bool PrintBarrier(KSLFunctionCallExpressionNode* fce);
	bool PrintImageStore(KSLFunctionCallExpressionNode* fce);
	bool PrintSubpassLoad(KSLFunctionCallExpressionNode* fce);
    bool PrintPow(KSLFunctionCallExpressionNode* fce);

	virtual std::string StorageQualifierToString(KSLStorageQualifier qualifier);

	void PrintHeader();
	std::string FloatPrecisionToString(KSLPrecision p) const;
	std::string GetImageTypeName(uint32_t t) const;

	KSLMetalTranslator* m_mtl_translator;
	bool m_visiting_main;
	bool m_visiting_global_vdn;
};


#endif //__KSL_MTL_GENERATOR__

