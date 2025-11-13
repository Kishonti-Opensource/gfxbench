/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_SHADER_COMMON_H
#define GLB_SHADER_COMMON_H

#include <string>
#include <vector>
#include <list>

#include "kcl_base.h"
#include "render_statistics_defines.h"

namespace GLB
{

	struct attrib {
		const char* name;
		const char* type;
		void set(const char* const n, const char* const t) { name = n; type = t; };
	};

	struct uniform {
		const char* name;
		const char* type;
		const char* precision;
		void set(const char* const n, const char* const t, const char* const p) { name = n; type = t; precision = p; };
	};

	struct attribs {
		enum {
			in_position = 0,
			in_bone_weight,
			in_bone_index,
			in_normal,
			in_tangent,
			in_color,
			in_texcoord0,
			in_texcoord1,
			in_texcoord2,
			in_texcoord3,
			in_instance_position,
			in_instance_life,
			in_instance_speed,
			in_instance_size,
			in_instance_mv0,
			in_instance_mv1,
			in_instance_mv2,
			in_instance_mv3,
			in_instance_inv_mv,
			in_instance_model,
			in_instance_inv_model,
			ATTRIB_COUNT
		};
	};

	struct uniforms {
		enum {
			mvp = 0,
			mv,
			bones,
			vp,
			model,
			inv_model,
			global_light_dir,
			global_light_color,
			view_dir,
			view_pos,
			time,
			background_color,
			fog_density,
			diffuse_intensity,
			specular_intensity,
			reflect_intensity,
			specular_exponent,
			transparency,
			envmaps_interpolator,
			fresnel_params,
			light_pos,
			shadow_matrix0,
			offset_2d,				
			inv_resolution,			
			color,
			translate_uv,
			particle_data,
			particle_color,
			alpha_threshold,
			world_fit_matrix,
			shadow_matrix1,
			envmap0 = 36,
			envmap1,				
            envmap1_dp,
            envmap2_dp,
			shadow_unit0,			
			shadow_unit1,			
			planar_reflection,		
			mvp2,					
			prev_bones,		 		
			mblur_mask,		 		
			envmap2,				
			static_envmaps,			
			camera_focus = 64,			
			light_color,			
			light_x,				
			attenuation_parameter,	
			spot_cos,				
			depth_parameters,		
			dof_strength,			
			light_positions = 80,		
			light_colors,			
			light_xs,				
			spot_coss,				
			num_lights,				
			attenuation_parameters,	
			corners,				
			texture_unit0 = 100,
			texture_unit1,
			texture_unit2,
			texture_unit3,
			texture_unit4,
			texture_unit5,
			texture_unit6,
			texture_unit7,
			texture_array_unit0,
			texture_3D_unit0,
			depth_unit0,
			deltatime,
			particleBufferParamsXYZ_pad,
			inv_modelview,
			emitter_worldmat,
			emitter_min_freqXYZ_speed,
			emitter_max_freqXYZ_speed,
			emitter_min_ampXYZ_accel,
			emitter_max_ampXYZ_accel,
			emitter_externalVel_gravityFactor,
			emitter_maxlifeX_sizeYZ_pad,
			emitter_apertureXYZ_focusdist,
			emitter_numSubsteps,
			tessellation_factor = 204,
			hdr_params = 216,
			car_ao_matrix0,
			car_ao_matrix1,
			view_port_size = 220,
			mb_tile_size,
			inv_view,
			cascaded_shadow_texture_array,
			cascaded_shadow_matrices,
			cascaded_frustum_distances,
			carindex_translucency_ssaostr_fovscale,
			ambient_colors,
			prev_vp,
			gauss_lod_level,
			gauss_weights,
			gauss_offsets,
			view,
            cam_near_far_pid_vpscale,
            instance_offset,
            dpcam_view,
            frustum_planes,
            gamma_exp,
            dof_strength_inv,
            camera_focus_inv,
            editor_mesh_selected,
            near_far_ratio,
            hiz_texture,
            tessellation_multiplier,
            projection_scale,
			UNIFORM_COUNT = 256
		};
	};

class GLBShaderCommon
{
public:

	enum PRECISION 
	{
		NONEP,
		LOWP,
		MEDIUMP,
		HIGHP
	};

	KCL::uint32 m_p;

	KCL::int32 m_uniform_locations[uniforms::UNIFORM_COUNT ];
	KCL::int32 m_attrib_locations[attribs::ATTRIB_COUNT];

	KCL::KCL_Status CompileShader(KCL::uint32 shader, const std::string & source);
	KCL::KCL_Status LinkProgram();
	void Validateprogram();

    std::vector<std::string> &GetSourceFiles()
    {
        return m_source_files;
    }
	
	static std::string GetVertexPrecisionHeader() ;
	static std::string GetFragmentPrecisionHeader() ;
	static void AddPrecisionQualifierToUniforms(std::string &source);

	static bool is_highp;

protected:
	GLBShaderCommon() ;
	virtual ~GLBShaderCommon() ;

	static uniform m_uniforms[uniforms::UNIFORM_COUNT];
	static attrib m_attribs[attribs::ATTRIB_COUNT];

	static PRECISION m_vs_float_prec;
	static PRECISION m_fs_float_prec;
	static PRECISION m_vs_int_prec;
	static PRECISION m_fs_int_prec;

	static void InitShaderCommon();

	void InitShaderLocations(bool has_vertex_shader);
		
	void DumpSourceFileNames();

	std::string m_glsl_version_string;
	std::vector<std::string> m_source_files;

	static bool LogShaderWarnings;

public:
	size_t m_instruction_count_v;
	size_t m_instruction_count_f;
    //size_t m_instruction_count_c; //this is done differently
};

}

template <typename T>
std::string ConcatenateShaderSourceFromContainer(const T &ShaderContainer) {
	typename T::const_iterator it ;
	
	std::string res = "" ;

	for (it = ShaderContainer.begin(); it != ShaderContainer.end() ; it++) {
		res += (*it) ;
	}
	
	return res ;
}

inline std::string ConcatenateShaderSourceFromCharArray(const char ** sources, int count) {
	std::vector<std::string> s ;
	
	for (int i = 0 ; i < count ; i++) {
		s.push_back(std::string(sources[i])) ;
	}

	return ConcatenateShaderSourceFromContainer(s) ;
}

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
#include <cstdlib>
#include <fstream>

enum CALCULATE_SHADER {CALC_VERTEX, CALC_FRAGMENT};

size_t CalcInstrCount(const std::string &source, const CALCULATE_SHADER type);

#endif

#endif // GLB_SHADER_COMMON_H
