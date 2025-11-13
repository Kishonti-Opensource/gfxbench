/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_shader_common.h"
#include "platform.h"

#include "ng/log.h"
#include "kcl_base.h"

using namespace KCL ;

namespace GLB
{

static const int MAXLOGLEN = 32768;

#ifdef DEBUG
bool GLBShaderCommon::LogShaderWarnings = true;
#else
bool GLBShaderCommon::LogShaderWarnings = false;
#endif

uniform GLBShaderCommon::m_uniforms[uniforms::UNIFORM_COUNT];
attrib GLBShaderCommon::m_attribs[attribs::ATTRIB_COUNT];

bool GLBShaderCommon::is_highp = false;

GLBShaderCommon::PRECISION GLBShaderCommon::m_vs_float_prec = NONEP;
GLBShaderCommon::PRECISION GLBShaderCommon::m_fs_float_prec = NONEP;
GLBShaderCommon::PRECISION GLBShaderCommon::m_vs_int_prec = NONEP;
GLBShaderCommon::PRECISION GLBShaderCommon::m_fs_int_prec = NONEP;

GLBShaderCommon::GLBShaderCommon()
{
	for(unsigned int i = 0; i < attribs::ATTRIB_COUNT; i++)
	{
		m_attrib_locations[i] = -1;
	}	
	for(unsigned int i = 0; i < uniforms::UNIFORM_COUNT; i++)
	{
		m_uniform_locations[i] = -1;
	}
	m_p = glCreateProgram();
	m_glsl_version_string = "Shader version string header not set!" ;

	m_instruction_count_v = 0;
	m_instruction_count_f = 0;
}

GLBShaderCommon::~GLBShaderCommon()
{
	glDeleteProgram(m_p);
}


KCL::KCL_Status GLBShaderCommon::CompileShader(KCL::uint32 shader, const std::string & shader_source)
{	
	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;

	std::string final_source = m_glsl_version_string + shader_source;
	const char* final_source_cstr = final_source.c_str();
	
	glShaderSource(shader, 1, &final_source_cstr , 0);
	glCompileShader(shader);

	// Check the compile status
	GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	GLint log_length = 0; 
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

	if (status != GL_TRUE || (LogShaderWarnings && (log_length > 1)))
	{
	
		if (status != GL_TRUE)
		{
			NGLOG_ERROR("Shader compile error!\n");
		}
		else
		{
			NGLOG_ERROR("Shader compile warning!\n");
		}

		// Dump the error. Add the line numbers to the source
        std::stringstream string_stream(final_source);
        std::string line;
        unsigned int line_counter = 0;
        while (std::getline(string_stream, line))
        {
            NGLOG_ERROR("%s: %s", ++line_counter, line);
        }
       
		// Write out the GL context
		const GLubyte *gl_vendor = glGetString(GL_VENDOR);
		const GLubyte *gl_version = glGetString(GL_VERSION);
		const GLubyte *gl_glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
		NGLOG_ERROR("Vendor: %s Version: %s GLSL: %s", gl_vendor, gl_version, gl_glsl_version);
				
		// Write out the error from the GL driver	
		if(log_length > 0 && log_length < MAXLOGLEN)
		{
			std::vector<char> log_buffer(log_length);
			glGetShaderInfoLog(shader, log_length, NULL, &log_buffer[0]);			
			
			string_stream.str(std::string(&log_buffer[0]));
			while(std::getline(string_stream, line))
			{
				NGLOG_ERROR("%s", line);
			} 			
			NGLOG_ERROR("GL driver log(%s):\n%s\n", log_length, &log_buffer[0]);
		}
		else
		{
			NGLOG_ERROR("Invalid GL driver log size: %s\n", log_length);
		}

		DumpSourceFileNames();

		result = (status == GL_TRUE) ? KCL_TESTERROR_NOERROR : KCL_TESTERROR_SHADER_ERROR;

		#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
            GLint shaderType = -1;
            glGetShaderiv( shader, GL_SHADER_TYPE, &shaderType );
            if(shaderType == GL_VERTEX_SHADER)
		    {
                m_instruction_count_v = CalcInstrCount(shader_source, CALC_VERTEX);
            }
            else if(shaderType == GL_FRAGMENT_SHADER)
		    {
                m_instruction_count_f = CalcInstrCount(shader_source, CALC_FRAGMENT);
            }
		#endif

	}	
	return result;
}


KCL::KCL_Status GLBShaderCommon::LinkProgram()
{
	// Link the program

	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;
	glLinkProgram(m_p);
	
	// Check the link status
	GLint status = 0;
    glGetProgramiv(m_p, GL_LINK_STATUS, &status);	

	GLint log_length = 0; 
	glGetProgramiv(m_p, GL_INFO_LOG_LENGTH, &log_length);
	if (status != GL_TRUE || (LogShaderWarnings && (log_length > 1)))
	{
		if (status != GL_TRUE)
		{	
			NGLOG_ERROR("Shader program link error!\n");
		}
		else
		{
			NGLOG_ERROR("Shader program link warning!\n");
		}
	
		// Write out the GL context
		const GLubyte * gl_vendor = glGetString(GL_VENDOR);
		const GLubyte * gl_version = glGetString(GL_VERSION);
		const GLubyte * gl_glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
		NGLOG_ERROR("Vendor: %s Version: %s GLSL: %s", gl_vendor, gl_version, gl_glsl_version);

		if(log_length > 0 && log_length < MAXLOGLEN)
		{
			std::vector<char> log_buffer(log_length);
			glGetProgramInfoLog(m_p, log_length, NULL, &log_buffer[0]);			
            NGLOG_ERROR("Log(%s):\n%s\n", log_length, &log_buffer[0]);
		}
		else
		{
			NGLOG_ERROR("Invalid log size: %s\n", log_length);
		}

		DumpSourceFileNames();

		result = (status == GL_TRUE) ? KCL_TESTERROR_NOERROR : KCL_TESTERROR_SHADER_ERROR;
	}

	return result;
}


void GLBShaderCommon::DumpSourceFileNames()
{
	if (!m_source_files.empty())
	{
		std::vector<std::string>::iterator it ;
		for (it = m_source_files.begin(); it != m_source_files.end(); it++)
		{
			NGLOG_ERROR("%s",it->c_str()) ;
		}
	}
	else
	{
		NGLOG_ERROR("No sourcefiles set.");
	}
}



void GLBShaderCommon::Validateprogram()
{
	glValidateProgram(m_p);
	GLint len = 0;
	glGetProgramiv(m_p, GL_VALIDATE_STATUS, &len);
	if(!len)
	{
		NGLOG_INFO("\nPROGRAM VALIDATE ERROR !!!\n");
		len = 0;
		glGetProgramiv(m_p, GL_INFO_LOG_LENGTH, &len);
		if(len > 1)
		{
			if(len < MAXLOGLEN)
			{
				char* log = new char[len];
				glGetProgramInfoLog(m_p, len, NULL, log);
				NGLOG_INFO("Log:\n%s\n",log);
				delete[] log;
			}
			else
			{
				NGLOG_INFO("LOG ERROR: invalid log size\n");
			}
		}
	}
}

void GLBShaderCommon::InitShaderLocations(bool has_vertex_shader)
{
	// We get INVALID OPERATION errors when trying to get attrib locations for compute programs.
	for (unsigned int i=0; i < attribs::ATTRIB_COUNT; i++)
	{
		if( m_attribs[i].name && has_vertex_shader)
		{
			m_attrib_locations[i] = glGetAttribLocation(m_p, m_attribs[i].name);
		}
		else
		{
			m_attrib_locations[i] = -1;
		}
	}

	for (unsigned int i = 0; i < uniforms::UNIFORM_COUNT; i++)
	{
		if( m_uniforms[i].name)
		{
			m_uniform_locations[i] = glGetUniformLocation(m_p, m_uniforms[i].name);
		}
		else
		{
			m_uniform_locations[i] = -1;
		}
	}
}


void GLBShaderCommon::AddPrecisionQualifierToUniforms(std::string &source)
{
	for (KCL::uint32 i = 0; i < uniforms::UNIFORM_COUNT; ++i)
	{
		if (0 == m_uniforms[i].name || 0 == m_uniforms[i].type || 0 == m_uniforms[i].precision) continue;

		std::string findThat = m_uniforms[i].type;
		findThat += " ";
		findThat += m_uniforms[i].name;
		findThat += ";";

		size_t findThatLength = findThat.length();

		size_t pos = 0;
		size_t last_pos = 0;
		size_t current_pos = 0;
		while (true)
		{
			if (last_pos >= source.size())
			{
				break;
			}
			pos = source.find(findThat, last_pos);
			if (pos == std::string::npos)
			{
				break;
			}
			current_pos = last_pos;
			last_pos = pos + findThat.size();

			//if we find any hand-specified precision modifier in the original shader code, AND we do not force highp, we skip replacing those
			std::string findThatLowp = "lowp " + findThat;
			std::string findThatMediump = "mediump " + findThat;
			std::string findThatHighp = "highp " + findThat;
			size_t lowp_pos = source.find(findThatLowp, current_pos);
			size_t mediump_pos = source.find(findThatMediump, current_pos);
			size_t highp_pos = source.find(findThatHighp, current_pos);
			if (!is_highp)
			{
				if (std::string::npos != lowp_pos) continue;
				if (std::string::npos != mediump_pos) continue;
				if (std::string::npos != highp_pos) continue;
			}
			else //force replace these to highp
			{
				if (std::string::npos != lowp_pos)
				{
					pos = lowp_pos;
					findThatLength = findThatLowp.length();
				}
				else if (std::string::npos != mediump_pos)
				{
					pos = mediump_pos;
					findThatLength = findThatMediump.length();
				}
				else if (std::string::npos != highp_pos)
				{
					// already highp, no need to replace
					continue;
				}
				else
				{
					// no precision qualifier
					findThatLength = findThat.length();
				}
			}

			std::string replWith = m_uniforms[i].precision;
			replWith += " ";
			replWith += findThat;
			source.replace(pos, findThatLength, "");
			source.insert(pos, replWith);
		}
	}
}


void GLBShaderCommon::InitShaderCommon()
{
	for( KCL::uint32 i=0; i<attribs::ATTRIB_COUNT; i++)
	{
		m_attribs[i].set( NULL, NULL );
	}
	for( KCL::uint32 i=0; i<uniforms::UNIFORM_COUNT; i++)
	{
		m_uniforms[i].set( NULL, NULL, NULL );
	}

	m_attribs[attribs::in_position          ].set( "in_position",           "vec3" );
	m_attribs[attribs::in_bone_weight       ].set( "in_bone_weight",        "vec4" );
	m_attribs[attribs::in_bone_index        ].set( "in_bone_index",         "vec4" );
	m_attribs[attribs::in_normal            ].set( "in_normal",             "vec3" );
	m_attribs[attribs::in_tangent           ].set( "in_tangent",            "vec3" );
	m_attribs[attribs::in_color             ].set( "in_color",              "vec3" );
	m_attribs[attribs::in_texcoord0         ].set( "in_texcoord0",          "vec2" );
	m_attribs[attribs::in_texcoord1         ].set( "in_texcoord1",          "vec2" );
	m_attribs[attribs::in_texcoord2         ].set( "in_texcoord2",          "vec2" );
	m_attribs[attribs::in_texcoord3         ].set( "in_texcoord3",          "vec2" );
	m_attribs[attribs::in_instance_position ].set( "in_instance_position",  "vec3" );
	m_attribs[attribs::in_instance_life     ].set( "in_instance_life",      "float" );
	m_attribs[attribs::in_instance_speed    ].set( "in_instance_speed",     "vec3" );
	m_attribs[attribs::in_instance_size     ].set( "in_instance_size",      "float" );
	//m_attribs[attribs::in_instance_mv       ].set( "in_instance_mv",        "mat4" );
	//m_attribs[attribs::in_instance_inv_mv   ].set( "in_instance_inv_mv",    "mat4" );
	m_attribs[attribs::in_instance_mv0      ].set( "in_instance_mv0",       "vec4" );
	m_attribs[attribs::in_instance_mv1      ].set( "in_instance_mv1",       "vec4" );
	m_attribs[attribs::in_instance_mv2      ].set( "in_instance_mv2",       "vec4" );
	m_attribs[attribs::in_instance_mv3      ].set( "in_instance_mv3",       "vec4" );
	m_attribs[attribs::in_instance_inv_mv   ].set( "in_instance_inv_mv",    "mat4" );
	m_attribs[attribs::in_instance_model    ].set( "in_instance_model",     "mat4" );
	m_attribs[attribs::in_instance_inv_model].set( "in_instance_inv_model", "mat4" );

	//frequency: per draw-call
	m_uniforms[uniforms::mvp                              ].set( "mvp",                               "mat4",             "highp"   ); // highp
	m_uniforms[uniforms::mv                               ].set( "mv",                                "mat4",             "highp"   ); //mediump
	m_uniforms[uniforms::inv_view                         ].set( "inv_view",                          "mat4",             "highp"   );
	m_uniforms[uniforms::vp                               ].set( "vp",                                "mat4",             "highp"   ); //mediump
	m_uniforms[uniforms::model                            ].set( "model",                             "mat4",             "highp"   );
	m_uniforms[uniforms::inv_model                        ].set( "inv_model",                         "mat4",             "highp"   );
	m_uniforms[uniforms::inv_modelview                    ].set( "inv_modelview",                     "mat4",             "highp"   );

	//frequency: per frame + per rendermode
	m_uniforms[uniforms::global_light_dir                 ].set( "global_light_dir",                  "vec3",             "lowp"    );
	m_uniforms[uniforms::global_light_color               ].set( "global_light_color",                "vec3",             "mediump" );
	m_uniforms[uniforms::view_dir                         ].set( "view_dir",                          "vec3",             "highp"   );
	m_uniforms[uniforms::view_pos                         ].set( "view_pos",                          "vec3",             "highp"   );
	m_uniforms[uniforms::time                             ].set( "time",                              "float",            "mediump" );
	m_uniforms[uniforms::background_color                 ].set( "background_color",                  "vec3",             "lowp"    );
	m_uniforms[uniforms::fog_density                      ].set( "fog_density",                       "float",            "lowp"    );
	m_uniforms[uniforms::inv_resolution                   ].set( "inv_resolution",                    "vec2",             "highp"   );

	//frequency: per material
	m_uniforms[uniforms::diffuse_intensity                ].set( "diffuse_intensity",                 "float",            "mediump" );
	m_uniforms[uniforms::specular_intensity               ].set( "specular_intensity",                "float",            "mediump" );
	m_uniforms[uniforms::reflect_intensity                ].set( "reflect_intensity",                 "float",            "mediump" );
	m_uniforms[uniforms::specular_exponent                ].set( "specular_exponent",                 "float",            "mediump" );
	m_uniforms[uniforms::transparency                     ].set( "transparency",                      "float",            "mediump" );
	m_uniforms[uniforms::fresnel_params                   ].set( "fresnel_params",                    "vec3",             "mediump" );

	m_uniforms[uniforms::bones                            ].set( "bones",                             "vec4",             "highp"   ); //lowp									   
	m_uniforms[uniforms::envmaps_interpolator             ].set( "envmaps_interpolator",              "float",            "lowp"    );
	m_uniforms[uniforms::light_pos                        ].set( "light_pos",                         "vec3",             "highp"   );
	m_uniforms[uniforms::shadow_matrix0                   ].set( "shadow_matrix0",                    "mat4",             "highp"   );
	m_uniforms[uniforms::offset_2d                        ].set( "offset_2d",                         "vec2",             "mediump" );
	m_uniforms[uniforms::color                            ].set( "color",                             "vec3",             "lowp"    );
	m_uniforms[uniforms::translate_uv                     ].set( "translate_uv",                      "vec2",             "mediump" );
	m_uniforms[uniforms::particle_data                    ].set( "particle_data",                     "vec4",             "highp"   );
	m_uniforms[uniforms::particle_color                   ].set( "particle_color",                    "vec4",             "mediump" );
	m_uniforms[uniforms::alpha_threshold                  ].set( "alpha_threshold",                   "float",            "mediump" );
	m_uniforms[uniforms::world_fit_matrix                 ].set( "world_fit_matrix",                  "mat4",             "highp"   );
	m_uniforms[uniforms::shadow_matrix1                   ].set( "shadow_matrix1",                    "mat4",             "highp"   );

	m_uniforms[uniforms::envmap0                          ].set( "envmap0",                           "samplerCube",      "lowp"    );
	m_uniforms[uniforms::envmap1                          ].set( "envmap1",                           "samplerCube",      "lowp"    );
    m_uniforms[uniforms::envmap1_dp                       ].set( "envmap1_dp",                        "sampler2DArray",   "lowp"    );
    m_uniforms[uniforms::envmap2_dp                       ].set( "envmap2_dp",                        "sampler2DArray",   "lowp"    );
	m_uniforms[uniforms::shadow_unit0                     ].set( "shadow_unit0",                      "sampler2DShadow",  "lowp"    );
	m_uniforms[uniforms::shadow_unit1                     ].set( "shadow_unit1",                      "sampler2DShadow",  "lowp"    );
	m_uniforms[uniforms::planar_reflection                ].set( "planar_reflection",                 "sampler2D",        "lowp"    );

	m_uniforms[uniforms::mvp2                             ].set( "mvp2",                              "mat4",             "highp"   );
	m_uniforms[uniforms::prev_bones                       ].set( "prev_bones",                        "vec4",             "highp"   ); //lowp							   
	m_uniforms[uniforms::mblur_mask                       ].set( "mblur_mask",                        "float",            "lowp"    ); //lowp							   
	m_uniforms[uniforms::envmap2                          ].set( "envmap2",                           "samplerCube",      "lowp"    );
	m_uniforms[uniforms::static_envmaps                   ].set( "static_envmaps",                    "samplerCubeArray", "lowp"    );

	m_uniforms[uniforms::camera_focus                     ].set( "camera_focus",                      "float",            "mediump" );
	m_uniforms[uniforms::light_color                      ].set( "light_color",                       "vec3",             "lowp"    );
	m_uniforms[uniforms::light_x                          ].set( "light_x",                           "vec4",             "lowp"    );
	m_uniforms[uniforms::attenuation_parameter            ].set( "attenuation_parameter",             "float",            "mediump" );
	m_uniforms[uniforms::spot_cos                         ].set( "spot_cos",                          "vec2",             "mediump" );
	m_uniforms[uniforms::depth_parameters                 ].set( "depth_parameters",                  "vec4",             "highp"   );
	m_uniforms[uniforms::dof_strength                     ].set( "dof_strength",                      "float",            "mediump" );

	m_uniforms[uniforms::light_positions                  ].set( "light_positions",                   "vec3",             "highp"   );
	m_uniforms[uniforms::light_colors                     ].set( "light_colors",                      "vec3",             "lowp"    );
	m_uniforms[uniforms::light_xs                         ].set( "light_xs",                          "vec4",             "lowp"    );
	m_uniforms[uniforms::spot_coss                        ].set( "spot_coss",                         "vec2",             "mediump" );
	m_uniforms[uniforms::num_lights                       ].set( "num_lights",                        "int",              "highp"   );
	m_uniforms[uniforms::attenuation_parameters           ].set( "attenuation_parameters",            "float",            "mediump" );
	m_uniforms[uniforms::corners                          ].set( "corners",		                      "vec3",             "highp"   );

	m_uniforms[uniforms::texture_unit0                    ].set( "texture_unit0",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_unit1                    ].set( "texture_unit1",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_unit2                    ].set( "texture_unit2",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_unit3                    ].set( "texture_unit3",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_unit4                    ].set( "texture_unit4",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_unit5                    ].set( "texture_unit5",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_unit6                    ].set( "texture_unit6",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_unit7                    ].set( "texture_unit7",                     "sampler2D",        "lowp"    );
	m_uniforms[uniforms::texture_array_unit0              ].set( "texture_array_unit0",               "sampler2DArray",   "lowp"    );
	m_uniforms[uniforms::texture_3D_unit0                 ].set( "texture_3D_unit0",                  "sampler3D",        "lowp"    );
	m_uniforms[uniforms::depth_unit0					  ].set( "depth_unit0",						  "sampler2D",		  "highp"   );

	//TODO check if below lines need highp, put into UBO
	m_uniforms[uniforms::deltatime                        ].set( "deltatime",                         "float",            "highp"   );
	m_uniforms[uniforms::particleBufferParamsXYZ_pad      ].set( "particleBufferParamsXYZ_pad",       "ivec4",            "highp"   );

	m_uniforms[uniforms::emitter_apertureXYZ_focusdist    ].set( "emitter_apertureXYZ_focusdist",     "vec4",             "mediump" );
	m_uniforms[uniforms::emitter_worldmat                 ].set( "emitter_worldmat",                  "mat4",             "highp"   );
	m_uniforms[uniforms::emitter_min_freqXYZ_speed        ].set( "emitter_min_freqXYZ_speed",         "vec4",             "mediump" );
	m_uniforms[uniforms::emitter_max_freqXYZ_speed        ].set( "emitter_max_freqXYZ_speed",         "vec4",             "mediump" );
	m_uniforms[uniforms::emitter_min_ampXYZ_accel         ].set( "emitter_min_ampXYZ_accel",          "vec4",             "mediump" );
	m_uniforms[uniforms::emitter_max_ampXYZ_accel         ].set( "emitter_max_ampXYZ_accel",          "vec4",             "mediump" );
	m_uniforms[uniforms::emitter_externalVel_gravityFactor].set( "emitter_externalVel_gravityFactor", "vec4",             "mediump" );
	m_uniforms[uniforms::emitter_maxlifeX_sizeYZ_pad      ].set( "emitter_maxlifeX_sizeYZ_pad",       "vec4",             "mediump" );
	m_uniforms[uniforms::emitter_numSubsteps              ].set( "emitter_numSubsteps",               "int",              "highp"   );

	m_uniforms[uniforms::tessellation_factor              ].set( "tessellation_factor",               "vec4",             "highp"   );
	m_uniforms[uniforms::hdr_params                       ].set( "hdr_params",                        "vec2",             "highp"   );
	m_uniforms[uniforms::car_ao_matrix0                   ].set( "car_ao_matrix0",                    "mat4",             "highp"   );
	m_uniforms[uniforms::car_ao_matrix1                   ].set( "car_ao_matrix1",                    "mat4",             "highp"   );

	m_uniforms[uniforms::view_port_size                   ].set( "view_port_size",                    "vec2",             "highp"   );
	m_uniforms[uniforms::mb_tile_size                     ].set( "mb_tile_size",                      "int",              "highp"   );

	m_uniforms[uniforms::cascaded_shadow_texture_array    ].set( "cascaded_shadow_texture_array",     "sampler2DArray",   "highp"   );
	m_uniforms[uniforms::cascaded_shadow_matrices         ].set( "cascaded_shadow_matrices",          "mat4",             "highp"   );
	m_uniforms[uniforms::cascaded_frustum_distances       ].set( "cascaded_frustum_distances",        "vec4",             "highp"   );

	m_uniforms[uniforms::carindex_translucency_ssaostr_fovscale].set( "carindex_translucency_ssaostr_fovscale",     "vec4",             "mediump" );
	m_uniforms[uniforms::ambient_colors                   ].set( "ambient_colors",              	  "vec4",             "mediump" );

	m_uniforms[uniforms::prev_vp                          ].set( "prev_vp",                           "mat4",             "highp"   );

	m_uniforms[uniforms::gauss_lod_level                  ].set( "gauss_lod_level",                   "int",              "highp"   );
	m_uniforms[uniforms::gauss_offsets                    ].set( "gauss_offsets",                     "float",            "highp"   );
	m_uniforms[uniforms::gauss_weights                    ].set( "gauss_weights",                     "float",            "highp"   );

	m_uniforms[uniforms::view                             ].set( "view",                              "mat4",             "highp"   );
    m_uniforms[uniforms::cam_near_far_pid_vpscale         ].set( "cam_near_far_pid_vpscale",          "vec4",             "highp"   );

    m_uniforms[uniforms::instance_offset                  ].set( "instance_offset",                   "int",              "mediump");

    m_uniforms[uniforms::dpcam_view                       ].set( "dpcam_view",                        "mat4",             "highp"   );
    m_uniforms[uniforms::frustum_planes                   ].set( "frustum_planes",                    "vec4",             "highp"   );
    m_uniforms[uniforms::gamma_exp                        ].set( "gamma_exp",                         "vec4",             "highp"   );

    m_uniforms[uniforms::camera_focus_inv                 ].set( "camera_focus_inv",                  "float",            "mediump" );
    m_uniforms[uniforms::dof_strength_inv                 ].set( "dof_strength_inv",                  "float",            "mediump" );

    m_uniforms[uniforms::camera_focus_inv                 ].set( "camera_focus_inv",                  "float",            "mediump" );
    m_uniforms[uniforms::dof_strength_inv                 ].set( "dof_strength_inv",                  "float",            "mediump" );

    m_uniforms[uniforms::editor_mesh_selected             ].set( "editor_mesh_selected",              "int",              "mediump" );

    m_uniforms[uniforms::near_far_ratio                   ].set("near_far_ratio",                     "float",            "highp"   );   
    m_uniforms[uniforms::hiz_texture                      ].set("hiz_texture",                        "sampler2D",        "highp"   );

    m_uniforms[uniforms::tessellation_multiplier          ].set("tessellation_multiplier",            "float",            "highp"   );

    m_uniforms[uniforms::projection_scale                 ].set("projection_scale",                   "float",            "highp"   );

	m_vs_float_prec = HIGHP;
    m_fs_float_prec = MEDIUMP;

	if (is_highp)
	{
		NGLOG_INFO("Forcing high precision");
		for (int i=0;i<uniforms::UNIFORM_COUNT;++i)
		{
			m_uniforms[i].precision = "highp";
		}
		m_fs_float_prec = HIGHP;
		m_vs_float_prec = HIGHP;
	}
	else
	{
		NGLOG_INFO("Using normal precision");
	}

	if(m_vs_float_prec == HIGHP)
	{
		NGLOG_INFO("Using highp in vs.");
	}
	else if(m_vs_float_prec == MEDIUMP)
	{
		NGLOG_INFO("Using mediump in vs.");
	}
	else if(m_vs_float_prec == LOWP)
	{
		NGLOG_INFO("Using lowp in vs.");
	}
	else
	{
		NGLOG_INFO("Using default precision in vs.");
	}

	if(m_fs_float_prec == HIGHP)
	{
		NGLOG_INFO("Using highp in fs.");
	}
	else if(m_fs_float_prec == MEDIUMP)
	{
		NGLOG_INFO("Using mediump in fs.");
	}
	else if(m_fs_float_prec == LOWP)
	{
		NGLOG_INFO("Using lowp in fs.");
	}
	else
	{
		NGLOG_INFO("Using default precision in vs.");
	}
}

std::string GLBShaderCommon::GetVertexPrecisionHeader()
{
	std::string vs_header = "";
	
	if( m_vs_float_prec == HIGHP)
	{
		vs_header += "#ifdef GL_ES\nprecision highp float;\n#endif\n";
	}
	else if( m_vs_float_prec == MEDIUMP)
	{
		vs_header += "#ifdef GL_ES\nprecision mediump float;\n#endif\n";
	}
	else if( m_vs_float_prec == LOWP)
	{
		vs_header += "#ifdef GL_ES\nprecision lowp float;\n#endif\n";
	}

	vs_header += "#ifndef GL_ES\n#define highp\n#define mediump\n#define lowp\n#endif\n";

	return vs_header ;
}

std::string GLBShaderCommon::GetFragmentPrecisionHeader()
{
	std::string fs_header = "";

	if( m_fs_float_prec == HIGHP)
	{
		fs_header += "#ifdef GL_ES\nprecision highp float;\n#endif\n";
	}
	else if( m_fs_float_prec == MEDIUMP)
	{
		fs_header += "#ifdef GL_ES\nprecision mediump float;\n#endif\n";
	}
	else if( m_fs_float_prec == LOWP)
	{
		fs_header += "#ifdef GL_ES\nprecision lowp float;\n#endif\n";
	}
	
	fs_header += "#ifndef GL_ES\n#define highp\n#define mediump\n#define lowp\n#endif\n";
	

	if (( m_fs_float_prec != NONEP) && !is_highp)
	{
		fs_header += "#ifdef GL_ES\n#if defined LIGHTING || defined REFLECTION || defined DEP_TEXTURING || defined TRANSITION_EFFECT\nprecision mediump float;\n#else\nprecision lowp float;\n#endif\n#endif\n";
		fs_header += "#ifdef GL_ES\n#if defined NEED_HIGHP\nprecision highp float;\n#endif\n#endif\n";
	}

	return fs_header ;
}


}  // end namespace GLB 

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
size_t CalcInstrCount(const std::string &source, const CALCULATE_SHADER type)
{
	std::string header = "";
#if defined HAVE_GLES3 || defined __glew_h__ //GFXBench3 only
	header = "#version 420 core\n";
#endif
	std::string finalSource = header + source;

	const std::string path_of_cgc = MY_CGC_PATH;
	const std::string tmp_in_file = "tmp_in";
	const std::string tmp_out_file = "tmp_out";
	
	{
		std::string tmpfile_str = path_of_cgc + tmp_in_file;
		std::ofstream tmpfile(tmpfile_str.c_str());
		tmpfile << finalSource << std::endl;
		tmpfile.close();
	}

	std::string command = path_of_cgc;

	#ifdef WIN32
	command += "cgc.exe -oglsl -profile ";
	#else //we believe we are on LINUX
	command += "cgc -oglsl -profile ";
	#endif

	//command += CALC_VERTEX == type ? "arbvp1 " : "arbfp1 ";
	command += CALC_VERTEX == type ? "gp5vp " : "gp5fp ";
	command += path_of_cgc;
	command += tmp_in_file;
	command += " -o ";
	command += path_of_cgc;
	command += tmp_out_file;

	system( command.c_str() );

	size_t result = 0;
	
	{
		std::string tmpfile_str = path_of_cgc + tmp_out_file;
		std::ifstream tmpfile(tmpfile_str.c_str());

		std::string line="", END="END";

		while(line != END)
			std::getline(tmpfile, line);

		char b;

		tmpfile >> b;
		tmpfile >> result;
		tmpfile.close();
	}

	return result;
}
#endif