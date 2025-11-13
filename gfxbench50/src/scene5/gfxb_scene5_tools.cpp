/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene5_tools.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_cubemap.h"

#include <kcl_io.h>
#include <kcl_os.h>
#include <jsonserializer.h>
#include <sstream>
#include <fstream>

using namespace GFXB;


TierLevel Scene5Tools::ResolveTierLevel(const std::string &name)
{
	if (name == "normal")
	{
		return TIER_NORMAL;
	}
	else if (name == "high")
	{
		return TIER_HIGH;
	}

	return TIER_INVALID;
}


bool Scene5Tools::LoadRenderFlags(KCL::uint32 &flags)
{
	// Set the input filename
	std::string input_filename = "render_flags.json";

	// Open the JSON file
	KCL::AssetFile parameter_file(input_filename, true);
	if (!parameter_file.Opened())
	{
		parameter_file.Close();

		return false;
	}

	// Read the file as std::string
	std::string json_string = parameter_file.ReadToStdString();

	// Parse the string
	JsonSerializer s(false);
	ng::Result status;
	s.JsonValue.fromString(json_string.c_str(), status);

	if (status.ok())
	{
		// Serialize the object
		SerializeRenderFlags(s, flags);
	}
	else
	{
		// JSON serialization error
		std::stringstream sstream;
		sstream << "Serializable::LoadParameters - Error: Can not parse: " << input_filename << " " << status.what();
		std::string error_msg = sstream.str();

		INFO("%s", error_msg.c_str());
		INFO("%s", json_string.c_str());

		throw KCL::IOException(error_msg);
	}

	return true;
}


void Scene5Tools::SaveRenderFlags(KCL::uint32 flags)
{
	// Serialize the object
	JsonSerializer s(true);
	SerializeRenderFlags(s, flags);

	// Set the output filename
	std::string output_filename = "render_flags.json";

	// Write the JSON to the file
	KCL::File parameter_file(KCL::File::GetDataRWPath() + output_filename, KCL::Write, KCL::RDir, true);
	if (parameter_file.Opened())
	{
		parameter_file.Write(s.JsonValue.toString());
		parameter_file.Close();
	}
	else
	{
		INFO("Serializable::SaveParameters - Error: Can not open parameter file: %s\n", parameter_file.getFilename().c_str());
		parameter_file.Close();
	}
}


struct Flag
{
	std::string m_name;
	int m_flag;
	bool m_value;

	Flag()
	{
		m_flag = 0;
		m_value = 0;
	}

	Flag(const char *name, int flag, bool value)
	{
		m_name = name;
		m_flag = flag;
		m_value = value;
	}
};


void Scene5Tools::SerializeRenderFlags(JsonSerializer& s, KCL::uint32 flags)
{
	Flag flag_array[RenderOpts::NUMBER_OF_FLAGS];

	flag_array[RenderOpts::FLAG_FP_RENDER_TARGETS] = Flag("FLAG_FP_RENDER_TARGETS", RenderOpts::FLAG_FP_RENDER_TARGETS, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_FP_RENDER_TARGETS));
	flag_array[RenderOpts::FLAG_NORMAL_MAPPING] = Flag("FLAG_NORMAL_MAPPING", RenderOpts::FLAG_NORMAL_MAPPING, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_NORMAL_MAPPING));
	flag_array[RenderOpts::FLAG_RENDER_PROBES_ATLAS] = Flag("FLAG_RENDER_PROBES_ATLAS", RenderOpts::FLAG_RENDER_PROBES_ATLAS, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_RENDER_PROBES_ATLAS));
	flag_array[RenderOpts::FLAG_RENDER_PROBES_SH] = Flag("FLAG_RENDER_PROBES_SH", RenderOpts::FLAG_RENDER_PROBES_SH, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_RENDER_PROBES_SH));
	flag_array[RenderOpts::FLAG_RENDER_IRRADIANCE_MESH] = Flag("FLAG_RENDER_IRRADIANCE_MESH", RenderOpts::FLAG_RENDER_IRRADIANCE_MESH, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_RENDER_IRRADIANCE_MESH));

	flag_array[RenderOpts::FLAG_DIRECT_SHADOWS] = Flag("FLAG_DIRECT_SHADOWS", RenderOpts::FLAG_DIRECT_SHADOWS, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_DIRECT_SHADOWS));
		flag_array[RenderOpts::FLAG_SSAO] = Flag("FLAG_SSAO", RenderOpts::FLAG_SSAO, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_SSAO));
	flag_array[RenderOpts::FLAG_MID_RANGE_SSAO] = Flag("FLAG_MID_RANGE_SSAO", RenderOpts::FLAG_MID_RANGE_SSAO, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_MID_RANGE_SSAO));

	flag_array[RenderOpts::FLAG_DIRECT_LIGHTING] = Flag("FLAG_DIRECT_LIGHTING", RenderOpts::FLAG_DIRECT_LIGHTING, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_DIRECT_LIGHTING));
	flag_array[RenderOpts::FLAG_IRRADIANCE_LIGHTING] = Flag("FLAG_IRRADIANCE_LIGHTING", RenderOpts::FLAG_IRRADIANCE_LIGHTING, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_IRRADIANCE_LIGHTING));
	flag_array[RenderOpts::FLAG_IBL] = Flag("FLAG_IBL", RenderOpts::FLAG_IBL, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_IBL));

	flag_array[RenderOpts::FLAG_HDR] = Flag("FLAG_HDR", RenderOpts::FLAG_HDR, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_HDR));
	flag_array[RenderOpts::FLAG_BLOOM] = Flag("FLAG_BLOOM", RenderOpts::FLAG_BLOOM, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_BLOOM));
	flag_array[RenderOpts::FLAG_MOTION_BLUR] = Flag("FLAG_MOTION_BLUR", RenderOpts::FLAG_MOTION_BLUR, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_MOTION_BLUR));
	flag_array[RenderOpts::FLAG_DOF] = Flag("FLAG_DOF", RenderOpts::FLAG_DOF, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_DOF));

	flag_array[RenderOpts::FLAG_LIGHTSHAFT] = Flag("FLAG_LIGHTSHAFT", RenderOpts::FLAG_LIGHTSHAFT, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_LIGHTSHAFT));
	flag_array[RenderOpts::FLAG_PARTICLE_SYSTEMS] = Flag("FLAG_PARTICLE_SYSTEMS", RenderOpts::FLAG_PARTICLE_SYSTEMS, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_PARTICLE_SYSTEMS));	
	flag_array[RenderOpts::FLAG_BLUR_TRANSPARENTS] = Flag("FLAG_BLUR_TRANSPARENTS", RenderOpts::FLAG_BLUR_TRANSPARENTS, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_BLUR_TRANSPARENTS));

	flag_array[RenderOpts::FLAG_SHARPEN_FILTER] = Flag("FLAG_SHARPEN_FILTER", RenderOpts::FLAG_SHARPEN_FILTER, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_SHARPEN_FILTER));
	flag_array[RenderOpts::FLAG_GAMMA_CORRECTION] = Flag("FLAG_GAMMA_CORRECTION", RenderOpts::FLAG_GAMMA_CORRECTION, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_GAMMA_CORRECTION));
	flag_array[RenderOpts::FLAG_GAMMA_CORRECTION_FAST] = Flag("FLAG_GAMMA_CORRECTION_FAST", RenderOpts::FLAG_GAMMA_CORRECTION_FAST, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_GAMMA_CORRECTION_FAST));

	flag_array[RenderOpts::FLAG_WIREFRAME] = Flag("FLAG_WIREFRAME", RenderOpts::FLAG_WIREFRAME, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_WIREFRAME));
	flag_array[RenderOpts::FLAG_WIREFRAME_SOLID] = Flag("FLAG_WIREFRAME_SOLID", RenderOpts::FLAG_WIREFRAME_SOLID, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_WIREFRAME_SOLID));

	flag_array[RenderOpts::FLAG_COLORIZE_SHADOW_CASTERS] = Flag("FLAG_COLORIZE_SHADOW_CASTERS", RenderOpts::FLAG_COLORIZE_SHADOW_CASTERS, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_COLORIZE_SHADOW_CASTERS));
	flag_array[RenderOpts::FLAG_COLORIZE_LOD_LEVELS] = Flag("FLAG_COLORIZE_LOD_LEVELS", RenderOpts::FLAG_COLORIZE_LOD_LEVELS, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_COLORIZE_LOD_LEVELS));
	flag_array[RenderOpts::FLAG_FORCE_SHADOW_CASTER_ALL] = Flag("FLAG_FORCE_SHADOW_CASTER_ALL", RenderOpts::FLAG_FORCE_SHADOW_CASTER_ALL, Scene5::RenderFlagEnabled(flags, RenderOpts::FLAG_FORCE_SHADOW_CASTER_ALL));

	flag_array[RenderOpts::FLAG_DOF_HALF_RES] = Flag( "FLAG_HALF_RES_DOF", RenderOpts::FLAG_DOF_HALF_RES, Scene5::RenderFlagEnabled( flags, RenderOpts::FLAG_DOF_HALF_RES ) );
	flag_array[RenderOpts::FLAG_BLOOM_MOBILE] = Flag( "FLAG_MOBILE_BLOOM", RenderOpts::FLAG_BLOOM_MOBILE, Scene5::RenderFlagEnabled( flags, RenderOpts::FLAG_BLOOM_MOBILE ) );

	for (int i = 0; i < RenderOpts::NUMBER_OF_FLAGS; i++)
	{
		s.Serialize(flag_array[i].m_name.c_str(), flag_array[i].m_value);
	}

	for (int i = 0; i < RenderOpts::NUMBER_OF_FLAGS; i++)
	{
		Scene5::SetRenderFlag(flags, (RenderOpts::Flag) flag_array[i].m_flag, flag_array[i].m_value);
	}
}


KCL::uint32 Scene5Tools::CaptureEnvironmentCube(KCL::uint32 command_buffer, const char *name, Shapes *shapes, KCL::uint32 size, std::vector<KCL::Mesh*> &meshes)
{
	INFO("Generate prefiltered environment map...");
	uint32_t cubemap_size[2] = { size, size };

	KCL::uint32 texture = CaptureCubemap(command_buffer, KCL::Vector3D(0, 0, 0), "SKY_FORMAT_RGBE", "ENCODE_RGBE8888", meshes, shapes->m_cube_vbid, shapes->m_cube_ibid, cubemap_size);

	//rgb9e5_texture = ConvertCubemapToRGB9E5(m_prefiltered_cubemap_texture, PREFILTERED_CUBEMAP_SIZE, PREFILTERED_CUBEMAP_SIZE, KCL::texture_levels(PREFILTERED_CUBEMAP_SIZE, PREFILTERED_CUBEMAP_SIZE));

	SaveCubemapRGB9E5("ibl.pvr", texture, size, size, KCL::texture_levels(size, size));

	//LoadCubemapRGB9E5("ibl.pvr", texture);

	return texture;
}


KCL::uint32 Scene5Tools::CreateDebugMipmapTexture()
{
	NGL_texture_descriptor texture_layout;
	std::vector<std::vector<uint8_t> > datas;

	texture_layout.m_name = "debug_mip";
	texture_layout.m_size[0] = 64;
	texture_layout.m_size[1] = 64;
	texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
	texture_layout.m_num_levels = 6;
	texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
	texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
	texture_layout.m_type = NGL_TEXTURE_2D;

	//levels
	for (uint32_t l = 0; l < texture_layout.m_num_levels; l++)
	{
		//faces
		for (uint32_t f = 0; f < 1; f++)
		{
			uint32_t w = texture_layout.m_size[0] / (1 << l);
			uint32_t h = texture_layout.m_size[1] / (1 << l);
			std::vector<uint8_t> data;

			data.resize(w * h * 4);

			for (uint32_t y = 0; y < h; y++)
			{
				for (uint32_t x = 0; x < w; x++)
				{
					uint32_t k = x + y * w;
					if (l == 0)
					{
						data[k * 4 + 0] = 0;
						data[k * 4 + 1] = 0;
						data[k * 4 + 2] = 255;
						data[k * 4 + 3] = static_cast<int>(255 * 0.8);
					}
					else if (l == 1)
					{
						data[k * 4 + 0] = 0;
						data[k * 4 + 1] = static_cast<int>(255 * 0.5);
						data[k * 4 + 2] = 255;
						data[k * 4 + 3] = static_cast<int>(255 * 0.4);
					}
					else if (l == 2)
					{
						data[k * 4 + 0] = 255;
						data[k * 4 + 1] = 255;
						data[k * 4 + 2] = 255;
						data[k * 4 + 3] = 0;
					}
					else if (l == 3)
					{
						data[k * 4 + 0] = 255;
						data[k * 4 + 1] = static_cast<int>(255 * 0.7);
						data[k * 4 + 2] = 0;
						data[k * 4 + 3] = static_cast<int>(255 * 0.2);
					}
					else if (l == 4)
					{
						data[k * 4 + 0] = 255;
						data[k * 4 + 1] = static_cast<int>(255 * 0.3);
						data[k * 4 + 2] = 0;
						data[k * 4 + 3] = static_cast<int>(255 * 0.6);
					}
					else if (l == 5)
					{
						data[k * 4 + 0] = 255;
						data[k * 4 + 1] = 0;
						data[k * 4 + 2] = 0;
						data[k * 4 + 3] = static_cast<int>(255 * 0.8);
					}
				}
			}

			datas.push_back(data);
		}
	}

	KCL::uint32 texture = 0;
	nglGenTexture(texture, texture_layout, &datas);
	return texture;
}

static void Write(std::fstream &csv, const std::string &name, KCL::uint64 size)
{
	const char SEP = ',';

	csv << name << SEP << size << SEP << size / 1024 << SEP << size / 1024 / 1024 << std::endl;
}


void Scene5Tools::SaveMemoryStatistics(const NGLStatistic &stats)
{
	const NGLMemoryStatistics &mem = stats.m_memory_statistics;
	KCL::uint64 sum_textures = 0;
	KCL::uint64 sum_images = 0;
	KCL::uint64 sum_render_targets = 0;
	KCL::uint64 sum_vertex_buffers = 0;
	KCL::uint64 sum_index_buffers = 0;
	KCL::uint64 sum_storage_buffers = 0;

	for (size_t i = 0; i < mem.m_render_targets.size(); i++)
	{
		sum_render_targets += mem.m_render_targets[i].m_size;
	}
	for (size_t i = 0; i < mem.m_images.size(); i++)
	{
		sum_images += mem.m_images[i].m_size;
	}
	for (size_t i = 0; i < mem.m_textures.size(); i++)
	{
		sum_textures += mem.m_textures[i].m_size;
	}

	for (size_t i = 0; i < mem.m_vertex_buffers.size(); i++)
	{
		sum_vertex_buffers += mem.m_vertex_buffers[i].m_size;
	}
	for (size_t i = 0; i < mem.m_index_buffers.size(); i++)
	{
		sum_index_buffers += mem.m_index_buffers[i].m_size;
	}
	for (size_t i = 0; i < mem.m_storage_buffers.size(); i++)
	{
		sum_storage_buffers += mem.m_storage_buffers[i].m_size;
	}

	const char SEP = ',';

	std::fstream csv("memory.csv", std::ios::out);

	csv << "ITEM" << SEP << "Bytes" << SEP << "KBytes" << SEP << "MBytes" << std::endl;

	Write(csv, "Render targets:", sum_render_targets);
	Write(csv, "Images:", sum_images);
	Write(csv, "Textures:", sum_textures);

	Write(csv, "Vertex buffers:", sum_vertex_buffers);
	Write(csv, "Index buffers:", sum_index_buffers);
	Write(csv, "Storage buffers:", sum_storage_buffers);

	Write(csv, "SUM:", sum_render_targets + sum_images + sum_textures + sum_vertex_buffers + sum_index_buffers + sum_storage_buffers);

	csv << std::endl;

	for (size_t i = 0; i < mem.m_render_targets.size(); i++)
	{
		Write(csv, mem.m_render_targets[i].m_name, mem.m_render_targets[i].m_size);
	}

	csv << std::endl;

	for (size_t i = 0; i < mem.m_images.size(); i++)
	{
		Write(csv, mem.m_images[i].m_name, mem.m_images[i].m_size);
	}

	csv << std::endl;

	for (size_t i = 0; i < mem.m_textures.size(); i++)
	{
		Write(csv, mem.m_textures[i].m_name, mem.m_textures[i].m_size);
	}

	csv << std::endl;

	csv.close();
}


void Scene5Tools::DumpPipelineStatistics(const NGLStatistic &stats)
{
	if (nglGetApi() == NGL_VULKAN)
	{
		INFO("NGL API: Vulkan\n");
	}
	else
	{
		INFO("NGL API: GL\n");
	}

	uint64_t results[NGL_STATISTIC_COUNT];
	memset(results, 0, sizeof(results));

	size_t draw_calls = 0;

	for (size_t i = 0; i < stats.jobs.size(); i++)
	{
		for (size_t j = 0; j < stats.jobs[i].m_sub_pass.size(); j++)
		{
			if (!stats.jobs[i].m_is_active)
			{
				continue;
			}
			for (size_t k = 0; k < stats.jobs[i].m_sub_pass[j].m_draw_calls.size(); k++)
			{
				for (size_t m = 0; m < NGL_STATISTIC_COUNT; m++)
				{
					results[m] += stats.jobs[i].m_sub_pass[j].m_draw_calls[k].m_query_results[m];
				}
			}

			draw_calls += stats.jobs[i].m_sub_pass[j].m_draw_calls.size();
		}
	}

	const char* result_names[NGL_STATISTIC_COUNT] =
	{
		"NGL_STATISTIC_PRIMITIVES_SUBMITED",
		"NGL_STATISTIC_VERTICES_SUBMITED",
		"NGL_STATISTIC_VS_INVOCATIONS",
		"NGL_STATISTIC_TCS_PATCHES",
		"NGL_STATISTIC_TES_INVOCATIONS",
		"NGL_STATISTIC_GS_INVOCATIONS",
		"NGL_STATISTIC_GS_PRIMITIVES_EMITTED",
		"NGL_STATISTIC_FS_INVOCATIONS",
		"NGL_STATISTIC_CLIPPING_INPUT_PRIMITIVES",
		"NGL_STATISTIC_CLIPPING_OUTPUT_PRIMITIVES",
		"NGL_STATISTIC_PRIMITIVES_GENERATED",
		"NGL_STATISTIC_SAMPLES_PASSED",
		"NGL_STATISTIC_TIME_ELAPSED",
	};

	std::stringstream sstream;

	sstream << "draw calls: " << draw_calls << '\n';

	for (size_t m = 0; m < NGL_STATISTIC_COUNT; m++)
	{
		sstream << result_names[m] << ' ' << int(results[m]) << '\n';
	}

	INFO("%s", sstream.str().c_str());

	FILE *file = nullptr;
	if (nglGetApi() == NGL_VULKAN)
	{
		file = fopen("vulkan.txt", "wt");
	}
	else
	{
		file = fopen("gl.txt", "wt");
	}

	fprintf(file, "%s", sstream.str().c_str());
	fclose(file);
}
