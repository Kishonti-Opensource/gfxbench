/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#include "parser.h"
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <assert.h>

//http://stackoverflow.com/questions/17348086/c-set-console-text-color-to-rgb-value
//http://tudasbazis.sulinet.hu/hu/informatika/informatika/informatika-9-12-evfolyam/a-c-nyelv-alapelemei-jelkeszlet-azonositok-konstansok-operatorok/a-c-nyelv-alapelemek
//http://tigcc.ticalc.org/doc/opers.html#comma

#include "glslpp/glslpp.h"

#ifdef _MSC_VER
	#pragma warning( disable : 4996)
#endif

__inline bool IsRMAMetalFamily(NGL_api api)
{
	return (api == NGL_METAL_IOS) || (api == NGL_METAL_MACOS);
}

__inline bool IsRMAClassicGLFamily(NGL_api api)
{
	return (api == NGL_OPENGL) || (api == NGL_OPENGL_ES);
}

__inline bool IsRMAVulkan(NGL_api api)
{
	return (api == NGL_VULKAN);
}

__inline bool IsRMAD3DFamily(NGL_api api)
{
	return (api == NGL_DIRECT3D_11) || (api == NGL_DIRECT3D_12);
}


const char* GL_BUFFER_LAYOUT_INNER_DATA = "_inner_data_";

void _parser::Process()
{
	if (m_shader_type == NGL_COMPUTE_SHADER)
	{
		// Add the workgroup size before the pre-processor
		assert(m_workgroup_size[0] > 0);
		assert(m_workgroup_size[1] > 0);
		assert(m_workgroup_size[2] > 0);

		std::stringstream s;
		s << "#define WORKGROUP_SIZE_X " << m_workgroup_size[0] << std::endl;
		s << "#define WORKGROUP_SIZE_Y " << m_workgroup_size[1] << std::endl;
		s << "#define WORKGROUP_SIZE_Z " << m_workgroup_size[2] << std::endl;
		s << m_text;
		m_text = s.str();
	}

	std::string pp_log;
	PreProcessShaderCode(m_text,pp_log);

	if( m_shader_type == NGL_VERTEX_SHADER)
	{
		m_text = "out vec4 gl_Position;\n" + m_text;
	}
	if (m_shader_type == NGL_FRAGMENT_SHADER)
	{
		//if (m_api != RMA_OPENGL_4_X)
		{
			m_text = "out vec4 _Frag_Data7;\n" + m_text;
			m_text = "out vec4 _Frag_Data6;\n" + m_text;
			m_text = "out vec4 _Frag_Data5;\n" + m_text;
			m_text = "out vec4 _Frag_Data4;\n" + m_text;
			m_text = "out vec4 _Frag_Data3;\n" + m_text;
			m_text = "out vec4 _Frag_Data2;\n" + m_text;
			m_text = "out vec4 _Frag_Data1;\n" + m_text;
			m_text = "out vec4 _Frag_Data0;\n" + m_text;
		}
	}
	CreateReservedWord();
	CreateOperatorsPunctuators();
	CreateTypes();
	CreatemBuiltinFunctions();
	Tokenizer3();
	if (m_shader_type == NGL_FRAGMENT_SHADER)
	{
		//if (m_api != RMA_OPENGL_4_X)
		{
			DetectFragDataUsage();
		}
	}
	SearchTypesAndScopes();
	BuildStructs();
	Analyze();
	//PrintTokens( m_tokens);
	Alter();
	BuildFunctions();
	
	if (IsRMAMetalFamily(m_api))
	{
		InsertGlobalBufferParametersForMetal();
	}
}


void _parser::Print( std::string &result)
{
	std::stringstream s;
	
	if (IsRMAMetalFamily(m_api))
	{
		s << "#include <metal_stdlib>\n";
		s << "using namespace metal;\n\n";
	}

	
	if (m_shader_type == NGL_COMPUTE_SHADER)
	{
		// Add the workgroup size after the pre-processor
		s << "#define WORKGROUP_SIZE_X " << m_workgroup_size[0] << std::endl;
		s << "#define WORKGROUP_SIZE_Y " << m_workgroup_size[1] << std::endl;
		s << "#define WORKGROUP_SIZE_Z " << m_workgroup_size[2] << std::endl;
	}	

	//
	//	print structs
	//
	for (size_t i = 0; i < m_structs.size(); i++)
	{
		_struct &_s = m_structs[i];

		if (_s.type_indices.size() == 0)
		{
			s << "#error // PARSER: unable to parse struct\n\n";
			continue;
		}

		s << "struct " << m_types[_s.name_index].to_string << "\n";
		s << "{\n";
		for (size_t k = 0; k<_s.type_indices.size(); k++)
		{
			_multistring &t = m_types[_s.type_indices[k]];
			_variable &v = m_variables[_s.name_indices[k]];

			size_t count = _s.count[k];

			if (count == 0)
			{
				s << "\t" << t.to_string << " " << v.m_name << ";\n";
			}
			else
			{
				s << "\t" << t.to_string << " " << v.m_name << "[" << count << "];\n";
			}
			
		}
		s << "};\n\n";
	}


	if (IsRMAClassicGLFamily(m_api) || IsRMAVulkan(m_api))
	{
		if (m_uniforms.size() > 0)
		{
			if (IsRMAVulkan(m_api))
			{
				s << "layout(std140, binding = " << num_binding_points << ") uniform uniformObject" << m_shader_type << "\n{\n";
			}
			for( size_t k=0; k<m_uniforms.size(); k++)
			{
				_variable &v = m_uniforms[k];
				_multistring &type = m_types[v.m_type_idx];

				if (IsRMAVulkan(m_api))
				{
					s << "\t";
				}
				s << "uniform " << type.to_string << " " << v.m_name;
				if (v.size > 0)
				{
					s << "[" << v.size << "]";
				}
				s << ";\n";
			}
			if (IsRMAVulkan(m_api))
			{
				s << "};\n";
			}

			num_binding_points++;
		}

		s << "\n";

		for (size_t k = 0; k<m_samplers.size(); k++)
		{
			_variable &v = m_samplers[k];
			_multistring &type = m_types[v.m_type_idx];
			
			if (IsRMAVulkan(m_api))
			{
				s << "layout(binding = " << num_binding_points << ") ";
			}
			s << "uniform " << type.to_string << " " << v.m_name << ";\n";

			num_binding_points++;
		}

		s << "\n";


		//
		//	Print buffers
		//
		for (size_t k = 0; k < m_buffers.size(); k++)
		{
			_variable &v = m_buffers[k];
			_multistring &type = m_types[v.m_type_idx];

			s << "layout(std430";
			s << ", binding = " << num_binding_points;
			s << ") buffer " << v.m_name << "\n";
			s << "{\n";
			if (v.size == 0)
			{
				s << "  " << type.to_string << " " << v.m_name << GL_BUFFER_LAYOUT_INNER_DATA << ";\n";
			}
			else
			{
				s << "  " << type.to_string << " " << v.m_name << GL_BUFFER_LAYOUT_INNER_DATA;

				if (v.size > -1)
				{
					s << "[" << v.size << "];\n";
				}
				else
				{
					s << "[];\n";
				}

			}
			s << "};\n\n";

			num_binding_points++;
		}
		s << "\n";


		//
		//	Print images
		//
		for (size_t k = 0; k < m_images.size(); k++)
		{
			_variable &v = m_images[k];
			//_multistring &type = m_types[v.m_type_idx];

			s << "layout(rgba8";
			s << ", binding = " << num_image_binding_points;
			s << ") uniform writeonly image2D " << v.m_name << ";\n";

			num_image_binding_points++;
		}
		s << "\n";


		//
		//	Print shared variables
		//
		for (size_t k = 0; k < m_shared.size(); k++)
		{
			_variable &v = m_shared[k];
			_multistring &type = m_types[v.m_type_idx];
			
			s << "shared " << type.to_string << " " << v.m_name;
			if (v.size > 0)
			{
				s << "[" << v.size << "]";
			}
			s << ";\n";
		}
		s << '\n';

		//
		//	Print global variables
		//
		for (size_t k = 0; k < m_globals.size(); k++)
		{
			_variable &v = m_globals[k];
			_multistring &type = m_types[v.m_type_idx];
			
			s << type.to_string << " " << v.m_name;
			if (v.size > 0)
			{
				s << "[" << v.size << "]";
			}
			s << ";\n";
		}
		s << '\n';

		for( size_t k=0; k<m_ins.size(); k++)
		{
			_variable &v = m_ins[k];
			_multistring &type = m_types[v.m_type_idx];

			if (IsRMAVulkan(m_api) || m_api == NGL_OPENGL || m_api == NGL_OPENGL_ES)
			{
				s << "layout (location = " << k << ") ";
			}
			s << "in " << type.to_string << " " << v.m_name << ";\n";
		}

		s << "\n";

		int id = 0;
		for( size_t k=0; k<m_outs.size(); k++)
		{
			_variable &v = m_outs[k];
			_multistring &type = m_types[v.m_type_idx];

			if( v.m_name == "gl_Position")
			{
				continue;
			}
			if (IsRMAVulkan(m_api) || m_api == NGL_OPENGL || m_api == NGL_OPENGL_ES)
			{
				s << "layout (location = " << id++ << ") ";
			}
			
			s << "out " << type.to_string << " " << v.m_name << ";\n";
		}

		s << "\n";
	}
	if (IsRMAD3DFamily(m_api))
	{
		size_t buffer_rid = 0;
		size_t texture_rid = 0;
		size_t sampler_rid = 0;
		size_t uav_rid = 0;

		//
		//	Print uniforms
		//
		if( m_uniforms.size())
		{
			s << "cbuffer cb0 : register(b" << buffer_rid << ")\n";
			s << "{\n";
			for( size_t k=0; k<m_uniforms.size(); k++)
			{
				_variable &v = m_uniforms[k];
				_multistring &type = m_types[v.m_type_idx];

				s << "\t" << type.to_string << " " << v.m_name;

				if (v.size > 0)
				{
					s << "[" << v.size << "]";
				}
				s << ";\n";
			}
			if (m_shader_type == NGL_COMPUTE_SHADER)
			{
				s << "\tuint4 gl_NumWorkGroups;\n";
			}

			buffer_rid++;

			s << "};\n";
		}

		//
		//	Print samplers
		//
		for (size_t k = 0; k<m_samplers.size(); k++)
		{
			_variable &v = m_samplers[k];
			_multistring &type = m_types[v.m_type_idx];

			s << "SamplerState " << v.m_name << "_sampler" << " : register(s" << sampler_rid << ")" << ";\n";
			s << type.to_string << " " << v.m_name << " : register(t" << texture_rid << ")" << ";\n";

			sampler_rid++;
			texture_rid++;
		}
		s << "\n";

		//
		//	Print buffers
		//
		for (size_t k = 0; k < m_buffers.size(); k++)
		{
			_variable &v = m_buffers[k];
			_multistring &type = m_types[v.m_type_idx];
			
			if (m_shader_type == NGL_FRAGMENT_SHADER)
			{
				s << "StructuredBuffer<" << type.to_string << "> " << v.m_name << " : register(t" << texture_rid << ");\n";
				texture_rid++;
			}
			else
			{
				s << "RWStructuredBuffer<" << type.to_string << "> " << v.m_name << " : register(u" << uav_rid << ");\n";
				uav_rid++;
			}
		}
		s << "\n";


		//
		//	Print images
		//
		for (size_t k = 0; k < m_images.size(); k++)
		{
			_variable &v = m_images[k];
			s << "RWTexture2D<float4> " << v.m_name << " : register(u" << uav_rid << ");\n";
			uav_rid++;
		}
		s << "\n";


		//
		//	Print shared variables
		//
		for (size_t k = 0; k < m_shared.size(); k++)
		{
			_variable &v = m_shared[k];
			_multistring &type = m_types[v.m_type_idx];
			
			s << "groupshared " << type.to_string << " " << v.m_name;
			if (v.size > 0)
			{
				s << "[" << v.size << "]";
			}
			s << ";\n";
		}
		s << "\n";

		//
		//	Print global variables. Treat them as static in HLSL.	
		//
		for (size_t k = 0; k < m_globals.size(); k++)
		{
			_variable &v = m_globals[k];
			_multistring &type = m_types[v.m_type_idx];
			
			s << "static " << type.to_string << " " << v.m_name;
			if (v.size > 0)
			{
				s << "[" << v.size << "]";
			}
			s << ";\n";
		}
		s << "\n";

		if (m_shader_type == NGL_VERTEX_SHADER || m_shader_type == NGL_FRAGMENT_SHADER)
		{
			s << "struct Input\n";
			s << "{\n";
			if (m_shader_type == NGL_FRAGMENT_SHADER)
			{
				s << "\tfloat4 gl_Position:SV_POSITION;\n";
			}
			for (size_t k = 0; k < m_ins.size(); k++)
			{
				_variable &v = m_ins[k];
				_multistring &type = m_types[v.m_type_idx];
				s << "\t" << type.to_string << " " << v.m_name << ":" << v.m_name << ";\n";
			}
			s << "};\n";

			s << "struct Output\n";
			s << "{\n";
			for (size_t k = 0; k < m_outs.size(); k++)
			{
				_variable &v = m_outs[k];
				_multistring &type = m_types[v.m_type_idx];
				std::string semantic;

				if (m_shader_type == NGL_VERTEX_SHADER)
				{
					if (v.m_name == "gl_Position")
					{
						semantic = "SV_POSITION";
					}
					else
					{
						semantic = v.m_name;
					}
				}
				if (m_shader_type == NGL_FRAGMENT_SHADER)
				{
					char output_semantic[128];
					sprintf(output_semantic, "SV_TARGET%zu", k);
					semantic = output_semantic;
				}

				s << "\t" << type.to_string << " " << v.m_name << ":" << semantic << ";\n";
			}
			s << "};\n";
		}
	}

	if (IsRMAMetalFamily(m_api))
	{
		if( m_uniforms.size())
		{
			s << "struct Uniforms\n";
			s << "{\n";
			for( size_t k=0; k<m_uniforms.size(); k++)
			{
				_variable &v = m_uniforms[k];
				_multistring &type = m_types[v.m_type_idx];

				s << "\t" << type.to_string << " " << v.m_name;
				
				if (v.size > 0)
				{
					s << "[" << v.size << "]";
				}
				
				s << ";\n";
			}
			s << "};\n";
		}
			
		if (m_shader_type != NGL_COMPUTE_SHADER)
		{
			s << "struct Input\n";
			s << "{\n";
			if( m_shader_type == NGL_FRAGMENT_SHADER)
			{
				s << "\tfloat4 gl_Position [[position]];\n";
			}
			for( size_t k=0; k<m_ins.size(); k++)
			{
				_variable &v = m_ins[k];
				_multistring &type = m_types[v.m_type_idx];
				if( m_shader_type == NGL_VERTEX_SHADER)
				{
					s << "\t" << type.to_string << " " << v.m_name << " [[attribute(" << k << ")]];\n";
				}
				if( m_shader_type == NGL_FRAGMENT_SHADER)
				{
					s << "\t" << type.to_string << " " << v.m_name << " [[user(" << v.m_name << ")]];\n";
				}
			}
			s << "};\n";

			s << "struct Output\n";
			s << "{\n";
			for( size_t k=0; k<m_outs.size(); k++)
			{
				_variable &v = m_outs[k];
				_multistring &type = m_types[v.m_type_idx];
				std::string semantic;

				if( m_shader_type == NGL_VERTEX_SHADER)
				{
					if( v.m_name == "gl_Position")
					{
						semantic = "[[position]]";
					}
					else
					{
						semantic = "[[user(" + v.m_name + ")]]";
					}
				}
				if( m_shader_type == NGL_FRAGMENT_SHADER)
				{
					char output_semantic[128];
					sprintf( output_semantic, "[[color(%zu)]]", k);
					semantic = output_semantic;
				}

				s << "\t" << type.to_string << " " << v.m_name << " " << semantic << ";\n";
			}
			s << "};\n";
		}
	}

	//functions
	{
		for (size_t f = 0; f < m_functions.size(); f++)
		{
			_function* func = m_functions[f];

			size_t loop_start_ind = 0;

			if (func == m_main_function)
			{
				loop_start_ind = func->num_args + 5;

				if ((m_shader_type == NGL_COMPUTE_SHADER) && IsRMAD3DFamily(m_api))
				{
					s << "[numthreads(WORKGROUP_SIZE_X, WORKGROUP_SIZE_Y, WORKGROUP_SIZE_Z)]\n";
					s << "void main(uint3 gl_WorkGroupID : SV_GroupID, uint gl_LocalInvocationIndex : SV_GroupIndex, uint3 gl_GlobalInvocationID : SV_DispatchThreadID)\n";
					s << "{\n";
					s << "\t";
				}
				if (IsRMAClassicGLFamily(m_api) || IsRMAVulkan(m_api))
				{
					if (m_shader_type == NGL_COMPUTE_SHADER)
					{
						s << "layout (local_size_x = WORKGROUP_SIZE_X, local_size_y = WORKGROUP_SIZE_Y, local_size_z = WORKGROUP_SIZE_Z) in;";
						s << '\n';
					}

					s << "void main()\n";
					s << "{\n";
					s << "\t";
				}
				if (IsRMAD3DFamily(m_api))
				{
					if (m_shader_type == NGL_VERTEX_SHADER)
					{
						s << "Output vertex_main( Input input, uint gl_VertexID : SV_VertexID)\n";
						s << "{\n";
						s << "\tOutput output = (Output)0;\n\t";
					}
					if (m_shader_type == NGL_FRAGMENT_SHADER)
					{
						s << "Output fragment_main( Input input)\n";
						s << "{\n";
						s << "\tOutput output = (Output)0;\n\t";
					}
				}
				
				if (IsRMAMetalFamily(m_api))
				{
					bool need_comma = false;
					
					if (m_shader_type == NGL_VERTEX_SHADER)
					{
						s << "vertex Output vertex_main( Input input [[stage_in]]";
						need_comma = true;
					}
					if (m_shader_type == NGL_FRAGMENT_SHADER)
					{
						s << "fragment Output fragment_main( Input input [[stage_in]]";
						need_comma = true;
					}
					if (m_shader_type == NGL_COMPUTE_SHADER)
					{
						s << "kernel void compute_main(\n";
					}

					if (m_uniforms.size())
					{
						if (need_comma) s << ", ";
						s << "constant Uniforms &uniforms [[buffer(1)]]\n";
						need_comma = true;
					}

					uint32_t texture_binding_point = 0;
					for (size_t k = 0; k < m_samplers.size(); k++)
					{
						_variable &v = m_samplers[k];
						_multistring &type = m_types[v.m_type_idx];
						if (need_comma) s << ", ";
						s << type.to_string << " " << v.m_name << " [[texture(" << texture_binding_point << ")]]\n";
						s << ", sampler " << v.m_name << "_sampler [[sampler(" << texture_binding_point << ")]]\n";
						need_comma = true;
						texture_binding_point++;
					}
					
					//
					//	Print buffers
					//
					for (size_t k = 0; k < m_buffers.size(); k++)
					{
						_variable &v = m_buffers[k];
						_multistring &type = m_types[v.m_type_idx];
						
						if (need_comma) s << ", ";
						const char *buffer_type = (v.size == 0)?" &":" *" ;
						
						std::string address_space = "";
						if ( (v.size == 0) && m_shader_type == NGL_FRAGMENT_SHADER )
						{
							address_space = "constant" ;
						}
						else
						{
							address_space = "device";
						}
						
						s << address_space << " " << type.to_string << buffer_type << v.m_name << "[[buffer(" << k+2 << ")]]\n";
						need_comma = true;
					}
					
					//
					//	Print buffers
					//
					for (size_t k = 0; k < m_images.size(); k++)
					{
						_variable &v = m_images[k];
						//_multistring &type = m_types[v.m_type_idx];
						
						if (need_comma) s << ", ";
						s << "texture2d<float, access::write> " << v.m_name << " [[texture(" << texture_binding_point << ")]]\n";
						need_comma = true;
						texture_binding_point++;
					}
					
					//
					//  Print compute constants
					//
					if (m_shader_type == NGL_COMPUTE_SHADER)
					{
						if (need_comma) s << ", ";
						s <<   "uint3 gl_WorkGroupID           [[  threadgroup_position_in_grid    ]]\n";
						s << ", uint3 gl_LocalInvocationID     [[  thread_position_in_threadgroup  ]]\n";
						s << ", uint3 gl_GlobalInvocationID    [[  thread_position_in_grid         ]]\n";
						s << ", uint  gl_LocalInvocationIndex  [[  thread_index_in_threadgroup     ]]\n";
						s << ", uint3 gl_NumWorkGroups         [[  threadgroups_per_grid           ]]\n";
						s << ", uint3 gl_WorkGroupSize         [[  threads_per_threadgroup         ]]\n";
					}
					if (m_shader_type == NGL_VERTEX_SHADER)
					{
						if (need_comma) s << ", ";
						s << "uint gl_VertexID [[  vertex_id  ]]" ;
					}

					s << ")";
					s << "{\n";
					
					//
					//	Print global variables
					//
					for (size_t k = 0; k < m_globals.size(); k++)
					{
						_variable &v = m_globals[k];
						_multistring &type = m_types[v.m_type_idx];
						
						s << type.to_string << " " << v.m_name;
						if (v.size > 0)
						{
							s << "[" << v.size << "]";
						}
						s << ";\n";
					}
					s << '\n';
					
					//
					//	Print shared variables
					//
					for (size_t k = 0; k < m_shared.size(); k++)
					{
						_variable &v = m_shared[k];
						_multistring &type = m_types[v.m_type_idx];
						
						s << "threadgroup " << type.to_string << " " << v.m_name;
						if (v.size > 0)
						{
							s << "[" << v.size << "]";
						}
						s << ";\n";
					}
					s << '\n';
					
					if (m_shader_type != NGL_COMPUTE_SHADER)
					{
						s << "\tOutput output;\n\t";
					}
				}
			}

			for (size_t i = loop_start_ind; i < func->num_tokens + func->num_args + 5; i++)
			{
				size_t k = func->start_token_idx + i;

				_token &t = m_tokens[k];
				//_token &t_p = m_tokens[k - 1];

				if (std::find(
					m_swapped_tokens_first.begin(), 
					m_swapped_tokens_first.end(), 
					k) != m_swapped_tokens_first.end())
				{
					s << " /*swap happened*/ ";
				}

				if (t.m_type == 0)
				{
					s << m_reserverd_words[t.m_table_idx].to_string << " ";
					
					if ( IsRMAD3DFamily(m_api) || IsRMAMetalFamily(m_api) )
					{
						if (t.IsType(TOKEN_RETURN) && (func == m_main_function) && (m_shader_type != NGL_COMPUTE_SHADER))
						{
							s << "output";
						}
					}
				}
				else if (t.m_type == 1)
				{
					if (func == m_main_function)
					{
						if (IsRMAMetalFamily(m_api))
						{
							if (m_variables[t.m_table_idx].m_storage_type == 0)
							{
								s << "uniforms.";
							}
						}

						if (IsRMAD3DFamily(m_api) || IsRMAMetalFamily(m_api))
						{
							if (m_variables[t.m_table_idx].m_storage_type == 2)
							{
								s << "output.";
							}
							if (m_variables[t.m_table_idx].m_storage_type == 1)
							{
								s << "input.";
							}
						}
					}

					_variable &v = m_variables[t.m_table_idx];

					bool is_buffer = (m_tokens[k - 2].m_type == 0) && (m_tokens[k - 2].m_table_idx == TOKEN_BUFFER);
					if (IsRMAD3DFamily(m_api) && is_buffer && v.size > 0)
					{
						s << "> ";
					}

					s << v.m_name << " ";

					if ((IsRMAMetalFamily(m_api) || IsRMAD3DFamily(m_api)) && is_buffer)
					{
						if (v.size > 0) i += 3;
					}

					if (v.m_storage_type == 4 && v.size == 0 && IsRMAD3DFamily(m_api))
					{
						s << "[0]";
					}

					if (v.m_type_idx == SAMPLER2D)
					{
						if (IsRMAD3DFamily(m_api) || IsRMAMetalFamily(m_api))
						{
							//in parameter list
							if (i < func->num_args + 5)
							{
								if (IsRMAMetalFamily(m_api))
								{
									s << " , sampler ";
								}
								else
								{
									s << " , SamplerState ";
								}
								s << v.m_name << "_sampler ";
							}
							else
							{
								_token &tn2 = m_tokens[k+2] ;
								bool is_sampling = tn2.IsType(HLSL_TEXTURE) || tn2.IsType(HLSL_TEXTURE_LOD) || tn2.IsType(METAL_TEXTURE) ;
								
								if (!is_sampling)
								{
									s << " , ";
									s << v.m_name << "_sampler ";
								}
							}
						}
					}
				}
				else if (t.m_type == 2)
				{
					_number& num = m_numbers[t.m_table_idx];
					if (num.m_type == 0)
					{
						s << std::showpoint << num.m_num << " ";
					}
					else
					{
						s << std::showpoint << uint32_t(num.m_num) << " ";
					}
				}
				else if (t.m_type == 4)
				{
					s << m_types[t.m_table_idx].to_string << " ";
				}
				else if (t.m_type == 5)
				{
					s << m_operators_punctuators[t.m_table_idx].to_string << " ";
					if (t.IsType(SEMICOLON) || t.IsType(LEFT_BRACE) || t.IsType(RIGHT_BRACE))
					{
						s << "\n\t";
					}
				}
				else if (t.m_type == 6)
				{
					_multistring bf = m_builtin_functions[t.m_table_idx];
					s << bf.to_string << " ";
					if (bf.from_string != "" && bf.to_string == "")
					{
						i += 3; //skip "();"
					}
				}
			}
			if ((func == m_main_function) && (m_shader_type != NGL_COMPUTE_SHADER))
			{
				if (IsRMAD3DFamily(m_api) || IsRMAMetalFamily(m_api))
				{
					s << "return output;\n";
				}
			}
			s << "}\n\n";
		}
	}
	result = s.str();
}


bool _parser::GetVariableSize(size_t start_token_id, _variable &v)
{
	size_t s = start_token_id;

	// only one int literal must be placed between the brackets
	if (m_tokens[s + 0].IsType(LEFT_BRACKET) && m_tokens[s + 2].IsType(RIGHT_BRACKET) && m_tokens[s + 3].IsType(SEMICOLON))
	{
		v.size = (int)m_numbers[m_tokens[s + 1].m_table_idx].m_num;
	}
	else if (m_tokens[s + 0].IsType(LEFT_BRACKET) && m_tokens[s + 1].IsType(RIGHT_BRACKET) && m_tokens[s + 2].IsType(SEMICOLON))
	{
		v.size = -1;
	}
	else if (m_tokens[s + 0].IsType(SEMICOLON))
	{
		v.size = 0;
	}
	else
	{
		return false;
	}

	return true;
}


void _parser::Analyze()
{
	for( size_t k=0; k<m_tokens.size(); k++)
	{
		_token &t0 = m_tokens[k];

		if( t0.IsType( TOKEN_UNIFORM) || t0.IsType( TOKEN_IN) || t0.IsType( TOKEN_OUT) ||
			t0.IsType( TOKEN_BUFFER) || t0.IsType( TOKEN_IMAGE) || t0.IsType( TOKEN_SHARED) || t0.IsType( TOKEN_GLOBAL))
		{
			_token &t1 = m_tokens[k+1];
			if( t1.m_type == 4)
			{
				_token &t2 = m_tokens[k+2];
				if( t2.m_type == 1)
				{
					_variable &v = m_variables[t2.m_table_idx];
					bool b = GetVariableSize(k + 3, v);
					if (!b) continue;

					if( t0.IsType( TOKEN_UNIFORM))
					{
						if (t1.IsType(SAMPLER2D) || t1.IsType(SAMPLER2DShadow) || t1.IsType(SAMPLER2DArray) || t1.IsType(SAMPLERCUBE))
						{
							v.m_storage_type = 3;
							m_samplers.push_back( v);
						}
						else
						{
							v.m_storage_type = 0;
							m_uniforms.push_back( v);
						}
					}
					if( t0.IsType( TOKEN_IN))
					{
						v.m_storage_type = 1;
						m_ins.push_back( v);
					}
					if( t0.IsType( TOKEN_OUT))
					{
						v.m_storage_type = 2;

						if (m_shader_type == NGL_FRAGMENT_SHADER)
						{
							std::string frag_data = "_Frag_Data";
							size_t l = v.m_name.find(frag_data);
							if (l != std::string::npos)
							{
								uint32_t id = std::atoi(v.m_name.substr(frag_data.size()).c_str());
								if (m_fragdata_indices.find(id) != m_fragdata_indices.end())
								{
									m_outs.push_back(v);
								}
							}
							else
							{
								m_outs.push_back(v);
							}
						}
						else
						{
							m_outs.push_back( v);
						}

					}
					if (t0.IsType(TOKEN_BUFFER))
					{
						v.m_storage_type = 4;
						m_buffers.push_back(v);
					}
					if (t0.IsType(TOKEN_IMAGE))
					{
						v.m_storage_type = 7;
						m_images.push_back(v);
					}
					if (t0.IsType(TOKEN_SHARED))
					{
						v.m_storage_type = 5;
						m_shared.push_back(v);
					}
					if (t0.IsType(TOKEN_GLOBAL))
					{
						v.m_storage_type = 6;
						m_globals.push_back(v);
					}
				}
			}
		}

#if 0
		if( t0.IsType( RIGHT_PARENT))
		{
			_token &t1 = m_tokens[k+1];
			if( t1.IsType( LEFT_BRACE))
			{
				size_t k1 = k;

				for(; k1>1; k1--)
				{
					_token &t2 = m_tokens[k1];
					if( t2.IsType( LEFT_PARENT))
					{
						break;
					}
				}
				int scope_depth = 0;
				size_t k2 = k+1;
				for(; k2<m_tokens.size(); k2++)
				{
					_token &t2 = m_tokens[k2];

					if( t2.IsType( LEFT_BRACE))
					{
						scope_depth++;
					}
					if( t2.IsType( RIGHT_BRACE))
					{
						scope_depth--;
					}
					if( scope_depth == 0)
					{
						break;
					}
				}

				_function *f = new _function;

				f->start_token_idx = k1 - 2;
				f->num_args = k - k1 - 1;
				f->num_tokens = k2 - k - 2;

				_token &t3 = m_tokens[f->start_token_idx+1];
				if (t3.m_type == 1)
				{
					_token &t2 = m_tokens[f->start_token_idx];

					if( t2.IsType( VOID))
					{
						_variable &v = m_variables[t3.m_table_idx];
						if( v.m_name == "main")
						{
							m_main_function = f;
						}
					}

					m_functions.push_back( f);
				}
			}
		}
#endif
	}

#if 0
	for( size_t i=0; i<m_blocks.size(); i++)
	{
		_block *b = m_blocks[i];
		for( size_t j=0; j<b->m_statements.size(); j++)
		{
			_statement *s = b->m_statements[j];
			for( size_t k=0; k<s->m_tokens.size(); k++)
			{
				_token &t = s->m_tokens[k];

				if( t.m_type == 0)
				{
					printf("%s ", m_reserverd_words[t.m_table_idx].from_string.c_str());
				}
				else if( t.m_type == 1)
				{
					printf("%s ", m_variables[t.m_table_idx].m_name.c_str());
				}
				else if( t.m_type == 2)
				{
					printf("%g ", m_numbers[t.m_table_idx]);
				}
			}
			if( s->m_tokens.size())
				printf(";\n");
		}
	}
#endif
}


void _parser::BuildFunctions()
{
	for (size_t k = 0; k < m_tokens.size(); k++)
	{
		_token &t0 = m_tokens[k];
		if (t0.IsType(RIGHT_PARENT))
		{
			_token &t1 = m_tokens[k + 1];
			if (t1.IsType(LEFT_BRACE))
			{
				size_t k1 = k;

				for (; k1 > 1; k1--)
				{
					_token &t2 = m_tokens[k1];
					if (t2.IsType(LEFT_PARENT))
					{
						break;
					}
				}
				int scope_depth = 0;
				size_t k2 = k + 1;
				for (; k2 < m_tokens.size(); k2++)
				{
					_token &t2 = m_tokens[k2];

					if (t2.IsType(LEFT_BRACE))
					{
						scope_depth++;
					}
					if (t2.IsType(RIGHT_BRACE))
					{
						scope_depth--;
					}
					if (scope_depth == 0)
					{
						break;
					}
				}


				_token &t3 = m_tokens[k1 - 1];
				if (t3.m_type == 1)
				{
					_function *f = new _function;

					f->start_token_idx = k1 - 2;
					f->num_args = k - k1 - 1;
					f->num_tokens = k2 - k - 2;

					_token &t2 = m_tokens[f->start_token_idx];

					if (t2.IsType(VOID))
					{
						_variable &v = m_variables[t3.m_table_idx];
						if (v.m_name == "main")
						{
							m_main_function = f;
						}
					}

					m_functions.push_back(f);
				}
			}
		}
	}
}


void _parser::DetectFragDataUsage()
{
	for (uint32_t i = 0; i < m_tokens.size(); i++)
	{
		//detect gl_FragData[id]=
		if (i +5 < m_tokens.size() &&
			m_tokens[i].m_type == 1 &&
			m_tokens[i+1].m_type == 5 && 
			m_tokens[i+2].m_type == 2 && 
			m_tokens[i+3].m_type == 5 && 
			m_tokens[i+4].m_type == 5)
		{
			_variable &v = m_variables[m_tokens[i].m_table_idx];
			if (v.m_name == "gl_FragData")
			{
				uint32_t id = uint32_t(m_numbers[m_tokens[i + 2].m_table_idx].m_num);
				m_fragdata_indices.insert(id);

				//swap gl_FragData[i] to gl_FragDatai
				m_tokens.erase(m_tokens.begin() + i + 3);
				m_tokens.erase(m_tokens.begin() + i + 2);
				m_tokens.erase(m_tokens.begin() + i + 1);

				_variable new_v;
				char cstr_id[4];
				sprintf(cstr_id, "%d", id);
				if (IsRMAClassicGLFamily(m_api) || IsRMAVulkan(m_api))
				{
					new_v.m_name = std::string("_Frag_Data") + cstr_id;
				}
				else
				{
					new_v.m_name = std::string("output._Frag_Data") + cstr_id;
				}
				m_variables.push_back(new_v);
				m_tokens[i].m_table_idx = m_variables.size() - 1;
			}
		}
	}
}


void _parser::PrintTokens( std::vector<_token> &tokens)
{
#if 0
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	for( size_t k=0; k<tokens.size(); k++)
	{
		_token &t = tokens[k];

		if( t.m_type == 0)
		{
			SetConsoleTextAttribute( hConsole, 12);
			printf("%s ", m_reserverd_words[t.m_table_idx].to_string.c_str());
		}
		else if( t.m_type == 1)
		{
			SetConsoleTextAttribute( hConsole, 7);
			if( m_variables[t.m_table_idx].m_type_idx > -1)
				SetConsoleTextAttribute( hConsole, 5);
			if (m_variables[t.m_table_idx].m_type_idx >= NUM_OF_PRIMITIVE_TYPES)
				SetConsoleTextAttribute( hConsole, 13);

			printf("%s ", m_variables[t.m_table_idx].m_name.c_str());
		}
		else if( t.m_type == 2)
		{
			SetConsoleTextAttribute( hConsole, 2);
			printf("%f ", m_numbers[t.m_table_idx]);
		}
		else if( t.m_type == 3)
		{
			//SetConsoleTextAttribute( hConsole, 2);
			//printf("%s\n", m_macros[t.m_table_idx].c_str());
			printf("\n");
		}
		else if( t.m_type == 4)
		{
			SetConsoleTextAttribute( hConsole, 3);
			if( t.m_table_idx > 14)
				SetConsoleTextAttribute( hConsole, 11);
			printf("%s ", m_types[t.m_table_idx].from_string.c_str());
		}
		else if( t.m_type == 5)
		{
			SetConsoleTextAttribute( hConsole, 14);
			printf("%s ", m_operators_punctuators[t.m_table_idx].from_string.c_str());
			if( t.IsType( SEMICOLON) || t.IsType( LEFT_BRACE) || t.IsType( RIGHT_BRACE))
			{
				printf("\n");
			}
		}
	}
#endif
}


unsigned char C_signs[] = 
{
	'!', '#', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '/', ':', ';', '<', '=', '>', '?', '[', '\\', ']', '^', '_', '{', '|', '}', '~'
};


static bool is_C_sign( unsigned char c)
{
	for( unsigned i = 0; i < sizeof(C_signs); ++i)
	{
		if( c == C_signs[i])
		{
			return true;
		}
	}
	
	return false;
}


void _parser::Alter()
{
	for( size_t k=0; k<m_tokens.size(); k++)
	{
		_token &t0 = m_tokens[k];

		if( IsRMAMetalFamily(m_api))
		{
			if (t0.IsType(BARRIER) || t0.IsType(MEMORY_BARRIER_SHARED) || t0.IsType(MEMORY_BARRIER))
			{
				m_tokens.erase(m_tokens.begin() + k + 1, m_tokens.begin() + k + 3) ;
			}
			if( t0.IsType( DISCARD))
			{
				t0.m_table_idx = METAL_DISCARD;

				std::vector<_token> ts;
				
				{
					_token t;
					t.m_type = 5;
					t.m_table_idx = LEFT_PARENT;
					ts.push_back( t);
				}
				{
					_token t;
					t.m_type = 5;
					t.m_table_idx = RIGHT_PARENT;
					ts.push_back( t);
				}
				
				m_tokens.insert( m_tokens.begin() + k + 1, ts.begin(), ts.end());
			}
			
			// translate texture lod
			if( t0.IsType(TEXTURE_LOD))
			{
				int c = 1 ;
				size_t i = k + 2;
				
				while (c > 0)
				{
					_token &t = m_tokens[i];
					if (t.IsType(LEFT_PARENT)) c++ ;
					if (t.IsType(RIGHT_PARENT)) c-- ;
					i++ ;
				}
				
				_token t;
				
				t.m_type = 0;
				t.m_table_idx = TOKEN_METAL_LEVEL;
				m_tokens.insert(m_tokens.begin()+i-2, t) ;
				
				t.m_type = 5;
				t.m_table_idx = LEFT_PARENT;
				m_tokens.insert(m_tokens.begin()+i-1, t) ;
				
				t.m_type = 5;
				t.m_table_idx = RIGHT_PARENT;
				m_tokens.insert(m_tokens.begin()+i+1, t) ;
			}
			
			if( t0.IsType( TEXTURE) || t0.IsType(TEXTURE_LOD))
			{
				t0.m_table_idx = METAL_TEXTURE;
				
				_token &texture_name = m_tokens[k+2];
				
				std::vector<_token> ts;
				
				{
					_token t = texture_name;
					
					ts.push_back( t);
				}
				{
					_token t;
					t.m_type = 5;
					t.m_table_idx = DOT;
					ts.push_back( t);
				}
				
				{
					_variable &v = m_variables[texture_name.m_table_idx];
					
					_variable sampler_v;
					sampler_v.m_name = v.m_name + "_sampler";
					sampler_v.m_type_idx = -1;
					sampler_v.m_storage_type = -1;
					
					texture_name.m_table_idx = m_variables.size();
					
					m_variables.push_back( sampler_v);
				}
				
				m_tokens.insert( m_tokens.begin() + k, ts.begin(), ts.end());
			}
		}
		
		if( IsRMAD3DFamily(m_api))
		{
			if (t0.IsType(TEXTURE) || t0.IsType(TEXTURE_LOD))
			{
				t0.m_table_idx = t0.IsType(TEXTURE) ? HLSL_TEXTURE : HLSL_TEXTURE_LOD;

				_token &texture_name = m_tokens[k+2];

				std::vector<_token> ts;

				{
					_token t = texture_name;

					ts.push_back( t);
				}
				{
					_token t;
					t.m_type = 5;
					t.m_table_idx = DOT;
					ts.push_back( t);
				}

				{
					_variable &v = m_variables[texture_name.m_table_idx];

					_variable sampler_v;
					sampler_v.m_name = v.m_name + "_sampler";
					sampler_v.m_type_idx = -1;
					sampler_v.m_storage_type = -1;
				
					texture_name.m_table_idx = m_variables.size();
				
					m_variables.push_back( sampler_v);
				}

				m_tokens.insert( m_tokens.begin() + k, ts.begin(), ts.end());
			}

			if( t0.IsType( MUL))
			{
				_token &t1 = m_tokens[k-1];
				_token &t2 = m_tokens[k+1];

				if( t1.m_type == 1 && t2.m_type == 1)
				{
					_variable &v1 = m_variables[t1.m_table_idx];
					_variable &v2 = m_variables[t2.m_table_idx];

					if( 
						(v1.m_type_idx == FLOAT4x4 && v2.m_type_idx == FLOAT4) || 
						(v1.m_type_idx == FLOAT3x3 && v2.m_type_idx == FLOAT3) ||

						(v1.m_type_idx == FLOAT4 && v2.m_type_idx == FLOAT4x4) || 
						(v1.m_type_idx == FLOAT3 && v2.m_type_idx == FLOAT3x3)
						
						)
					{
						//check if one of them uniform
						bool un1 = v1.m_storage_type == 0;
						bool un2 = v2.m_storage_type == 0;
						bool swap_variables = !un1 && !un2;

						//swap
						if (swap_variables)
						{
							_token temp = m_tokens[k - 1];
							m_tokens[k - 1] = m_tokens[k + 1];
							m_tokens[k + 1] = temp;
							m_swapped_tokens_first.push_back(k - 1);
						}

						t0.m_table_idx = COMMA;
						{
							std::vector<_token> ts;

							{
								_token t;
								t.m_type = 6;
								t.m_table_idx = HLSL_MUL;
								ts.push_back( t);
							}
							{
								_token t;
								t.m_type = 5;
								t.m_table_idx = LEFT_PARENT;
								ts.push_back( t);
							}

							m_tokens.insert( m_tokens.begin() + k - 1, ts.begin(), ts.end());

							ts.clear();

							{
								_token t;
								t.m_type = 5;
								t.m_table_idx = RIGHT_PARENT;
								ts.push_back( t);
							}

							m_tokens.insert( m_tokens.begin() + k + 2 + 2, ts.begin(), ts.end());
						}
					}
				}
			}
		}
	}

	//
	//	rename buffer variables in gl, because the reflection
	//	works by the layout name
	//
	if (IsRMAClassicGLFamily(m_api) || IsRMAVulkan(m_api))
	{
		for (size_t i = 0; i < m_variables.size(); i++)
		{
			_variable &v = m_variables[i];
			if (v.m_storage_type == 4)
			{
				v.m_name += GL_BUFFER_LAYOUT_INNER_DATA;
			}
		}
	}
}


void _parser::RemoveComments()
{
	int comment_mode = -1;
	std::string tmp;

	tmp = " " + m_text + " ";
	m_text.clear();

	for( size_t i=1; i<tmp.length()-1; i++)
	{
		if( comment_mode == -1)
		{
			if( tmp[i] == '/')
			{
				if( tmp[i+1] == '/')
				{
					comment_mode = 0;
				}
				else if( tmp[i+1] == '*')
				{
					comment_mode = 1;
				}
			}
		}
		else if( comment_mode == 0)
		{
			if( tmp[i] == '\n')
			{
				comment_mode = -1;
			}
		}
		else if( comment_mode == 1)
		{
			if( tmp[i-1] == '*' && tmp[i] == '/')
			{
				comment_mode = -1;
				continue;
			}
		}

		if( comment_mode == -1)
		{
			m_text.push_back( tmp[i]);
		}
	}
}


void _parser::CreatemBuiltinFunctions()
{
	m_builtin_functions.resize(_parser::NUM_OF_BUILTIN_FUNCTIONS);

	m_builtin_functions[NORMALIZE] = _multistring("normalize", "normalize");

	if (IsRMAClassicGLFamily(m_api) || IsRMAVulkan(m_api))
	{
		m_builtin_functions[MIX]                   = _multistring("mix", "mix");
		m_builtin_functions[TEXTURE]               = _multistring("texture", "texture");
		m_builtin_functions[TEXTURE_ARRAY]		   = _multistring("texture", "texture");
		m_builtin_functions[TEXTURE_LOD]           = _multistring("textureLod", "textureLod");
		m_builtin_functions[DISCARD]               = _multistring("discard", "discard");
		m_builtin_functions[POW]                   = _multistring("pow", "pow");
		m_builtin_functions[FRACT]                 = _multistring("fract", "fract");
		m_builtin_functions[BARRIER]               = _multistring("barrier", "barrier");
		m_builtin_functions[MEMORY_BARRIER_SHARED] = _multistring("memoryBarrierShared", "memoryBarrierShared");
	}
	if (IsRMAD3DFamily(m_api))
	{
		m_builtin_functions[MIX]                   = _multistring("mix", "lerp");
		m_builtin_functions[TEXTURE]               = _multistring("texture", "Sample");
		m_builtin_functions[TEXTURE_LOD]           = _multistring("textureLod", "SampleLevel");
		m_builtin_functions[DISCARD]               = _multistring("discard", "discard");
		m_builtin_functions[HLSL_MUL]              = _multistring("mul", "mul");
		m_builtin_functions[HLSL_TEXTURE]          = _multistring("texture", "Sample");
		m_builtin_functions[HLSL_TEXTURE_LOD]      = _multistring("texture", "SampleLevel");
		m_builtin_functions[POW]                   = _multistring("pow", "pow");
		m_builtin_functions[FRACT]                 = _multistring("fract", "frac");
		m_builtin_functions[BARRIER]               = _multistring("barrier", "");
		m_builtin_functions[MEMORY_BARRIER_SHARED] = _multistring("memoryBarrierShared", "GroupMemoryBarrierWithGroupSync");
	}
	if (IsRMAMetalFamily(m_api))
	{
		m_builtin_functions[MIX]           = _multistring("mix", "mix");
		m_builtin_functions[TEXTURE]       = _multistring("texture", "sample");
		m_builtin_functions[TEXTURE_LOD]   = _multistring("textureLod", "sample");
		m_builtin_functions[DISCARD]       = _multistring("discard", "discard_fragment");
		m_builtin_functions[METAL_TEXTURE] = _multistring("texture", "sample");
		m_builtin_functions[METAL_DISCARD] = _multistring("discard", "discard_fragment");
		m_builtin_functions[POW]           = _multistring("pow", "powr");
		m_builtin_functions[FRACT]         = _multistring("fract", "fract");
		m_builtin_functions[BARRIER]               = _multistring("barrier", "threadgroup_barrier( mem_flags::mem_none )");
		m_builtin_functions[MEMORY_BARRIER_SHARED] = _multistring("memoryBarrierShared", "threadgroup_barrier( mem_flags::mem_threadgroup )");
		m_builtin_functions[MEMORY_BARRIER] = _multistring("memoryBarrier", "threadgroup_barrier( mem_flags::mem_device_and_threadgroup )");
	}
}


void _parser::CreateOperatorsPunctuators()
{
	m_operators_punctuators.resize(NUM_OF_OPERATORS_PUNCTUATORS);

	m_operators_punctuators[SEMICOLON]          = _multistring(";");
	m_operators_punctuators[LEFT_BRACKET]       = _multistring("[");
	m_operators_punctuators[RIGHT_BRACKET]      = _multistring("]");
	m_operators_punctuators[LEFT_PARENT]        = _multistring("(");
	m_operators_punctuators[RIGHT_PARENT]       = _multistring(")");
	m_operators_punctuators[LEFT_BRACE]         = _multistring("{");
	m_operators_punctuators[RIGHT_BRACE]        = _multistring("}");
	m_operators_punctuators[EQUAL]              = _multistring("=");
	m_operators_punctuators[MUL]                = _multistring("*");
	m_operators_punctuators[INCR]               = _multistring("++");
	m_operators_punctuators[DECR]               = _multistring("--");
	m_operators_punctuators[INCR_ASSIGN]        = _multistring("+=");
	m_operators_punctuators[DECR_ASSIGN]        = _multistring("-=");
	m_operators_punctuators[MUL_ASSIGN]         = _multistring("*=");
	m_operators_punctuators[DIV_ASSIGN]         = _multistring("/=");
	m_operators_punctuators[IS_EQUAL]           = _multistring("==");
	m_operators_punctuators[NOT_EQUAL]          = _multistring("!=");
	m_operators_punctuators[LESS_EQUAL]         = _multistring("<=");
	m_operators_punctuators[GREATER_EQUAL]      = _multistring(">=");
	m_operators_punctuators[LESS]               = _multistring("<");
	m_operators_punctuators[GREATER]            = _multistring(">");
	m_operators_punctuators[SHIFT_LEFT]         = _multistring("<<");
	m_operators_punctuators[SHIFT_RIGHT]        = _multistring(">>");
	m_operators_punctuators[DOT]                = _multistring(".");
	m_operators_punctuators[COMMA]              = _multistring(",");
	m_operators_punctuators[AND]                = _multistring("&&");
	m_operators_punctuators[OR]                 = _multistring("||");
	m_operators_punctuators[AMPERSAND]          = _multistring("&");
}


void _parser::CreateTypes()
{
	m_types.resize(_parser::NUM_OF_PRIMITIVE_TYPES);

	m_types[_parser::BOOL]   = _multistring("bool");
	m_types[_parser::CHAR]   = _multistring("char");
	m_types[_parser::UCHAR]  = _multistring("uchar");
	m_types[_parser::DOUBLE] = _multistring("double");
	m_types[_parser::FLOAT]  = _multistring("float");
	m_types[_parser::INT]    = _multistring("int");
	m_types[_parser::UINT]   = _multistring("uint");
	m_types[_parser::SHORT]  = _multistring("short");
	m_types[_parser::USHORT] = _multistring("ushort");
	m_types[_parser::VOID]   = _multistring("void");

	if (IsRMAClassicGLFamily(m_api) || IsRMAVulkan(m_api))
	{
		m_types[_parser::FLOAT2]      = _multistring("vec2");
		m_types[_parser::FLOAT3]      = _multistring("vec3");
		m_types[_parser::FLOAT4]      = _multistring("vec4");
		m_types[_parser::IVEC2]       = _multistring("ivec2");
		m_types[_parser::IVEC3]       = _multistring("ivec3");
		m_types[_parser::IVEC4]       = _multistring("ivec4");
		m_types[_parser::FLOAT4x4]    = _multistring("mat4");
		m_types[_parser::FLOAT3x3]    = _multistring("mat3");
		m_types[_parser::SAMPLER2D]   = _multistring("sampler2D");
		m_types[_parser::SAMPLER2DShadow] = _multistring("sampler2DShadow");
		m_types[_parser::SAMPLER2DArray] = _multistring("sampler2DArray");
		m_types[_parser::SAMPLERCUBE] = _multistring("samplerCube");
	}
	if( IsRMAD3DFamily(m_api) || IsRMAMetalFamily(m_api))
	{
		m_types[_parser::FLOAT2]   = _multistring("vec2", "float2");
		m_types[_parser::FLOAT3]   = _multistring("vec3", "float3");
		m_types[_parser::FLOAT4]   = _multistring("vec4", "float4");
		m_types[_parser::FLOAT4x4] = _multistring("mat4", "float4x4");
		m_types[_parser::FLOAT3x3] = _multistring("mat3", "float3x3");
	}
	if( IsRMAD3DFamily(m_api))
	{
		m_types[_parser::IVEC2] = _multistring("ivec2", "int2");
		m_types[_parser::IVEC3] = _multistring("ivec3", "int3");
		m_types[_parser::IVEC4] = _multistring("ivec4", "int4");
		m_types[_parser::SAMPLER2D]   = _multistring("sampler2D", "Texture2D");
		m_types[_parser::SAMPLERCUBE] = _multistring("samplerCube", "TextureCube");
	}
	if( IsRMAMetalFamily(m_api))
	{
		m_types[_parser::IVEC2]       = _multistring("ivec2","int2");
		m_types[_parser::IVEC3]       = _multistring("ivec3","int3");
		m_types[_parser::IVEC4]       = _multistring("ivec4","int4");
		m_types[_parser::SAMPLER2D]   = _multistring("sampler2D", "texture2d<float>");
		m_types[_parser::SAMPLERCUBE] = _multistring("samplerCube", "texturecube<float>");
	}
}


void _parser::CreateReservedWord()
{
	m_reserverd_words.resize(_parser::NUM_OF_RESERVERED_WORDS);

	m_reserverd_words[TOKEN_BREAK]    = _multistring("break");
	m_reserverd_words[TOKEN_CONST]    = _multistring("const");
	m_reserverd_words[TOKEN_CONTINUE] = _multistring("continue");
	m_reserverd_words[TOKEN_DO]       = _multistring("do");
	m_reserverd_words[TOKEN_ELSE]     = _multistring("else");
	m_reserverd_words[TOKEN_FOR]      = _multistring("for");
	m_reserverd_words[TOKEN_GOTO]     = _multistring("goto");
	m_reserverd_words[TOKEN_IF]       = _multistring("if");
	m_reserverd_words[TOKEN_RETURN]   = _multistring("return");
	m_reserverd_words[TOKEN_STRUCT]   = _multistring("struct");
	m_reserverd_words[TOKEN_WHILE]    = _multistring("while");
	m_reserverd_words[TOKEN_IN]       = _multistring("in");
	m_reserverd_words[TOKEN_OUT]      = _multistring("out");
	m_reserverd_words[TOKEN_UNIFORM]  = _multistring("uniform");
	m_reserverd_words[TOKEN_BUFFER]   = _multistring("buffer");
	m_reserverd_words[TOKEN_IMAGE]    = _multistring("image");
	m_reserverd_words[TOKEN_SHARED]   = _multistring("shared");
	m_reserverd_words[TOKEN_GLOBAL]   = _multistring("global");
	m_reserverd_words[TOKEN_METAL_DEVICE] = _multistring("device");
	m_reserverd_words[TOKEN_METAL_LEVEL]  = _multistring("level");
	m_reserverd_words[TOKEN_METAL_CONSTANT]  = _multistring("constant");
	m_reserverd_words[TOKEN_METAL_THREAD] = _multistring("thread");
}


void _parser::Tokenizer( const std::string &str)
{
	_token t;

	t.m_type = -1;

	if( str[0] == '#')
	{
		t.m_type = 3;
		t.m_table_idx = m_macros.size();
		m_macros.push_back( str);

		goto success;
	}

	for( size_t i=0; i<m_operators_punctuators.size(); i++)
	{
		if( str == m_operators_punctuators[i].from_string)
		{
			t.m_type = 5;
			t.m_table_idx = i;

			goto success;
		}
	}

	for( size_t i=0; i<m_reserverd_words.size(); i++)
	{
		if (str == m_reserverd_words[i].from_string)
		{
			t.m_type = 0;
			t.m_table_idx = i;

			goto success;
		}
	}

	for( size_t i=0; i<m_types.size(); i++)
	{
		if (str == m_types[i].from_string)
		{
			t.m_type = 4;
			t.m_table_idx = i;

			goto success;
		}
	}

	for( size_t i=0; i<m_builtin_functions.size(); i++)
	{
		if (str == m_builtin_functions[i].from_string)
		{
			t.m_type = 6;
			t.m_table_idx = i;

			goto success;
		}
	}

	if( isdigit( str[0]))
	{
		double number;

		int r = sscanf( str.c_str(), "%lf", &number);
		if( r == 1)
		{
			t.m_type = 2;
			t.m_table_idx = m_numbers.size();
			
			_number num;
			num.m_num = number;

			if (str.find(".") != std::string::npos)
			{
				num.m_type = 0;
			}
			else
			{
				num.m_type = 1;
			}

			m_numbers.push_back( num);
			goto success;
		}

	}
	else
	{
		if( m_tokens.size() && m_tokens[m_tokens.size()-1].IsType( TOKEN_STRUCT))
		{
			t.m_type = 4;
			t.m_table_idx = m_types.size();
			m_types.push_back( str);
		}
		else
		{
			t.m_type = 1;

			for( size_t i=0; i<m_variables.size(); i++)
			{
				if( m_variables[i].m_name == str)
				{
					t.m_table_idx = i;
					goto success;
				}
			}

			_variable v;

			v.m_name = str;
			v.m_type_idx = -1;
			t.m_table_idx = m_variables.size();
			m_variables.push_back( v);
		}
		goto success;
	}

	return;
success:
	m_tokens.push_back( t);
}


void _parser::Tokenizer3()
{
	/*!
	0 - szam
	1 - nev
	2>= - jel, egyeb - WS
	*/
	int token_type = 0;
	std::string token;

	for( size_t i=0; i<m_text.length(); i++)
	{
		unsigned char cur_char = m_text[i];

		if( token.length() == 0)
		{
			if( isdigit( cur_char) || cur_char == '.')
			{
				token_type = 0;
				token.push_back( cur_char);
			}
			else if( isalpha( cur_char) || cur_char == '_')
			{
				token_type = 1;
				token.push_back( cur_char);
			}
			else if( 
				cur_char == '+' || 
				cur_char == '-' || 
				cur_char == '*' || 
				cur_char == '/' ||
				cur_char == '>' ||
				cur_char == '<' ||
				cur_char == '!' ||
				cur_char == '&' ||
				cur_char == '|' ||
				cur_char == '=')
			{
				token_type = 2 + (cur_char << 16);
				token.push_back( cur_char);
			}
			else if( cur_char == '#')
			{
				token_type = 4;
				token.push_back( cur_char);
			}
			else if( is_C_sign( cur_char))
			{
				token.push_back( cur_char);

				Tokenizer( token);

				token.clear();
			}
			else
			{
				token_type = -1;
			}
		}
		else
		{
			int tt = (token_type & 0xFFFF);

			if( tt == 0)
			{
				if( isdigit( cur_char) || cur_char == 'f' || cur_char == '.')
				{
					token.push_back( cur_char);
				}
				else
				{
					Tokenizer( token);
					token.clear();
					i--;
				}
			}
			else if( tt == 1)
			{
				if( isalnum( cur_char) || cur_char == '_')
				{
					token.push_back( cur_char);
				}
				else
				{
					Tokenizer( token);
					token.clear();
					i--;
				}
			}
			else if( tt == 2)
			{
				unsigned char ll = token_type >> 16;

				if( ll == '+' && cur_char == '+')
				{
					token.push_back( cur_char);
				}
				else if( ll == '-' && cur_char == '-')
				{
					token.push_back( cur_char);
				}
				else if (ll == '&' && cur_char == '&')
				{
					token.push_back(cur_char);
				}
				else if (ll == '|' && cur_char == '|')
				{
					token.push_back(cur_char);
				}
				else if (ll == '>' && cur_char == '>')
				{
					token.push_back(cur_char);
				}
				else if (ll == '<' && cur_char == '<')
				{
					token.push_back(cur_char);
				}
				else if (cur_char == '=')
				{
					token.push_back( cur_char);
				}
				else
				{
					i--;
				}

				Tokenizer( token);
				token.clear();
			}
			else if( tt == 4)
			{
				if( cur_char == '\n' || cur_char == '\r')
				{
					Tokenizer( token);
					token.clear();
				}
				else
				{
					token.push_back( cur_char);
				}
			}
		}
	}
}


void _parser::SearchTypesAndScopes()
{
	for(size_t i=1; i<m_tokens.size(); i++)
	{
		_token &t = m_tokens[i];

		if( t.m_type == 1)
		{
			_variable &v = m_variables[t.m_table_idx];

			if( v.m_type_idx == -1)
			{
				if( m_tokens[i-1].m_type == 4)
				{
					v.m_type_idx = (int)m_tokens[i-1].m_table_idx;
				}
			}
		}
	}
}


void _parser::ProcessStruct(size_t start_token_id)
{
	_struct new_struct;
	size_t s = start_token_id;

	new_struct.name_index = m_tokens[s + 1].m_table_idx;

	if (!m_tokens[s + 2].IsType(LEFT_BRACE))
	{
		m_structs.push_back(_struct());
		return;
	}

	size_t i = s + 3; //step to the first attribute token
	while (!m_tokens[i].IsType(RIGHT_BRACE))
	{
		//        id: 0      1      2  3       4  5
		//  is array: <type> <name> [  <size>  ]  ; - 6
		// non array: <type> <name> ;               - 3

		bool is_array = false;

		new_struct.type_indices.push_back(m_tokens[i + 0].m_table_idx);
		new_struct.name_indices.push_back(m_tokens[i + 1].m_table_idx);

		if (m_tokens[i + 2].IsType(SEMICOLON))
		{
			is_array = false;
			new_struct.count.push_back(0);
		}
		else if (m_tokens[i + 2].IsType(LEFT_BRACKET) && m_tokens[i + 4].IsType(RIGHT_BRACKET) && m_tokens[i + 5].IsType(SEMICOLON))
		{
			is_array = true;
			new_struct.count.push_back( static_cast<const unsigned int>(m_numbers[m_tokens[i + 3].m_table_idx].m_num));
		}
		else
		{
			m_structs.push_back(_struct());
			return;
		}

		i += (is_array)?6:3; // step to the next attribute token
	}

	m_structs.push_back(new_struct);
}


void _parser::BuildStructs()
{
	for (size_t i = 1; i<m_tokens.size(); i++)
	{
		_token &t = m_tokens[i];

		if ( (t.m_type == 0) && (t.m_table_idx == _parser::TOKEN_STRUCT) )
		{
			ProcessStruct(i);
		}
	}
}


void _parser::CreateReflection(NGL_shader_source_descriptor &ssd)
{
	if (m_shader_type == NGL_VERTEX_SHADER)
	{
		for (size_t k = 0; k < m_ins.size(); k++)
		{
			_variable &v = m_ins[k];
			NGL_vertex_attrib sra;

			sra.m_semantic = v.m_name;
			switch (v.m_type_idx)
			{
			case FLOAT:
			{
				sra.m_format = NGL_R32_FLOAT;
				break;
			}
			case FLOAT2:
			{
				sra.m_format = NGL_R32_G32_FLOAT;
				break;
			}
			case FLOAT3:
			{
				sra.m_format = NGL_R32_G32_B32_FLOAT;
				break;
			}
			case FLOAT4:
			{
				sra.m_format = NGL_R32_G32_B32_A32_FLOAT;
				break;
			}
			}
			ssd.m_used_vertex_attribs.push_back(sra);
		}
	}

    
    // TODO
#if 0
	for (size_t k = 0; k<m_uniforms.size(); k++)
	{
		_variable &v = m_uniforms[k];
		_gpu_api::_shader::_reflection::_uniform sru;

		sru.m_name = v.m_name;
		sru.m_size = 1;

		switch (v.m_type_idx)
		{
		case FLOAT4:
		{
			sru.m_format = _gpu_api::_shader::FLOAT4;
			sr.m_uniforms.push_back(sru);
			break;
		}
		case FLOAT4x4:
		{
			sru.m_format = _gpu_api::_shader::FLOAT16;
			sr.m_uniforms.push_back(sru);
			break;
		}
		case FLOAT2:
		{
			sru.m_format = _gpu_api::_shader::FLOAT2;
			sr.m_uniforms.push_back(sru);
			break;
		}
		case FLOAT:
		{
			sru.m_format = _gpu_api::_shader::FLOAT;
			sr.m_uniforms.push_back(sru);
			break;
		}
		case IVEC4:
		{
			sru.m_format = _gpu_api::_shader::INT4;
			sr.m_uniforms.push_back(sru);
			break;
		}
		case INT:
		{
			sru.m_format = _gpu_api::_shader::INT;
			sr.m_uniforms.push_back(sru);
			break;
		}
		default:
			printf("shader reflection: unknown uniform type\n");
			assert(0);
		}
	}
	for (size_t k = 0; k<m_samplers.size(); k++)
	{
		_variable &v = m_samplers[k];
		_gpu_api::_shader::_reflection::_uniform sru;

		sru.m_name = v.m_name;
		sru.m_size = 1;

		switch (v.m_type_idx)
		{
		case SAMPLER2D:
		case SAMPLER2DShadow:
		case SAMPLER2DArray:
		{
			sru.m_format = _gpu_api::_shader::TEXTURE;
			sr.m_uniform_textures.push_back(sru);
			break;
		}
		case SAMPLERCUBE:
		{
			sru.m_format = _gpu_api::_shader::TEXTURE;
			sr.m_uniform_textures.push_back(sru);
			break;
		}
		default:
			printf("shader reflection: unknown sampler type\n");
			assert(0);
		}
	}
	for (size_t k = 0; k<m_buffers.size(); k++)
	{
		_variable &v = m_buffers[k];
		_gpu_api::_shader::_reflection::_uniform sru;

		sru.m_name = v.m_name;
		sru.m_size = 1;
		sru.m_format = _gpu_api::_shader::BUFFER;
		sr.m_uniform_buffers.push_back(sru);
	}
    #endif
}


void _parser::InsertTokens(size_t start_id, std::vector<_token> tokens)
{
	m_tokens.insert( m_tokens.begin()+start_id, tokens.begin(), tokens.end() );
	
	for (size_t i = 0; i < m_functions.size(); i++)
	{
		_function* f = m_functions[i] ;
		if (f->start_token_idx < start_id)
		{
			size_t f_arg_end = f->start_token_idx + f->num_args + 6;
			size_t f_end = f_arg_end + f->num_tokens;
			
			if (start_id <= f_arg_end)
			{
				f->num_args += tokens.size() ;
			}
			
			if ((start_id > f_arg_end) && (start_id <= f_end))
			{
				f->num_tokens += tokens.size() ;
			}
		}
		else
		{
			f->start_token_idx += tokens.size() ;
		}
	}
}


void _parser::InsertGlobalBufferParametersForMetal()
{
	// early out, if there is only main function
	if (m_functions.size() == 1)
	{
		return;
	}
	
	// early out, if there are no buffers
	if ((m_buffers.size() == 0) && (m_globals.size() == 0) && (m_shared.size() == 0))
	{
		return;
	}
	
	//
	//  get method names
	//
	std::vector<std::string> method_names ;
	for (size_t i = 0; i < m_functions.size() - 1; i++)
	{
		_function* f = m_functions[i] ;
		_token & t = m_tokens[f->start_token_idx+1];
		method_names.push_back(m_variables[t.m_table_idx].m_name) ;
	}
	
	
	//
	//  get used buffers and called methods
	//
	std::vector< std::set< std::pair<unsigned int, unsigned int> > > used_buffers_idx ; // buffer id, buffer type (shared, global, buffer)
	used_buffers_idx.resize(m_functions.size()-1) ;
	std::vector< std::set< size_t > > used_methods_idx;
	used_methods_idx.resize(m_functions.size()-1) ;
	
	for (size_t i = 0; i < m_functions.size() - 1; i++)
	{
		_function* f = m_functions[i] ;
		
		size_t token_count = f->num_tokens + f->num_args + 6;
		
		for (size_t j = 0; j < token_count ; j++)
		{
			size_t id = f->start_token_idx + j;
			_token & t = m_tokens[id];

			if( t.m_type == 1 )
			{
				size_t storage_type = m_variables[t.m_table_idx].m_storage_type;
				if ((storage_type == 4) || (storage_type == 5) || (storage_type == 6))
				{
					used_buffers_idx[i].insert( std::pair<unsigned int, unsigned int>( static_cast<unsigned int>(t.m_table_idx), static_cast<unsigned int>(storage_type)) ) ;
				}
			}
			
			if (j < (f->num_args + 6)) continue ;
			
			if (t.m_type == 1)
			{
				size_t k = 0;
				for (; k < method_names.size(); k++)
				{
					if (method_names[k] == m_variables[t.m_table_idx].m_name) break ;
				}
				size_t function_id = k ;
			 
				if (function_id < method_names.size())
				{
					used_methods_idx[i].insert(function_id) ;
				}
			}
		}
	}
	

#if 0
	// dump used buffers
	printf("\n");
	for (int i = 0; i < method_names.size(); i++)
	{
		printf("%s:\n",method_names[i].c_str());
		
		printf("used methods: ") ;
		for (auto it = used_methods_idx[i].begin(); it != used_methods_idx[i].end();it++)
		{
			printf("%s, ", method_names[*it].c_str()) ;
		}
		printf("\n") ;
		
		printf("used buffers: ") ;
		for (auto it = used_buffers_idx[i].begin(); it != used_buffers_idx[i].end();it++)
		{
			printf("%s, ", m_variables[*it].m_name.c_str()) ;
		}
		
		printf("\n\n");
	}
#endif
	
	
	//
	//  recursively fill the used buffers
	//
	bool changed = true ;
	while (changed)
	{
		changed = false ;
		
		for (size_t i = 0; i < m_functions.size()-1; i++)
		{
			for (std::set<size_t>::iterator m_it = used_methods_idx[i].begin(); m_it != used_methods_idx[i].end(); m_it++)
			{
				for (std::set< std::pair<unsigned int, unsigned int> >::iterator b_it = used_buffers_idx[*m_it].begin() ; b_it != used_buffers_idx[*m_it].end(); b_it++)
				{
					if (used_buffers_idx[i].find(*b_it) == used_buffers_idx[i].end())
					{
						used_buffers_idx[i].insert(*b_it);
						changed = true ;
					}
				}
			}
		}
	}

	

	//
	//  insert buffer arguments to argument list
	//
	for (size_t i = 0; i < m_functions.size() - 1; i++)
	{
		_function* f = m_functions[i] ;
		std::vector<_token> new_tokens;
		
		for (std::set< std::pair<unsigned int, unsigned int> >::iterator it = used_buffers_idx[i].begin() ; it != used_buffers_idx[i].end() ; it++)
		{
			_token t ;
			
			t.m_type = 0;
			if (it->second == 6)
			{
				t.m_table_idx = TOKEN_METAL_THREAD;
			}
			else
			{
				if (m_shader_type == NGL_FRAGMENT_SHADER)
				{
					t.m_table_idx = (m_variables[it->first].size == 0) ? TOKEN_METAL_CONSTANT : TOKEN_METAL_DEVICE;
				}
				else
				{
					t.m_table_idx = TOKEN_METAL_DEVICE;
				}
			}
			new_tokens.push_back(t);
			
			t.m_type = 4;
			t.m_table_idx = m_variables[it->first].m_type_idx;
			new_tokens.push_back(t);
			
			t.m_type = 5;
			t.m_table_idx = (m_variables[it->first].size == 0)?AMPERSAND:MUL ;
			new_tokens.push_back(t);
			
			t.m_type = 1;
			t.m_table_idx = it->first;
			new_tokens.push_back(t);
			
			t.m_type = 5;
			t.m_table_idx = COMMA;
			new_tokens.push_back(t);
		}
		
		InsertTokens(f->start_token_idx + 3, new_tokens) ;
	}
	
	
	//
	//  insert buffers to function calls
	//
	for (size_t i = 0; i < m_functions.size(); i++)
	{
		_function* f = m_functions[i] ;
		
		for (size_t j = 0; j < (f->num_tokens + f->num_args + 6); j++)
		{
			size_t id = f->start_token_idx + j;
			_token & t = m_tokens[id];
			
			if (j < (f->num_args + 6))
			{
				continue ;
			}
			
			if (t.m_type == 1)
			{
				size_t k = 0;
				for (; k < method_names.size(); k++)
				{
					if (method_names[k] == m_variables[t.m_table_idx].m_name) break ;
				}
				size_t function_id = k;

				if (function_id >= method_names.size())
				{
					continue ;
				}
				
				std::vector<_token> new_tokens;
				for (std::set< std::pair<unsigned int, unsigned int> >::iterator it = used_buffers_idx[function_id].begin() ; it != used_buffers_idx[function_id].end() ; it++)
				{
					_token t ;
					
					t.m_type = 1;
					t.m_table_idx = it->first;
					new_tokens.push_back(t);
					
					t.m_type = 5;
					t.m_table_idx = COMMA;
					new_tokens.push_back(t);
				}
				
				InsertTokens(id + 2, new_tokens) ;
			}
		}
	}
}


