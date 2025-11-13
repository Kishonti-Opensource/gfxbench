/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_mtl_generator.h"

#include <assert.h>


KSLMetalGenerator::KSLMetalGenerator(bool mtl_use_subpass, uint32_t target_api)
{
	m_mtl_translator = NULL;
	m_visiting_main = false;
	m_visiting_global_vdn = false;
	m_mtl_subpass_as_framebuffer_fetch = mtl_use_subpass && (target_api == NGL_METAL_IOS);
}


KSLMetalGenerator::~KSLMetalGenerator()
{

}


bool KSLMetalGenerator::Generate()
{
	PrintHeader();

	return KSLGenerator::Generate();
}


void KSLMetalGenerator::PrintHeader()
{
	m_result << "#include <metal_stdlib>"; NewLine();
	m_result << "using namespace metal;"; NewLine();
	NewLine();

	m_result << "constexpr sampler " << KSL_SHADOW_SAMPLER_NAME << "(coord::normalized, filter::linear, address::clamp_to_edge, compare_func::less_equal);"; NewLine();
	NewLine();
	
	// sample 2D array
	m_result << "float4 "<< KSL_METAL_SAMPLE_2D_ARRAY_NAME << "(texture2d_array<float> t, sampler s, float3 tc)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   return t.sample(s, tc.xy, uint(tc.z));"; NewLine();
	m_result << "}"; NewLine();
	NewLine();

	// sample 2d depth
	m_result << "float " << KSL_METAL_SAMPLE_2D_DEPTH_NAME << "(depth2d<float> t, float3 tc)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   float a = clamp(tc.z,0.0,1.0);"; NewLine();
	m_result << "   return t.sample_compare( "<< KSL_SHADOW_SAMPLER_NAME <<", tc.xy, a); "; NewLine();
	m_result << "}"; NewLine();
	NewLine();
	
	// sample 2d depth array
	m_result << "float "<< KSL_METAL_SAMPLE_2D_DEPTH_ARRAY_NAME << "(depth2d_array<float> t, float4 tc)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   float a = clamp(tc.w,0.0,1.0);"; NewLine();
	m_result << "   return t.sample_compare(" << KSL_SHADOW_SAMPLER_NAME << ", tc.xy, uint(tc.z), a);"; NewLine();
	m_result << "}"; NewLine();
	NewLine();
	
	// sample depth cube
	m_result << "float "<< KSL_METAL_SAMPLE_CUBE_DEPTH_NAME << "(depthcube<float> t, float4 tc)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   float a = clamp(tc.w,0.0,1.0);"; NewLine();
	m_result << "   return t.sample_compare(" << KSL_SHADOW_SAMPLER_NAME << ", tc.xyz, a);"; NewLine();
	m_result << "}"; NewLine();
	NewLine();

	
	// sample 2d array lod
	m_result << "float4 "<< KSL_METAL_SAMPLE_2D_ARRAY_LOD_NAME << "(texture2d_array<float> t, sampler s, float3 tc, float lod)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   return t.sample(s, tc.xy, uint(tc.z), level(lod));"; NewLine();
	m_result << "}"; NewLine();
	NewLine();

#if 0
	// sample 2d depth lod
	m_result << "float4 " << KSL_METAL_SAMPLE_2D_DEPTH_LOD_NAME << "(depth2d<float> t, float3 tc, float lod)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   return float4(t.sample_compare(" << KSL_SHADOW_SAMPLER_NAME << ", tc.xy, tc.z, level(lod))); "; NewLine();
	m_result << "}"; NewLine();
	NewLine();
	
	// sample 2d depth array lod
	m_result << "float4 "<< KSL_METAL_SAMPLE_2D_DEPTH_ARRAY_LOD_NAME << "(depth2d_array<float> t, float4 tc, float lod)"; NewLine();
	m_result << "{"; NewLine();
	m_result << "   return float4(t.sample_compare(" << KSL_SHADOW_SAMPLER_NAME << ", tc.xy, uint(tc.z), tc.w, level(lod)));"; NewLine();
	m_result << "}"; NewLine();
	NewLine();
#endif
}


bool  KSLMetalGenerator::VisitMetalUniformInterfaceNode(KSLMetalUniformInterfaceNode* muin)
{
	m_do_syncline = false;
	bool b = VisitStructDefinition(muin);
	m_do_syncline = true;
	return b;
}


bool KSLMetalGenerator::VisitVariableExpression(KSLVariableExpressionNode *ve)
{
	KSLVariable &v = ast->variables[ve->variable_id];

	std::string suffix;

	switch (v.storage_type)
	{
	case KSL_STORAGE_IN: m_result << KSL_METAL_INPUT_NAME << "."; break;
	case KSL_STORAGE_OUT: m_result << KSL_METAL_OUTPUT_NAME << "."; break;
	case KSL_STORAGE_UNIFORM:
		{
			KSLType base_type = v.type.IsArray() ? v.type.GetBaseType() : v.type;
			if (base_type.IsNumeric() || base_type.IsBool())
			{
				m_result << KSLMetalTranslator::NGLGroupToUniformIntefaceVariableName(m_mtl_translator->m_uniform_variable_id_to_group.at(ve->variable_id)) << ".";
			}
			else
			{
				suffix = KSL_METAL_GLOBAL_SUFFIX;
			}
		}
		break;
	case KSL_STORAGE_BUFFER:
	case KSL_STORAGE_SHARED:
		suffix = KSL_METAL_GLOBAL_SUFFIX;
	case KSL_STORAGE_DEFAULT:
		if (v.type.GetTypeClass() == KSL_TYPECLASS_IMAGE)
		{
			suffix = KSL_METAL_GLOBAL_SUFFIX;
		}
	case KSL_STORAGE_CONST:
		break;
	default:
		assert(0);
		return false;
	}

	bool s = KSLGenerator::VisitVariableExpression(ve);
	if (!m_visiting_main) m_result << suffix;
	return s;
}




bool KSLMetalGenerator::PrintBarrier(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "workgroupMemoryBarrierAll")
	{
		m_result << "threadgroup_barrier(mem_flags::mem_device_and_threadgroup)";
		return true;
	}
	else if (fce->name == "workgroupMemoryBarrierGlobal")
	{
		m_result << "threadgroup_barrier(mem_flags::mem_device)";
		return true;
	}
	else if (fce->name == "workgroupMemoryBarrierShared") 
	{
		m_result << "threadgroup_barrier(mem_flags::mem_threadgroup)";
		return true;
	}

	return false;
}


bool KSLMetalGenerator::PrintImageStore(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "imageStore")
	{
		KSLExpressionNode* texture_exp = fce->arguments[0];
		KSLExpressionNode* texcoord_exp = fce->arguments[1];
		KSLExpressionNode* value_exp = fce->arguments[2];

		VisitExpression(texture_exp);
		m_result << ".write(";
		VisitExpression(value_exp);
		m_result << ", uint2(";
		VisitExpression(texcoord_exp);
		m_result << "))";
		return true;
	}

	return false;
}


bool KSLMetalGenerator::PrintSubpassLoad(KSLFunctionCallExpressionNode* fce)
{
	if (fce->name == "subpassLoad")
	{
		VisitExpression(fce->arguments[0]);
		if (!m_mtl_subpass_as_framebuffer_fetch)
		{
			m_result << ".read(uint2(gl_FragCoord.xy))";
		}
		return true;
	}

	return false;
}


bool KSLMetalGenerator::PrintPow(KSLFunctionCallExpressionNode* fce)
{
    if (fce->name == "pow")
    {
        m_result << "powr(";
        VisitExpression(fce->arguments[0]);
        m_result << ",";
        VisitExpression(fce->arguments[1]);
        m_result << ")";
        return true;
    }
    
    return false;
}


bool KSLMetalGenerator::VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce)
{
	SyncLine(fce);

	if (PrintTextureAccess(fce)) return true;
	if (PrintBarrier(fce)) return true;
	if (PrintImageStore(fce)) return true;
	if (PrintSubpassLoad(fce)) return true;
    if (PrintPow(fce)) return true;

	// other functions
	m_result << fce->name << "(";

	for (size_t i = 0; i < fce->arguments.size(); i++)
	{
		KSLExpressionNode* ae = fce->arguments[i];
		VisitExpression(ae);

		if ((ae->node_type == KSL_NODE_VARIABLE_EXPRESSION) && ae->type.IsSampler())
		{
			m_result << ", ";
			VisitExpression(ae);
			m_result << KSL_METAL_SAMPLER_SUFFIX;
		}

		if (i + 1 < fce->arguments.size()) m_result << ",";
	}

	// print global arguments
	const std::string actual_suffix = (m_visiting_main) ? "" : KSL_METAL_GLOBAL_SUFFIX;
	{
		KSLGlobalUsageInfo &gui = m_mtl_translator->m_global_usage[fce->name];
		bool need_comma = fce->arguments.size() > 0;

		{
			const uint32_t NGL_GROUP_COUNT = 3;
			NGL_shader_uniform_group ngl_groups[NGL_GROUP_COUNT] = { NGL_GROUP_PER_DRAW, NGL_GROUP_PER_RENDERER_CHANGE, NGL_GROUP_MANUAL };

			for (uint32_t i = 0; i < NGL_GROUP_COUNT; i++)
			{
				NGL_shader_uniform_group ngl_group = ngl_groups[i];

				if (gui.UsedUniformGroup(ngl_group))
				{
					if (need_comma) m_result << ", ";
					m_result << KSLMetalTranslator::NGLGroupToUniformIntefaceVariableName(ngl_group);
					need_comma = true;
				}
			}
		}


		if (gui.use_in_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << KSL_METAL_INPUT_NAME;
			need_comma = true;
		}


		if (gui.use_out_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << KSL_METAL_OUTPUT_NAME;
			need_comma = true;
		}


		// Samplers
		{
			std::set<uint32_t>::iterator sampler_it = gui.used_samplers.begin();
			for (; sampler_it != gui.used_samplers.end(); sampler_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*sampler_it];
				m_result << v.new_name << actual_suffix << ", ";
				m_result << v.new_name << actual_suffix << KSL_METAL_SAMPLER_SUFFIX;

				need_comma = true;
			}
		}


		// Buffers
		{
			std::set<uint32_t>::iterator buffer_it = gui.used_buffers.begin();
			for (; buffer_it != gui.used_buffers.end(); buffer_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*buffer_it];
				m_result << v.new_name << actual_suffix;

				need_comma = true;
			}
		}


		// Shareds
		{
			std::set<uint32_t>::iterator shared_it = gui.used_shared.begin();
			for (; shared_it != gui.used_shared.end(); shared_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*shared_it];
				m_result << v.new_name << actual_suffix;

				need_comma = true;
			}
		}


		// Images
		{
			std::set<uint32_t>::iterator image_it = gui.used_images.begin();
			for (; image_it != gui.used_images.end(); image_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*image_it];

				m_result << v.new_name << actual_suffix;

				need_comma = true;
			}
		}

		// inbuilts
		{
			std::set<uint32_t>::iterator inbuilt_it = gui.used_inbuilts.begin();
			for (; inbuilt_it != gui.used_inbuilts.end(); inbuilt_it++)
			{
				if (need_comma) m_result << ", ";
				KSLVariable &v = ast->variables[*inbuilt_it];
				m_result << v.new_name;
				need_comma = true;
			}
		}
	}

	m_result << ")";
	return true;
}


bool  KSLMetalGenerator::VisitMetalInputOutputInterfaceNode(KSLMetalInputOutputIntefaceNode* mioin)
{
	SyncLine(mioin);

	m_result << "struct " << mioin->name; NewLine();
	m_result << "{";
	m_indent++;

	for (size_t i = 0; i < mioin->members.size(); i++)
	{
		NewLine();

		KSLVariableDefinitionsNode* vardef = mioin->members[i];
		assert(vardef->variables.size() == 1);

		m_result << TypeToString(vardef->variable_type) << " ";
		
		KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[0];
		KSLVariable &v = ast->variables[dn.variable_id];
		m_result << v.new_name;

		if (v.type.IsArray())
		{
			if ((dn.size_expression == NULL) && (vardef->storage_type != KSL_STORAGE_BUFFER))
			{
				assert(0);
				return false;
			}

			m_result << "[";
			if (dn.size_expression != NULL) VisitExpression(dn.size_expression);
			m_result << "]";
		}

		assert(dn.init_expressions.size() == 0);

		std::string &q = mioin->qualifiers[i];
		assert(q != "");
		m_result << " [[ " << q << " ]];";
	}

	m_indent--;
	NewLine();
	m_result << "};"; NewLine();

	return true;
}


bool KSLMetalGenerator::VisitFunction(KSLFunctionNode* fn)
{
	SyncLine(fn);

	KSLFunction &function = ast->functions[fn->function_id];

	m_result << TypeToString(function.return_type) << " " << function.function_name << "(";

	for (size_t i = 0; i < fn->attribs.size(); i++)
	{
		KSLFunctionNode::AttribNode &an = fn->attribs[i];

		KSLVariable &v = ast->variables[an.variable_id];

		switch (function.attrib_access[i])
		{
			case KSL_ATTRIB_ACCESS_IN:
				if(v.type.IsArray())
				{
					m_result<<"thread const ";
				}
				break;
			case KSL_ATTRIB_ACCESS_OUT:
			case KSL_ATTRIB_ACCESS_INOUT: m_result << "thread "; break;
			default: assert(0); break;
		}

		m_result << TypeToString(v.type.IsArray()?v.type.GetBaseType():v.type) << " ";

		if (v.type.IsArray())
		{
			m_result << "*";
		}
		else
		{
			switch (function.attrib_access[i])
			{
				case KSL_ATTRIB_ACCESS_IN: break;
				case KSL_ATTRIB_ACCESS_OUT:
				case KSL_ATTRIB_ACCESS_INOUT: m_result << "&"; break;
				default: assert(0); break;
			}
		}

		m_result << v.new_name;

		if (v.type.IsSampler())
		{
			m_result << ", sampler " << v.new_name << KSL_METAL_SAMPLER_SUFFIX;
		}

		if (i + 1 < fn->attribs.size()) m_result << ", ";
	}

	// print global arguments
	{
		KSLGlobalUsageInfo &gui = m_mtl_translator->m_global_usage[function.function_name];
		bool need_comma = fn->attribs.size() > 0;

		if (gui.use_in_attribs || gui.use_out_attribs || gui.use_in_attribs) NewLine();

		{
			const uint32_t NGL_GROUP_COUNT = 3;
			NGL_shader_uniform_group ngl_groups[NGL_GROUP_COUNT] = { NGL_GROUP_PER_DRAW, NGL_GROUP_PER_RENDERER_CHANGE, NGL_GROUP_MANUAL };
			
			for (uint32_t i = 0; i < NGL_GROUP_COUNT; i++)
			{
				NGL_shader_uniform_group ngl_group = ngl_groups[i];

				if (gui.UsedUniformGroup(ngl_group))
				{
					if (need_comma) m_result << ", ";
					m_result << "constant " << KSLMetalTranslator::NGLGroupToUniformIntefaceTypeName(ngl_group) << " &"
						<< KSLMetalTranslator::NGLGroupToUniformIntefaceVariableName(ngl_group);
					need_comma = true;
				}
			}
		}


		if (gui.use_in_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << "thread " << KSL_METAL_INPUT_LAYOUT_TYPE_NAME << " &" << KSL_METAL_INPUT_NAME;
			need_comma = true;
		}


		if (gui.use_out_attribs)
		{
			if (need_comma) m_result << ", ";
			m_result << "thread " << KSL_METAL_OUTPUT_LAYOUT_TYPE_NAME << " &" << KSL_METAL_OUTPUT_NAME;
			need_comma = true;
		}


		// Samplers
		{
			if (gui.used_samplers.size() > 0) NewLine();
			std::set<uint32_t>::iterator sampler_it = gui.used_samplers.begin();
			for (; sampler_it != gui.used_samplers.end(); sampler_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*sampler_it];
				m_result << TypeToString(v.type) << " " << v.new_name << KSL_METAL_GLOBAL_SUFFIX << ", ";
				m_result << "sampler " << v.new_name << KSL_METAL_GLOBAL_SUFFIX << KSL_METAL_SAMPLER_SUFFIX;

				need_comma = true;
			}
		}


		// Buffers
		{
			if (gui.used_buffers.size() > 0) NewLine();
			std::set<uint32_t>::iterator buffer_it = gui.used_buffers.begin();
			for (; buffer_it != gui.used_buffers.end(); buffer_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*buffer_it];
				m_result << ((ast->shader_type == NGL_COMPUTE_SHADER)?"device ":"constant ");
				if (v.type.IsArray())
				{
					m_result << TypeToString(v.type.GetBaseType()) << "*";
				}
				else
				{
					m_result << TypeToString(v.type) << "&";
				}
				m_result << " " << v.new_name << KSL_METAL_GLOBAL_SUFFIX;

				need_comma = true;
			}
		}


		// Shareds
		{
			if (gui.used_shared.size() > 0) NewLine();
			std::set<uint32_t>::iterator shared_it = gui.used_shared.begin();
			for (; shared_it != gui.used_shared.end(); shared_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*shared_it];
				
				const KSLType &t = (v.type.IsArray())?v.type.GetBaseType():v.type;
				m_result << "threadgroup " << TypeToString(t) << (v.type.IsArray() ? " *" : " &") << v.new_name << KSL_METAL_GLOBAL_SUFFIX;

				need_comma = true;
			}
		}


		// Images
		{
			if (gui.used_images.size() > 0) NewLine();
			std::set<uint32_t>::iterator image_it = gui.used_images.begin();
			for (; image_it != gui.used_images.end(); image_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*image_it];

				m_result << GetImageTypeName(v.type.id) << "<" << FloatPrecisionToString(v.type.precision);

				switch (v.access)
				{
				case KSL_ACCESS_READ_ONLY: m_result << ", access::read"; break;
				case KSL_ACCESS_WRITE_ONLY: m_result << ", access::write"; break;
				default:
					assert(0);
					m_result << ", access::error";
					break;
				}

				m_result << "> " << v.new_name << KSL_METAL_GLOBAL_SUFFIX;

				need_comma = true;
			}
		}

		// inbuilts
		{
			if (gui.used_inbuilts.size() > 0) NewLine();
			std::set<uint32_t>::iterator inbuilt_it = gui.used_inbuilts.begin();
			for (; inbuilt_it != gui.used_inbuilts.end(); inbuilt_it++)
			{
				if (need_comma) m_result << ", ";

				KSLVariable &v = ast->variables[*inbuilt_it];

				m_result << "thread " << TypeToString(v.type) << " &" << v.new_name;

				need_comma = true;
			}
		}
	}

	m_result << ")";
	NewLine();

	VisitBlockStatement(fn->body);

	return true;
}


bool KSLMetalGenerator::VisitMetalMainFunctionNode(KSLMetalMainFunctionNode* mmfn)
{
	uint32_t next_texture_binding_point = 0;
	uint32_t next_buffer_binding_point = 4;

	// function name
	{
		switch (ast->shader_type)
		{
		case NGL_VERTEX_SHADER:   m_result << "vertex "; break;
		case NGL_FRAGMENT_SHADER: m_result << "fragment "; break;
		case NGL_COMPUTE_SHADER:  m_result << "kernel "; break;
		default:
			assert(0);
			break;
		}

		m_result << (m_mtl_translator->m_has_output_interface?KSL_METAL_OUTPUT_LAYOUT_TYPE_NAME:"void");

		switch (ast->shader_type)
		{
		case NGL_VERTEX_SHADER:   m_result << " vertex_main("; break;
		case NGL_FRAGMENT_SHADER: m_result << " fragment_main("; break;
		case NGL_COMPUTE_SHADER:  m_result << " compute_main("; break;
		default:
			assert(0);
			break;
		}
	}
	
	m_indent++;
	bool need_comma = false;


	// input
	if (m_mtl_translator->m_has_input_interface)
	{
		if (need_comma) m_result << ",";
		NewLine();
		m_result << KSL_METAL_INPUT_LAYOUT_TYPE_NAME << " " << KSL_METAL_INPUT_NAME << " [[ stage_in ]]";
		need_comma = true;
	}

	
	// uniforms
	{ 
		const uint32_t NGL_GROUP_COUNT = 3;
		NGL_shader_uniform_group ngl_groups[NGL_GROUP_COUNT] = { NGL_GROUP_PER_DRAW, NGL_GROUP_PER_RENDERER_CHANGE, NGL_GROUP_MANUAL };

		for (uint32_t i = 0; i < NGL_GROUP_COUNT; i++)
		{
			NGL_shader_uniform_group ngl_group = ngl_groups[i];

			if (m_mtl_translator->m_has_uniform_interface.at(ngl_group))
			{
				if (need_comma) m_result << ",";
				NewLine();
				m_result << "constant " << KSLMetalTranslator::NGLGroupToUniformIntefaceTypeName(ngl_group) << " &"
					<< KSLMetalTranslator::NGLGroupToUniformIntefaceVariableName(ngl_group) << " [[ buffer("
					<< KSLMetalTranslator::NGLGroupToUniformIntefaceBinding(ngl_group) << ") ]]";
				need_comma = true;
			}
		}
	}


	// samplers
	for (size_t i = 0; i < mmfn->samplers.size(); i++)
	{
		if (need_comma) m_result << ",";
		NewLine();

		KSLVariableDefinitionsNode* s = mmfn->samplers[i];

		assert(s->variables.size() == 1);
		KSLVariable &v = ast->variables[s->variables[0].variable_id];

		m_result << TypeToString(s->variable_type) << " " << v.new_name << " [[ texture("<< next_texture_binding_point<< ") ]]";

		m_result << ",";
		NewLine();
		m_result << "sampler " << v.new_name << KSL_METAL_SAMPLER_SUFFIX << " [[ sampler(" << next_texture_binding_point << ") ]]";

		next_texture_binding_point++;

		need_comma = true;
	}


	// buffers
	for (size_t i = 0; i < mmfn->buffers.size(); i++)
	{
		if (need_comma) m_result << ",";
		NewLine();

		KSLVariableDefinitionsNode* b = mmfn->buffers[i];

		assert(b->variables.size() == 1);
		KSLVariable &v = ast->variables[b->variables[0].variable_id];

		m_result << ((ast->shader_type == NGL_COMPUTE_SHADER)?"device ":"constant ") << TypeToString(b->variable_type);
		m_result << (v.type.IsArray() ? " *" : " &");
		m_result << v.new_name << " [[ buffer(" << next_buffer_binding_point++ << ") ]]";

		need_comma = true;
	}


	// images
	for (size_t i = 0; i < mmfn->images.size(); i++)
	{
		if (need_comma) m_result << ",";
		NewLine();

		KSLImageDefinitionNode* id = mmfn->images[i];

		KSLVariable &v = ast->variables[id->variable_id];

		m_result << GetImageTypeName(v.type.id) << "<" << FloatPrecisionToString(v.type.precision) ;
		
		switch (v.access)
		{
		case KSL_ACCESS_READ_ONLY: m_result << ", access::read"; break;
		case KSL_ACCESS_WRITE_ONLY: m_result << ", access::write"; break;
		default:
			assert(0);
			m_result << ", access::error";
			break;
		}

		m_result << "> " << v.new_name << " [[ texture(" << next_texture_binding_point++ << ") ]]";

		need_comma = true;
	}


	// subpasses
	for (size_t i = 0; i < mmfn->subpass_inputs.size(); i++)
	{
		if (need_comma) m_result << ",";
		NewLine();

		KSLVariableDefinitionsNode* si = mmfn->subpass_inputs[i];

		assert(si->variables.size() == 1);
		KSLVariable &v = ast->variables[si->variables[0].variable_id];

		m_result << TypeToString(si->variable_type) << " " << v.new_name;

		if (!m_mtl_subpass_as_framebuffer_fetch)
		{
			m_result << " [[ texture(" << next_texture_binding_point << ") ]]";
			next_texture_binding_point++;
		}
		else
		{
			m_result << " [[ color(";
			if (si->int_attribs.find(KSL_ATTRIB_QUALIFIER_COLOR) != si->int_attribs.end())
			{
				m_result << si->int_attribs[KSL_ATTRIB_QUALIFIER_COLOR];
			}
			else if (si->int_attribs.find(KSL_ATTRIB_QUALIFIER_DEPTH) != si->int_attribs.end())
			{
				m_result << si->int_attribs[KSL_ATTRIB_QUALIFIER_DEPTH];
			}
			m_result << ") ]]";
		}
		
		need_comma = true;
	}


	// inbuilts variables
	if (ast->shader_type == NGL_COMPUTE_SHADER)
	{
		if (need_comma) m_result << ","; NewLine();
		m_result << "uint3 gl_WorkGroupID           [[  threadgroup_position_in_grid    ]],"; NewLine();
		m_result << "uint3 gl_LocalInvocationID     [[  thread_position_in_threadgroup  ]],"; NewLine();
		m_result << "uint3 gl_GlobalInvocationID    [[  thread_position_in_grid         ]],"; NewLine();
		m_result << "uint  gl_LocalInvocationIndex  [[  thread_index_in_threadgroup     ]],"; NewLine();
		m_result << "uint3 gl_NumWorkGroups         [[  threadgroups_per_grid           ]],"; NewLine();
		m_result << "uint3 gl_WorkGroupSize         [[  threads_per_threadgroup         ]]";
	}

	else if (ast->shader_type == NGL_FRAGMENT_SHADER)
	{
		if (need_comma) m_result << ","; NewLine();
		m_result << "float4 gl_FragCoord    [[  position      ]],"; NewLine();
		m_result << "bool   gl_FrontFacing  [[  front_facing  ]]";
	}
	
	else if (ast->shader_type == NGL_VERTEX_SHADER)
	{
		if (need_comma) m_result << ","; NewLine();
		m_result << "uint gl_VertexID [[ vertex_id ]]";		
	}

	m_indent--; NewLine();
	m_result << ")"; NewLine();

	m_visiting_main = true;
	VisitBlockStatement(mmfn->body);
	m_visiting_main = false;

	return true;
}


bool KSLMetalGenerator::VisitControlStatement(KSLControlStatementNode* cs)
{
	SyncLine(cs);

	if (cs->node_type == KSL_NODE_DISCARD_STATEMENT)
	{
		m_result << "discard_fragment();";
		return true;
	}

	return KSLGenerator::VisitControlStatement(cs);
}


bool KSLMetalGenerator::VisitGlobalSpaceNode(KSLASTNode* n)
{
	switch (n->node_type)
	{
	case KSL_NODE_METAL_UNIFORM_INTERFACE:
		return VisitMetalUniformInterfaceNode(dynamic_cast<KSLMetalUniformInterfaceNode*>(n));

	case KSL_NODE_METAL_INPUT_INTERFACE:
	case KSL_NODE_METAL_OUTPUT_INTERFACE:
		return VisitMetalInputOutputInterfaceNode(dynamic_cast<KSLMetalInputOutputIntefaceNode*>(n));

	case KSL_NODE_METAL_MAIN_FUNCTION:
		return VisitMetalMainFunctionNode(dynamic_cast<KSLMetalMainFunctionNode*>(n));

	case KSL_NODE_NUMTHREAD:
		return true;

	case KSL_NODE_VARIABLE_DEFINITIONS:
	{
		m_visiting_global_vdn = true;
		bool s = VisitVariableDefinitions(dynamic_cast<KSLVariableDefinitionsNode*>(n));
		m_visiting_global_vdn = false;
		return s;
		break;
	}
		
	default:
		return KSLGenerator::VisitGlobalSpaceNode(n);
	}

	return true;
}


bool KSLMetalGenerator::VisitReturnStatement(KSLReturnStatementNode* rs)
{
	SyncLine(rs);

	if (m_visiting_main && m_mtl_translator->m_has_output_interface)
	{
		assert(rs->expression == NULL);
		m_result << "return " << KSL_METAL_OUTPUT_NAME << ";";
		return true;
	}

	return KSLGenerator::VisitReturnStatement(rs);
}


std::string KSLMetalGenerator::TypeToString(KSLType type)
{
	CHECK_FORCE_HIGHP(type);

	std::string f_prec = FloatPrecisionToString(type.precision);
	std::string i_prec;

	switch (type.precision)
	{
	case KSL_PRECISION_HIGH:
		i_prec = "int";
		break;
	case KSL_PRECISION_MEDIUM:
		i_prec = "short";
		break;
	case KSL_PRECISION_LOW:
		i_prec = "char";
		break;
	case KSL_PRECISION_NONE:
		i_prec = "int_noprec";
		break;
	default:
		assert(0);
		break;
	}

	if (m_mtl_subpass_as_framebuffer_fetch)
	{
		switch (type.id)
		{
			case KSL_TYPE_SUBPASS_INPUT: return f_prec + "4";
			default: break;
		}
	}
	else
	{
		switch (type.id)
		{
			case KSL_TYPE_SUBPASS_INPUT: return "texture2d<" + f_prec + ">";
			default: break;
		}
	}

	switch (type.id)
	{
		// float
		case KSL_TYPE_FLOAT: return f_prec;

		// vectors
		case KSL_TYPE_VEC4:  return f_prec + "4";
		case KSL_TYPE_VEC3:  return f_prec + "3";
		case KSL_TYPE_VEC2:  return f_prec + "2";

		// matrices
		case KSL_TYPE_MAT4:  return f_prec + "4x4";
		case KSL_TYPE_MAT3:  return f_prec + "3x3";
		case KSL_TYPE_MAT2:  return f_prec + "2x2";

		case KSL_TYPE_INT:   return i_prec;
		case KSL_TYPE_UINT:  return "u" + i_prec;

		// Integer vectors
		case KSL_TYPE_INT4:  return i_prec + "4";
		case KSL_TYPE_INT3:  return i_prec + "3";
		case KSL_TYPE_INT2:  return i_prec + "2";

		case KSL_TYPE_UINT4: return "u" + i_prec + "4";
		case KSL_TYPE_UINT3: return "u" + i_prec + "3";
		case KSL_TYPE_UINT2: return "u" + i_prec + "2";

		// samplers
		case KSL_TYPE_SAMPLER_2D: return "texture2d<" + f_prec + ">";
		case KSL_TYPE_SAMPLER_2D_ARRAY: return "texture2d_array<" + f_prec + ">";
		case KSL_TYPE_SAMPLER_CUBE: return "texturecube<" + f_prec + ">";
		case KSL_TYPE_SAMPLER_CUBE_ARRAY: return "texturecube_array<" + f_prec + ">";

		// shadow samplers
		case KSL_TYPE_SAMPLER_2D_SHADOW: return "depth2d<" + f_prec + ">";
		case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW: return "depth2d_array<" + f_prec + ">";
		case KSL_TYPE_SAMPLER_CUBE_SHADOW: return "depthcube<" + f_prec + ">";

		default:
			return KSLGenerator::TypeToString(type);
	}

	return "error_type";
}


std::string KSLMetalGenerator::StorageQualifierToString(KSLStorageQualifier qualifier)
{
	switch (qualifier)
	{
	case KSL_STORAGE_SHARED:  return "threadgroup ";
	case KSL_STORAGE_CONST: return m_visiting_global_vdn ? "constant " : "const ";
	case KSL_STORAGE_IN:      
	case KSL_STORAGE_OUT:     
	case KSL_STORAGE_UNIFORM: 
	case KSL_STORAGE_BUFFER:  
	case KSL_STORAGE_DEFAULT: return "";
	default: assert(0); return "error_qualifer";
	}
}


std::string KSLMetalGenerator::GetImageTypeName(uint32_t t) const
{
	switch (t)
	{
	case KSL_TYPE_IMAGE2D: return "texture2d";

	default:
		break;
	}

	assert(0);
	return "error_type";
}

std::string KSLMetalGenerator::FloatPrecisionToString(KSLPrecision p) const
{
	switch (p)
	{
	case KSL_PRECISION_HIGH:
		return "float";
	case KSL_PRECISION_MEDIUM:
	case KSL_PRECISION_LOW:
		return "half";
	case KSL_PRECISION_NONE:
		return "float_noprec";
	default:
		break;
	}

	assert(0);
	return "error";
}


void KSLMetalGenerator::SetTranslator(KSLTranslator* translator)
{
	m_mtl_translator = dynamic_cast<KSLMetalTranslator*>(translator);
}

