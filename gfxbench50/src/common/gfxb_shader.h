/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_SHADER_H
#define GFXB5_SHADER_H

#include <map>
#include <ngl.h>
#include <kcl_base.h>
#include <kcl_serializable.h>

namespace Poco
{
	class Mutex;
}

namespace GFXB
{
	void LoadShader(NGL_job_descriptor &jd, uint32_t pass, uint32_t shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms);

	enum Uniform
	{
		// Frame
		UNIFORM_DELTA_TIME = 0,
		UNIFORM_TIME,

		// Camera
		UNIFORM_VIEW_POS,
		UNIFORM_VIEW_DIR,
		UNIFORM_VP,
		UNIFORM_PREV_VP,
		UNIFORM_VIEW,
		UNIFORM_INV_VP,
		UNIFORM_DEPTH_PARAMETERS,

		UNIFORM_FRUSTUM_PLANES,
		UNIFORM_CORNERS,

		// Mesh
		UNIFORM_MODEL,
		UNIFORM_INV_MODEL,
		UNIFORM_MV,
		UNIFORM_MVP,
		UNIFORM_PREV_MVP,
		UNIFORM_BONES,
		UNIFORM_PREV_BONES,

		// Material
		UNIFORM_COLOR_TEX,
		UNIFORM_NORMAL_TEX,
		UNIFORM_SPECULAR_TEX,
		UNIFORM_DISPLACEMENT_TEX,
		UNIFORM_EMISSIVE_TEX,
		UNIFORM_EMISSIVE_INTENSITY,
		UNIFORM_AUX_TEX0,
		UNIFORM_AUX_TEX1,
		UNIFORM_ALPHA_TEST_THRESHOLD,
		UNIFORM_ROUGHNESS,
		UNIFORM_DITHER_VALUE,

		// Lights
		UNIFORM_LIGHT_VP,
		UNIFORM_LIGHT_POS,
		UNIFORM_LIGHT_DIR,
		UNIFORM_LIGHT_COLOR,
		UNIFORM_SPOT_COS,
		UNIFORM_ATTENUATION_PARAMETERS,
		UNIFORM_LIGHT_FUSTUM_PLANES,
		UNIFORM_SKY_COLOR,
		UNIFORM_GROUND_COLOR,
		UNIFORM_SHADOW_LIGHT_POS,

		// Light shaft
		UNIFORM_LIGHTSHAFT_PARAMETERS,
		UNIFORM_VERTEX_ARRAY,
		UNIFORM_BAYER_ARRAY,

		// OVR Tessellation
		UNIFORM_CAM_NEAR,

		// Fire
		UNIFORM_FIRE_TIME,

		// Shadow
		UNIFORM_SHADOW_MAP,
		UNIFORM_SHADOW_MAP0,
		UNIFORM_SHADOW_MAP1,
		UNIFORM_SHADOW_MAP2,
		UNIFORM_SHADOW_MAP_SIZE0,
		UNIFORM_SHADOW_MAP_SIZE1,
		UNIFORM_SHADOW_MAP_SIZE2,
		UNIFORM_SHADOW_MATRIX,
		UNIFORM_SHADOW_MATRIX0,
		UNIFORM_SHADOW_MATRIX1,
		UNIFORM_SHADOW_MATRIX2,
		UNIFORM_SHADOW_MATRICES,
		UNIFORM_SHADOW_FRUSTUM_DISTANCES,

		// SSAO
		UNIFORM_world2_and_screen_radius,
		UNIFORM_SAO_PROJECTION_SCALE,

		// IBL
		UNIFORM_BRDF,
		UNIFORM_IBL_DIFFUSE_INTENSITY,
		UNIFORM_IBL_REFLECTION_INTENSITY,

		// GI
		UNIFORM_INDIRECT_LIGHTING_FACTOR,
		UNIFORM_direct_lightmap,
		UNIFORM_envprobe_indirect_uv_map,
		UNIFORM_envprobe_sh_atlas,
		UNIFORM_envprobe_sh_atlas_texture,
		UNIFORM_envprobe_envmap_atlas,
		UNIFORM_envprobe_index,
		UNIFORM_envprobe_inv_half_extent,

		// Fog
		UNIFORM_FOG_PARAMETERS1,
		UNIFORM_FOG_PARAMETERS2,
		UNIFORM_FOG_COLOR,

		// Tone mapper
		UNIFORM_HDR_REDUCTION_VALUES,
		UNIFORM_HDR_ABCD,
		UNIFORM_HDR_EFW_TAU,
		UNIFORM_HDR_TONEMAP_WHITE,
		UNIFORM_HDR_AUTO_EXPOSURE_VALUES,
		UNIFORM_HDR_EXPOSURE,
		UNIFORM_HDR_PREDEFINED_LUMINANACE,

		UNIFORM_HDR_UV_TEST_TEXTURE,

		// Bloom
		UNIFORM_BLOOM_PARAMETERS,

		// Color correction
		UNIFORM_COLOR_CORRECTION,
		UNIFORM_SHARPEN_FILTER,

		// Motion blur
		UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR,
		UNIFORM_TILE_MAX_BUFFER,
		UNIFORM_NEIGHBOR_MAX_TEXTURE,
		UNIFORM_TAP_OFFSETS,
		UNIFORM_MB_TILE_UV_TEST_TEXTURE,

		// DoF
		UNIFORM_DOF_INPUT_TEXTURE,
		UNIFORM_DOF_PARAMETERS,

		// Gauss blur
		UNIFORM_GAUSS_LOD_LEVEL,

		// Debug
		UNIFORM_MIP_TEX,
		UNIFORM_MIN_IDEAL_MAX_TEXTURE_DENSITY,
		UNIFORM_TEXSIZE_SIZE,
		UNIFORM_PERIMETER_THRESHOLD,
		UNIFORM_SHADOW_MAP_DIRECT_DEBUG,
		UNIFORM_SHADOW_MAP_INDIRECT_DEBUG,
		UNIFORM_POS0,
		UNIFORM_POS1,
		UNIFORM_COLOR,
		UNIFORM_COLOR0,
		UNIFORM_COLOR1,
		UNIFORM_DEBUG_FRUSTUM_IDS,
		UNIFORM_DEBUG_WIREFRAME_MODE,
		UNIFORM_DEBUG_MESH_LOD_INDEX,

		// Render targets
		UNIFORM_GBUFFER_COLOR_TEX,
		UNIFORM_GBUFFER_NORMAL_TEX,
		UNIFORM_GBUFFER_SPECULAR_TEX,
		UNIFORM_GBUFFER_EMISSIVE_TEX,
		UNIFORM_GBUFFER_VELOCITY_TEX,
		UNIFORM_GBUFFER_DEPTH_TEX,
		UNIFORM_DEPTH_DOWNSAMPLE_TEX,
		UNIFORM_LIGHTING_TEX,
		UNIFORM_SSAO_TEXTURE,
		UNIFORM_BLURRED_SSAO_TEX,
		UNIFORM_ENVMAP0,
		UNIFORM_ENVMAP1,
		UNIFORM_BRIGHT_TEXTURE,
		UNIFORM_BRIGHT_TEXTURE0,
		UNIFORM_BRIGHT_TEXTURE1,
		UNIFORM_BRIGHT_TEXTURE2,
		UNIFORM_BRIGHT_TEXTURE3,
		UNIFORM_BLOOM_TEXTURE,
		UNIFORM_BLOOM_TEXTURE0,
		UNIFORM_BLOOM_TEXTURE1,
		UNIFORM_BLOOM_TEXTURE2,
		UNIFORM_BLOOM_TEXTURE3,
		UNIFORM_FINAL_TEXTURE,
		UNIFORM_MOTION_BLUR_TEXTURE,
		UNIFORM_DOF_BLUR_TEXTURE,
		UNIFORM_HALF_RES_TRANSPARENT_TEXTURE,
		UNIFORM_INDIRECT_WEIGHT_TEXTURE,

		// Particle system
		UNIFORM_PARTICLE_UPLOAD_POOL,
		UNIFORM_PARTICLE_UPLOAD_SIZE,
		UNIFORM_PARTICLE_POSITION,
		UNIFORM_PARTICLE_SIZE_ROTATION_OPACITY,
		UNIFORM_FLIPBOOK_FRAME,

		// FSAA
		UNIFORM_g_Depth,
		UNIFORM_g_screenTexture,
		UNIFORM_g_resultTextureFlt4Slot1,
		UNIFORM_g_resultTexture,
		UNIFORM_g_src0Texture4Uint,
		UNIFORM_g_src0TextureFlt,
		UNIFORM_g_resultTextureSlot2,
		UNIFORM_OneOverScreenSize,
		UNIFORM_albedo_tex,
		UNIFORM_area_tex,
		UNIFORM_edge_tex,
		UNIFORM_search_tex,
		UNIFORM_blend_tex,

		// Common uniforms
		UNIFORM_NUM_WORK_GROUPS,
		UNIFORM_CENTER,
		UNIFORM_SCALE,
		UNIFORM_INV_RESOLUTION,
		UNIFORM_TEXTURE_SAMPLES_INV,
		UNIFORM_LOD_LEVEL,
		UNIFORM_INPUT_TEXTURE,
		UNIFORM_INPUT_TEXTURE_LOD,
		UNIFORM_TEXTURE_UNIT0,
		UNIFORM_TEXTURE_UNIT7,
		UNIFORM_TEXEL_CENTER,
		UNIFORM_SAMPLE_COUNT,
		UNIFORM_STEP_UV,

		// Car Chase
		UNIFORM_CAM_NEAR_FAR_PID_VPSCALE,
		UNIFORM_TESSELLATION_FACTOR,
		UNIFORM_TESSELLATION_MULTIPLIER,

		// ADAS
		UNIFORM_VIEW_MATRIX,
		UNIFORM_Environment_BRDF,
		UNIFORM_FreiChenK,

		UNIFORM_MAX
	};


	class ShaderDescriptor : public KCL::Serializable
	{
	public:
		ShaderDescriptor();
		ShaderDescriptor(const char *filename_compute);
		ShaderDescriptor(const char *filename_vert, const char *filename_frag);

		ShaderDescriptor &SetName(const char *name);

		ShaderDescriptor &AddHeaderFile(const char* filename);

		ShaderDescriptor &SetVSFile(const char* filename);
		ShaderDescriptor &SetTCSFile(const char *filename);
		ShaderDescriptor &SetTESFile(const char *filename);
		ShaderDescriptor &SetGSFile(const char *filename);
		ShaderDescriptor &SetFSFile(const char* filename);
		ShaderDescriptor &SetCSFile(const char* filename);

		ShaderDescriptor &AddDefine(const char *define);
		ShaderDescriptor &AddDefineInt(const char* define, KCL::int32 value);
		ShaderDescriptor &AddDefineUInt(const char* define, KCL::uint32 value);
		ShaderDescriptor &AddDefineFloat(const char* define, float value);
		ShaderDescriptor &AddDefineHalf(const char* define, float value);
		ShaderDescriptor &AddDefineString(const char* define, const char* value);
		ShaderDescriptor &AddDefineVec2(const char* define, const KCL::Vector2D &value);

		void SetUniforms(const std::vector<NGL_shader_uniform> &uniforms);
		const std::vector<NGL_shader_uniform> &GetUniforms() const;

		ShaderDescriptor &SetWorkgroupSize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 z);
		KCL::uint32 GetWorkgroupSizeX() const;
		KCL::uint32 GetWorkgroupSizeY() const;
		KCL::uint32 GetWorkgroupSizeZ() const;

		const std::string &GetName() const;
		const std::vector<std::string> &GetHeaderFiles() const;
		const std::string &GetFilename(NGL_shader_type type) const;
		std::string GetDefinesString() const;

		// KCL::Serializable
		virtual void Serialize(JsonSerializer& s) override;
		virtual std::string GetParameterFilename() const override;

		// Operators
		const bool operator==(const ShaderDescriptor& other)const;

	private:
		std::string m_name;

		std::vector<std::string> m_header_files;

		std::string m_filenames[NGL_NUM_SHADER_TYPES];
		std::map<std::string, std::string> m_defines;
		std::vector<NGL_shader_uniform> m_uniforms;
		KCL::uint32 m_workgroup_size_x;
		KCL::uint32 m_workgroup_size_y;
		KCL::uint32 m_workgroup_size_z;
	};


	class PipelineCache
	{
	public:
		PipelineCache();
		~PipelineCache();

		class Key
		{
		public:
			Key(const KCL::uint32 sc, const NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES]);
			bool operator<(const Key& other) const;
		private:
			KCL::uint32 m_sc;
			std::vector<NGL_shader_source_descriptor> m_descriptors;
		};

		bool Search(const Key &key, NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES]);
		void Add(const Key &key, const NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES]);
	private:

		Poco::Mutex* m_mutex;
		std::map<Key, std::vector<NGL_shader_source_descriptor>> m_data;
	};


	class ShaderCache
	{
	public:
		ShaderCache();
		~ShaderCache();

		bool Search(NGL_shader_source_descriptor &ssd);
		void Add(const NGL_shader_source_descriptor &key, const NGL_shader_source_descriptor &value);
	private:

		Poco::Mutex* m_mutex;
		std::map<std::string, NGL_shader_source_descriptor> m_data;
	};


	class ShaderFactory : public KCL::Serializable
	{
	public:
        static void CreateInstance();
		static ShaderFactory *GetInstance();
		static void Release();

		ShaderFactory &ClearGlobals();

		KCL::uint32 AddDescriptor(const ShaderDescriptor &shader_descriptor);
		const ShaderDescriptor *GetDescriptor(const char *name) const;
		void GetNamedDescriptors(std::vector<ShaderDescriptor> &descriptors);

		ShaderFactory &AddShaderSubdirectory(const char *directory);

		ShaderFactory &ClearShaderSubdirectories();
		ShaderFactory &SetGlobalHeader(const char *header);

		ShaderFactory &ClearGlobalHeaderFiles();
		ShaderFactory &AddGlobalHeaderFile(const char* filename);

		ShaderFactory &ClearGlobalDefines();
		ShaderFactory &AddGlobalDefine(const char *define);
		ShaderFactory &AddGlobalDefineInt(const char *define, KCL::int32 value);
		ShaderFactory &AddGlobalDefineFloat(const char *define, float value);
		ShaderFactory &SetForceHighp(bool force_highp);

		const std::string &GetGlobalHeader() const;
		std::string GetGlobalDefinesString() const;

		void Create(NGL_job_descriptor &jd, uint32_t pass, uint32_t shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms);

		// KCL::Serializable
		virtual void Serialize(JsonSerializer& s) override;
		virtual std::string GetParameterFilename() const override;

		void EnableParser(bool yes)
		{
			m_parser_enabled = yes;
		}

	private:
		static ShaderFactory *instance;
		PipelineCache m_pipeline_cache;
		ShaderCache m_shader_cache;

		std::vector<std::string> m_directories;
		std::vector<std::string> m_global_header_files;
		std::string m_global_header;
		std::map<std::string, std::string> m_global_defines;
		std::map<KCL::uint32, ShaderDescriptor> m_shader_descriptors;
		uint32_t m_generated_shader_code_counter;
		bool m_parser_enabled;
		bool m_force_highp;

		ShaderFactory()
		{
			m_generated_shader_code_counter = 1;
			m_parser_enabled = true;
			m_force_highp = false;
		}
		~ShaderFactory() {}

		bool AppendShaderFile(const std::string &filename, std::stringstream &sstream);
		static const char* GetShaderTypeDefine(NGL_shader_type type);
		static void InitCommonUniforms(std::vector<NGL_shader_uniform> &uniforms);

		bool CompileWithPipelineCache(NGL_shader_source_descriptor descriptor[NGL_NUM_SHADER_TYPES], const uint32_t sc, const ShaderDescriptor &shader_descriptor, const std::vector<NGL_shader_uniform> &uniforms);
		bool CompileWithShaderCache(NGL_shader_source_descriptor descriptor[NGL_NUM_SHADER_TYPES], const ShaderDescriptor &shader_descriptor, const std::vector<NGL_shader_uniform> &uniforms);
	};
}

#endif