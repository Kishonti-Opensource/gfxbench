/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_cubemap.h"
#include "gfxb_shader.h"
#include "gfxb_material.h"
#include "gfxb_mesh_shape.h"
#include "gfxb_texture.h"
#include "gfxb_barrier.h"
#include <kcl_math3d.h>
#include <etc1.h>
#include <sstream>

using namespace GFXB;


Cubemap::Cubemap()
{
	m_texture = 0;
	m_width = 0;
	m_height = 0;
	m_levels = 0;
}


Cubemap::~Cubemap()
{
}


Cubemap *Cubemap::Create(KCL::uint32 width, KCL::uint32 height, NGL_format format)
{
	NGL_texture_descriptor texture_layout;

	texture_layout.m_type = NGL_TEXTURE_CUBE;
	texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
	texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
	texture_layout.m_format = format;
	texture_layout.m_size[0] = width;
	texture_layout.m_size[1] = height;
	texture_layout.m_is_renderable = true;

	Cubemap *cubemap = new Cubemap();
	cubemap->m_texture = 0;
	nglGenTexture(cubemap->m_texture, texture_layout, nullptr);
	cubemap->m_format = format;
	cubemap->m_width = width;
	cubemap->m_height = height;
	cubemap->m_levels = KCL::texture_levels(int(width), int(height));

	Transitions::Get().Register(cubemap->m_texture, texture_layout);

	return cubemap;
}


Cubemap *Cubemap::Load(const char *filename, bool rgbe)
{
	KCL::Image img;
	if (img.loadCube(filename) == false)
	{
		INFO("Cubemap: Can not open: %s", filename);
		return nullptr;
	}


	NGL_texture_descriptor texture_layout;
	texture_layout.m_type = NGL_TEXTURE_CUBE;
	texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
	texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
	texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
	texture_layout.m_is_renderable = false;
	texture_layout.m_size[0] = img.getWidth();
	texture_layout.m_size[1] = img.getHeight();
	texture_layout.m_num_levels = img.getMipCount();

	std::vector<std::vector<uint8_t> > data(6);
	for (unsigned int i = 0; i < 6; i++)
	{
		uint8_t* from = (uint8_t*)img.getData(i);
		data[i] = std::vector < uint8_t >(from, from + img.getSlicePitch());
	}

	KCL::uint32 texture = 0;
	nglGenTexture(texture, texture_layout, &data);

	Cubemap *cubemap = new Cubemap();
	cubemap->m_texture = texture;
	cubemap->m_format = NGL_R8_G8_B8_A8_UNORM;
	cubemap->m_is_rgbe = rgbe;
	cubemap->m_width = img.getWidth();
	cubemap->m_height = img.getHeight();
	cubemap->m_levels = KCL::texture_levels(int(img.getWidth()), int(img.getHeight()));
	return cubemap;
}


KCL::KCL_Status Cubemap::SaveTGA(const char *filename)
{
	/*
	for (size_t dir = 0; dir < 6; dir++)
	{
		for (size_t level = 0; level < m_levels; level++)
		{
			uint32_t w, h;
			std::vector<uint8_t> data;

			m_gpu_api->GetTextureContent(m_texture, dir, level, m_format, w, h, data);

			KCL::Image img(w, h, KCL::Image_RGB9E5);
			img.setData(&data[0]);

			img.decodeRGB9E5toRGB888();

			KCL::Image::saveTga((KCL::File::GetScenePath() + filename + "_" + std::to_string(dir) + "_" + std::to_string(level) + ".tga").c_str(), res, res, img.getData(), KCL::Image_RGBA8888, false);
		}
	}
	*/
	return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
}


KCL::uint32 Cubemap::GetTexture() const
{
	return m_texture;
}


KCL::uint32 *Cubemap::GetTexturePtr()
{
	return &m_texture;
}


KCL::uint32 Cubemap::GetLevels() const
{
	return m_levels;
}


KCL::uint32 Cubemap::GetWidth() const
{
	return m_width;
}


KCL::uint32 Cubemap::GetHeight() const
{
	return m_height;
}


bool Cubemap::IsRGBE() const
{
	return m_is_rgbe && m_format == NGL_R8_G8_B8_A8_UNORM;
}


KCL::KCL_Status SaveTGA(const char *filename, uint32_t envmap, uint32_t levels);


uint32_t GFXB::CaptureCubemap(uint32_t command_buffer, const KCL::Vector3D &pos, const std::string &sky_format, const std::string &encode_format, std::vector<KCL::Mesh*> &sky, uint32_t cube_vbid, uint32_t cube_ibid, uint32_t size[2])
{
	const KCL::Vector3D refs[6] =
	{
		KCL::Vector3D(1, 0, 0),
		KCL::Vector3D(-1, 0, 0),
		KCL::Vector3D(0, 1, 0),
		KCL::Vector3D(0, -1, 0),
		KCL::Vector3D(0, 0, 1),
		KCL::Vector3D(0, 0, -1)
	};
	const KCL::Vector3D ups[6] =
	{
		KCL::Vector3D(0, -1, 0),
		KCL::Vector3D(0, -1, 0),
		KCL::Vector3D(0, 0, 1),
		KCL::Vector3D(0, 0, -1),
		KCL::Vector3D(0, -1, 0),
		KCL::Vector3D(0, -1, 0)
	};

	uint32_t color_texture0;
	uint32_t color_texture1;
	uint32_t levels = KCL::texture_levels(size[0], size[1]);
	KCL::Camera2 camera;
	const void *p[UNIFORM_MAX];

	camera.Perspective(90.0f, size[0], size[1], 0.01f, 10000.0f);

	{
		uint32_t depth_texture;
		uint32_t jobs[6];
		uint32_t sky_shader;

		{
			ShaderDescriptor sd;

			sd.SetVSFile("sky.vert");
			sd.SetFSFile("sky.frag");
			if (sky_format.length())
			{
				sd.AddDefine(sky_format.c_str());
			}
			if (encode_format.length())
			{
				//sd.AddDefine(encode_format.c_str());
			}
			sky_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
		}
		{
			NGL_texture_descriptor texture_layout;

			texture_layout.m_type = NGL_TEXTURE_CUBE;
			texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
			texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
			texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
			if (nglGetInteger(NGL_FLOATING_POINT_RENDERTARGET))
			{
				texture_layout.m_format = NGL_R16_G16_B16_A16_FLOAT;
			}
			texture_layout.m_size[0] = size[0];
			texture_layout.m_size[1] = size[1];
			texture_layout.m_is_renderable = true;
			texture_layout.m_num_levels = KCL::texture_levels(size[0], size[1]);

			color_texture0 = 0;
			nglGenTexture(color_texture0, texture_layout, 0);
			Transitions::Get().Register(color_texture0, texture_layout);
		}
		{
			NGL_texture_descriptor texture_layout;

			texture_layout.m_type = NGL_TEXTURE_CUBE;
			texture_layout.m_filter = NGL_LINEAR;
			texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
			texture_layout.m_format = NGL_D24_UNORM;
			texture_layout.m_size[0] = size[0];
			texture_layout.m_size[1] = size[1];
			texture_layout.m_is_renderable = true;
			texture_layout.m_clear_value[0] = 1.0f;

			depth_texture = 0;
			nglGenTexture(depth_texture, texture_layout, 0);
			Transitions::Get().Register(depth_texture, texture_layout);
		}

		for (uint32_t side = 0; side < 6; side++)
		{
			NGL_job_descriptor rrd;
			{
				NGL_attachment_descriptor ad;
				ad.m_attachment.m_idx = color_texture0;
				ad.m_attachment.m_face = side;
				ad.m_attachment.m_level = 0;
				ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
				ad.m_attachment_store_op = NGL_STORE_OP_STORE;
				rrd.m_attachments.push_back(ad);
			}
			{
				NGL_attachment_descriptor ad;
				ad.m_attachment.m_idx = depth_texture;
				ad.m_attachment.m_face = side;
				ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
				ad.m_attachment_store_op = NGL_STORE_OP_STORE;
				rrd.m_attachments.push_back(ad);
			}
			{
				std::stringstream sstream;
				sstream << "cubemap_capture " << side;

				NGL_subpass sp;
				sp.m_name = sstream.str();
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
				sp.m_usages.push_back(NGL_DEPTH_ATTACHMENT);
				rrd.m_subpasses.push_back(sp);
			}

			rrd.m_load_shader_callback = LoadShader;
			jobs[side] = nglGenJob(rrd);

			int32_t viewport[4] =
			{
				0, 0, (int32_t)size[0], (int32_t)size[1]
			};

			nglDepthState(jobs[side], NGL_DEPTH_LESS, false);
			nglDepthState(jobs[side], NGL_DEPTH_TO_FAR, false);
			nglViewportScissor(jobs[side], viewport, viewport);
		}
		for (int dir = 0; dir < 6; dir++)
		{
			camera.LookAt(pos, pos + refs[dir], ups[dir]);
			camera.Update();

			KCL::Matrix4x4 mvp;
			p[UNIFORM_MVP] = &mvp.v;

			nglBegin(jobs[dir], command_buffer);

			for (size_t i = 0; i < sky.size(); i++)
			{
				KCL::Mesh* mesh = sky[i];
				Mesh3 *gfxb5_mesh = (Mesh3*)mesh->m_mesh;

				mvp = mesh->m_world_pom * camera.GetViewProjectionOrigo();
				p[UNIFORM_COLOR_TEX] = &((Material*)mesh->m_material)->GetTexture(Material::COLOR)->m_id;

				nglDrawTwoSided(jobs[dir], sky_shader, gfxb5_mesh->m_vbid, gfxb5_mesh->m_ibid, p);
			}

			nglEnd(jobs[dir]);
		}
	}

	if (1)
	{
		uint32_t jobs[6][10];
		uint32_t filter_shader;

		{
			ShaderDescriptor sd;

			sd.SetVSFile("prefilter_envmap.vert");
			sd.SetFSFile("prefilter_envmap.frag");
			//sd.AddDefine("ENCODE_RGBE8888");

			filter_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
		}
		{
			NGL_texture_descriptor texture_layout;

			texture_layout.m_type = NGL_TEXTURE_CUBE;
			texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
			texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
			texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
			if (nglGetInteger(NGL_FLOATING_POINT_RENDERTARGET))
			{
				texture_layout.m_format = NGL_R16_G16_B16_A16_FLOAT;
			}
			texture_layout.m_size[0] = size[0];
			texture_layout.m_size[1] = size[1];
			texture_layout.m_is_renderable = true;
			texture_layout.m_num_levels = KCL::texture_levels(size[0], size[1]);

			color_texture1 = 0;
			nglGenTexture(color_texture1, texture_layout, 0);
			Transitions::Get().Register(color_texture1, texture_layout);
		}
		for (uint32_t level = 0; level < levels; level++)
		{
			for (uint32_t side = 0; side < 6; side++)
			{
				NGL_job_descriptor rrd;
				{
					NGL_attachment_descriptor ad;
					ad.m_attachment.m_idx = color_texture1;
					ad.m_attachment.m_face = side;
					ad.m_attachment.m_level = (KCL::uint32)level;
					ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
					ad.m_attachment_store_op = NGL_STORE_OP_STORE;
					rrd.m_attachments.push_back(ad);
				}
				{
					std::stringstream sstream;
					sstream << "cubemap_prefilter " << side << ' ' << level;

					NGL_subpass sp;
					sp.m_name = sstream.str();
					sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
					rrd.m_subpasses.push_back(sp);

				}

				rrd.m_load_shader_callback = LoadShader;
				jobs[side][level] = nglGenJob(rrd);
			}
		}


		KCL::Matrix4x4 mvp;
		float roughness = 0.0f;

		p[UNIFORM_MVP] = &mvp.v;
		p[UNIFORM_ROUGHNESS] = &roughness;
		p[UNIFORM_ENVMAP0] = &color_texture0;

		for (uint32_t level = 0; level < levels; level++)
		{
			for (int dir = 0; dir < 6; dir++)
			{
				KCL::int32 width = KCL::int32(size[0] / (1 << level));
				KCL::int32 height = KCL::int32(size[1] / (1 << level));
				KCL::int32 box[4] =
				{
					0, 0, width, height
				};
				camera.LookAt(KCL::Vector3D(0.0f, 0.0f, 0.0f), refs[dir], ups[dir]);
				camera.Update();

				mvp = camera.GetViewProjection();

				// Linear roughness distribution
				roughness = (float)level / (float)levels;

				// Logaritmic distribution
				// TODO: Move these constants
				#define REFLECTION_CAPTURE_ROUGHEST_MIP 1.0f
				#define REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE 1.2f

				const float HardcodedNumCaptureArrayMips = (float)levels;
				float LevelFrom1x1 = HardcodedNumCaptureArrayMips - 1.0f - (float)level;
				roughness = powf(2.0f, (REFLECTION_CAPTURE_ROUGHEST_MIP - LevelFrom1x1) / REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE);

				if (roughness > 1.0f)
				{
					roughness = 1.0f;
				}

				nglViewportScissor(jobs[dir][level], box, box);

				nglBegin(jobs[dir][level], command_buffer);
				nglDrawTwoSided(jobs[dir][level], filter_shader, cube_vbid, cube_ibid, p);
				nglEnd(jobs[dir][level]);

				nglCustomAction(jobs[dir][level], 101);
			}
		}

		if (0)
		{
			SaveTGA("cica", color_texture1, levels);
		}

		return color_texture1;
	}
	else
	{
		nglCustomAction(color_texture0, 102);

		return color_texture0;
	}
}


void GFXB::ConvertCubemapToRGB9E5(uint32_t input_texture, uint32_t width, uint32_t height, uint32_t levels, std::vector<std::vector<uint8_t>> &pixel_data)
{
	for (uint32_t level = 0; level < levels; level++)
	{
		uint32_t mipmap_width = width / (1 << level);
		uint32_t mipmap_height = height / (1 << level);
		mipmap_width = mipmap_width ? mipmap_width : 1;
		mipmap_height = mipmap_height ? mipmap_height : 1;

		for (uint32_t face = 0; face < 6; face++)
		{
			// Read back the face level
			uint32_t w, h;
			std::vector<uint8_t> face_data;
			NGL_texture_subresource subres(input_texture, level, 0, face);
			NGL_resource_state state = Transitions::Get().GetTextureState(subres);
			nglGetTextureContent(input_texture, level, 0, face, NGL_R32_G32_B32_A32_FLOAT, state, w, h, face_data);

			// Convert to RGB9_E5
			std::vector<uint8_t> rgbe5_face_data;
			float *float_ptr = (float*)face_data.data();
			for (uint32_t i = 0; i < mipmap_width * mipmap_height; i++)
			{
				float rgb[3];
				rgb[0] = float_ptr[i * 4 + 0];
				rgb[1] = float_ptr[i * 4 + 1];
				rgb[2] = float_ptr[i * 4 + 2];
				KCL::uint32 raw_int = KCL::Image::Float3_To_RGB9E5(rgb);

				uint8_t *byte_ptr = (uint8_t*)&raw_int;

				rgbe5_face_data.push_back(byte_ptr[0]);
				rgbe5_face_data.push_back(byte_ptr[1]);
				rgbe5_face_data.push_back(byte_ptr[2]);
				rgbe5_face_data.push_back(byte_ptr[3]);
			}

			pixel_data.push_back(rgbe5_face_data);
		}
	}
}


uint32_t GFXB::ConvertCubemapToRGB9E5(uint32_t input_texture, uint32_t width, uint32_t height, uint32_t levels)
{
	std::vector<std::vector<uint8_t>> pixel_data;

	ConvertCubemapToRGB9E5(input_texture, width, height, levels, pixel_data);

	// Create the new texture
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_type = NGL_TEXTURE_CUBE;
		texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_format = NGL_R9_G9_B9_E5_SHAREDEXP;
		texture_layout.m_size[0] = width;
		texture_layout.m_size[1] = height;
		texture_layout.m_is_renderable = false;
		texture_layout.m_num_levels = levels;

		KCL::uint32 rgb_e5_texture = 0;
		nglGenTexture(rgb_e5_texture, texture_layout, &pixel_data);
		return rgb_e5_texture;
	}
}


KCL::KCL_Status GFXB::SaveCubemapRGB9E5(const char* filename, uint32_t input_texture, uint32_t width, uint32_t height, uint32_t levels)
{
	std::vector<std::vector<uint8_t>> pixel_data;

	ConvertCubemapToRGB9E5(input_texture, width, height, levels, pixel_data);

	PVRHeaderV3 header;
	header.m_pixel_format = PVR3_PIXEL_FORMAT_R9G9B9E5;
	header.m_width = width;
	header.m_height = height;
	header.m_num_mipmaps = levels;
	header.m_num_faces = 6;

	KCL::File file(filename, KCL::Write, KCL::RWDir, true);
	if (!file.Opened())
	{
		INFO("Can not save cubemap: %s", filename);
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	file.Write(&header, PVRTEX3_HEADERSIZE, 1);
	for (size_t i = 0; i < pixel_data.size(); i++)
	{
		file.Write(pixel_data[i].data(), 1, pixel_data[i].size());
	}
	file.Close();

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status GFXB::LoadCubemapRGB9E5(const char* filename, uint32_t &texture, uint32_t &size)
{
	KCL::AssetFile file(filename, true);
	if (!file.Opened())
	{
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	PVRHeaderV3 header;
	file.Read(&header, PVRTEX3_HEADERSIZE, 1);

	if (header.m_width != header.m_height)
	{
		INFO("Invalid cubemap file: %s", filename);
		return KCL::KCL_TESTERROR_UNKNOWNERROR;
	}

	uint32_t width = header.m_width;
	uint32_t height = header.m_height;
	uint32_t levels = header.m_num_mipmaps;

	std::vector<std::vector<uint8_t>> pixel_data;
	for (uint32_t level = 0; level < levels; level++)
	{
		uint32_t mipmap_width = width / (1 << level);
		uint32_t mipmap_height = height / (1 << level);
		mipmap_width = mipmap_width ? mipmap_width : 1;
		mipmap_height = mipmap_height ? mipmap_height : 1;


		std::vector<uint8_t> face_data(mipmap_width * mipmap_height * 4);

		for (uint32_t face = 0; face < 6; face++)
		{
			file.Read(face_data.data(), 1, face_data.size());
			pixel_data.push_back(face_data);
		}
	}

	// Create the new texture
	{
		NGL_texture_descriptor texture_layout;
		texture_layout.m_name = filename;
		texture_layout.m_type = NGL_TEXTURE_CUBE;
		texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_format = NGL_R9_G9_B9_E5_SHAREDEXP;
		texture_layout.m_size[0] = width;
		texture_layout.m_size[1] = height;
		texture_layout.m_is_renderable = false;
		texture_layout.m_num_levels = levels;

		texture = 0;
		nglGenTexture(texture, texture_layout, &pixel_data);
	}

	size = width;

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status SaveTGA(const char *filename, uint32_t envmap, uint32_t levels)
{
	for (uint32_t level = 0; level < levels; level++)
	{
		for (uint32_t dir = 0; dir < 6; dir++)
		{
			uint32_t w, h;
			std::vector<uint8_t> data;

			NGL_texture_subresource subres(envmap, level, 0, dir);
			NGL_resource_state state = Transitions::Get().GetTextureState(subres);
			nglGetTextureContent(envmap, level, 0, dir, NGL_R8_G8_B8_A8_UNORM, state, w, h, data);

			std::stringstream sstream;
			sstream << KCL::File::GetScenePath();
			sstream << filename;
			sstream << '_' << level << '_' << dir << ".tga";
			KCL::Image::saveTga(sstream.str().c_str(), w, h, &data[0], KCL::Image_RGBA8888, false);

#if 0
			//KCL::Image img(w, h, KCL::Image_RGB9E5);
			KCL::Image img(w, h, KCL::Image_RGBA8888);
			img.setData(&data[0]);

			//img.decodeRGB9E5toRGB888();

			KCL::Image::saveTga((KCL::File::GetScenePath() + filename + "_" + std::to_string(dir) + "_" + std::to_string(level) + ".tga").c_str(), w, h, img.getData(), KCL::Image_RGBA8888, false);
#endif
		}
	}
	return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
}


uint32_t GFXB::CreateIntegrateBRDF_LUT(uint32_t command_buffer, uint32_t quad_vbid, uint32_t quad_ibid, uint32_t size)
{
	uint32_t shader = 0;
	{
		ShaderDescriptor sd;

		sd.SetVSFile("integrate_brdf.vert");
		sd.SetFSFile("integrate_brdf.frag");

		shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
	}

	uint32_t texture = 0;
	{
		NGL_texture_descriptor texture_layout;
		texture_layout.m_name = "brdf_lut";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_format = NGL_R10_G10_B10_A2_UNORM;
		texture_layout.m_size[0] = size;
		texture_layout.m_size[1] = size;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(0.0f);
		texture_layout.m_clear_value[1] = 1.0f;

		nglGenTexture(texture, texture_layout, 0);
		Transitions::Get().Register(texture, texture_layout);
	}

	uint32_t job = 0;
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "CreateIntegrateBRDF_LUT";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		job = nglGenJob(rrd);

		int32_t viewport[4] =
		{
			0, 0, (int32_t)size, (int32_t)size
		};

		nglViewportScissor(job, viewport, viewport);
		nglDepthState(job, NGL_DEPTH_DISABLED, false);
	}

	Transitions::Get().TextureBarrier(texture, NGL_COLOR_ATTACHMENT).Execute(command_buffer);

	const void *p[UNIFORM_MAX];
	nglBegin(job, command_buffer);
	nglDrawTwoSided(job, shader, quad_vbid, quad_ibid, p);
	nglEnd(job);

	Transitions::Get().TextureBarrier(texture, NGL_SHADER_RESOURCE).Execute(command_buffer);

	if (0)
	{
		uint32_t w, h;
		std::vector<uint8_t> data;

		nglGetTextureContent(texture, 0, 0, 0, NGL_R8_G8_B8_A8_UNORM, Transitions::Get().GetTextureState(texture), w, h, data);

		KCL::Image::saveTga((KCL::File::GetScenePath() + "mica" + ".tga").c_str(), w, h, &data[0], KCL::Image_RGBA8888, false);
	}

	return texture;
}
