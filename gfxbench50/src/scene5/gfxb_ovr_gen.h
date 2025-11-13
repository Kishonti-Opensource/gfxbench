/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if 0

#ifndef GFXB_OVR_GEN_H
#define GFXB_OVR_GEN_H

#include "gfxb_scene5.h"
#include "common/gfxb_cascaded_shadow_map.h"

using namespace GFXB;

//#define OVR_GEN

#define OVR_WRITE_TO_FILE
//#define OVR_DISPLAY_RESULT

namespace ovr_gen
{
	KCL::uint32 m_cubemap_texture = 0;

	int ovr_full_size = 4096;
	int ovr_final_size = int(ovr_full_size * 0.5f);

	int OVR_CAM_ID = 0;
	const int OVR_MAX_CAMS = 12;

	enum OVR_CAM
	{
		OVR_CAM_POS_X_LEFT = 0,
		OVR_CAM_POS_X_RIGHT,
		OVR_CAM_NEG_X_LEFT,
		OVR_CAM_NEG_X_RIGHT,
		OVR_CAM_POS_Y_LEFT,
		OVR_CAM_POS_Y_RIGHT,
		OVR_CAM_NEG_Y_LEFT,
		OVR_CAM_NEG_Y_RIGHT,
		OVR_CAM_POS_Z_LEFT,
		OVR_CAM_POS_Z_RIGHT,
		OVR_CAM_NEG_Z_LEFT,
		OVR_CAM_NEG_Z_RIGHT,
		OVR_CAM_INVALID
	};

	const char* CAM_ID_STR[] =
	{
		"OVR_CAM_POS_X_LEFT",
		"OVR_CAM_POS_X_RIGHT",
		"OVR_CAM_NEG_X_LEFT",
		"OVR_CAM_NEG_X_RIGHT",
		"OVR_CAM_POS_Y_LEFT",
		"OVR_CAM_POS_Y_RIGHT",
		"OVR_CAM_NEG_Y_LEFT",
		"OVR_CAM_NEG_Y_RIGHT",
		"OVR_CAM_POS_Z_LEFT",
		"OVR_CAM_POS_Z_RIGHT",
		"OVR_CAM_NEG_Z_LEFT",
		"OVR_CAM_NEG_Z_RIGHT",
		"OVR_CAM_INVALID"
	};

	KCL::Camera2 cam;

	void constructor(Scene5* p)
	{
#ifdef OVR_GEN
		p->SetRenderFlag(RenderOpts::FLAG_CSM, true);
#endif
	}

	void init(Scene5* p)
	{
#ifdef OVR_GEN
		//HAXXX
		p->SetViewportWidth( ovr_full_size );
		p->SetViewportHeight(ovr_full_size);
#endif
	}

	void init_csm(Scene5* p, Scene5CSMMeshFilter **m_csm_mesh_filter, CascadedShadowMap **m_cascaded_shadow)
	{
#ifdef OVR_GEN
		delete *m_csm_mesh_filter;
		delete *m_cascaded_shadow;
		*m_csm_mesh_filter = new Scene5CSMMeshFilter();
		*m_cascaded_shadow = new CascadedShadowMap(p, 1024, NGL_D32_UNORM, *m_csm_mesh_filter);
		(*m_cascaded_shadow)->SetShadowNearFar(0.1f, 50.0f).SetShadowNegativeRange(10.0f).SetShadowPositiveRange(1.0f);
		(*m_cascaded_shadow)->SetFitMode(CascadedShadowMap::FIT_OBB).SetSelectionMode(CascadedShadowMap::SELECTION_MAP_BASED);
		(*m_cascaded_shadow)->AddCascade(0.0f).AddCascade(5.0f).AddCascade(10.0f).AddCascade(20.0f);
#endif
	}

	void animate(KCL::Camera2 *m_fps_camera, KCL::Camera2 **m_active_camera)
	{
#ifdef OVR_GEN
#ifdef OVR_DISPLAY_RESULT
		OVR_CAM_ID = OVR_CAM_POS_Z_LEFT;
#endif

		//const float eye_sep = 0.04f / 10.0f; //keves
		//const float eye_sep = 0.04f / 5.0f;
		const float eye_sep = 0.0640f * 0.5f; //FROM OCCULUS SDK

		const float goal_position = 10.0f;

		const float to_rads = 0.0174533f;
		const float top_eye_rot = 1.0f * 0.5f * to_rads; //radians
														 //const float bot_eye_rot = 3.0f * 0.5f * to_rads; //radians

		cam = *m_fps_camera;

		//cam.LookAtOmni(KCL::Vector3D(51.6269608f, 1.09762466f, 0.660734951f), unsigned(OVR_CAM_ID*0.5f));
		auto eye = cam.GetEye();
		auto goal = cam.GetCenter();
		auto view = (goal - eye).normalize();
		//eye = KCL::Vector3D(51.6269608f, 1.09762466f, 0.660734951f);
		eye = KCL::Vector3D(50.0f, 1.8f, 0.0f);
		goal = eye + view;
		//auto up = -cam.GetUp();
		auto up = cam.GetUp();
		auto right = KCL::Vector3D::cross(up, view).normalize();
		//cam.Perspective(90.0f, 1, 1, 1.0f, 2500.0f);

		INFO("CAM ID: %i", OVR_CAM_ID);

		/*if (OVR_CAM_ID % 2 == 0)
		{
		//left
		eye += -1.0f * right * eye_sep;
		}
		else
		{
		//right
		eye += 1.0f * right * eye_sep;
		}*/

		switch (OVR_CAM_ID)
		{
		case OVR_CAM_POS_Z_LEFT:
		{//
			goal = eye + view * goal_position;
			eye += -1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_POS_Z_RIGHT:
		{//
			goal = eye + view * goal_position;
			eye += 1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_NEG_Z_LEFT:
		{//
			goal = eye - view * goal_position;
			eye += 1.0f * right * eye_sep;
			//eye += -1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_NEG_Z_RIGHT:
		{//
			goal = eye - view * goal_position;
			eye += -1.0f * right * eye_sep;
			//eye += 1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_NEG_X_LEFT:
		{//
			goal = eye + right * goal_position;
			eye += -1.0f * view * eye_sep;
			//eye += -1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_NEG_X_RIGHT:
		{//
			goal = eye + right * goal_position;
			eye += 1.0f * view * eye_sep;
			//eye += 1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_POS_X_LEFT:
		{//
			goal = eye - right * goal_position;
			eye += 1.0f * view * eye_sep;
			//eye += -1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_POS_X_RIGHT:
		{//
			goal = eye - right * goal_position;
			eye += -1.0f * view * eye_sep;
			//eye += 1.0f * right * eye_sep;
			break;
		}
		case OVR_CAM_POS_Y_LEFT:
		{
			goal = eye + up * goal_position;
			up = -view;
			//up += KCL::Vector3D(sinf(1.0f * top_eye_rot), 0.0f, 0.0f);
			//eye += -1.0f * right * eye_sep;
			up.normalize();
			break;
		}
		case OVR_CAM_POS_Y_RIGHT:
		{
			goal = eye + up * goal_position;
			up = -view;
			//up += KCL::Vector3D(sinf(-1.0f * top_eye_rot), 0.0f, 0.0f);
			//eye += 1.0f * right * eye_sep;
			up.normalize();
			break;
		}
		case OVR_CAM_NEG_Y_LEFT:
		{
			goal = eye - up * goal_position;
			up = view;
			up += KCL::Vector3D(sinf(1.0f * top_eye_rot), 0.0f, 0.0f);
			//eye += -1.0f * right * eye_sep;
			up.normalize();
			break;
		}
		case OVR_CAM_NEG_Y_RIGHT:
		{
			goal = eye - up * goal_position;
			up = view;
			up += KCL::Vector3D(sinf(-1.0f * top_eye_rot), 0.0f, 0.0f);
			//eye += 1.0f * right * eye_sep;
			up.normalize();
			break;
		}
		default:
		{
			throw new int(0XDEADBEEF); //REKT
			break;
		}
		}

		/*if (OVR_CAM_ID % 2 == 0)
		{
		//left
		eye += -1.0f * right * eye_sep;
		}
		else
		{
		//right
		eye += 1.0f * right * eye_sep;
		}*/

		cam.LookAt(eye, goal, up);
		cam.Perspective(90.0f, 1, 1, 1.0f, 1000.0f);
		cam.Update();

		*m_active_camera = &cam;
#endif
	}

	void write_to_file(Shapes *m_shapes)
	{
#ifdef OVR_GEN
#ifdef OVR_WRITE_TO_FILE
		GenMipmaps gm;
		gm.Init(m_shapes, m_cubemap_texture, ovr_full_size, ovr_full_size, 2);
		gm.GenerateMipmaps(0);

		std::vector<uint8_t> data;
		uint32_t w, h;
		nglGetTextureContent(m_cubemap_texture, 1, 0, 0, NGL_R16_G16_B16_A16_FLOAT, w, h, data);

		if (w && h && data.size() > 0)
		{
			// Convert to RGB9_E5
			std::vector<std::vector<uint8_t> > rgbe5_cubemap_data;
			rgbe5_cubemap_data.resize(1);
			float *float_ptr = (float*)data.data();

			for (uint32_t i = 0; i < w * h; ++i)
			{
				float rgb[3];
				rgb[0] = float_ptr[i * 4 + 0];
				rgb[1] = float_ptr[i * 4 + 1];
				rgb[2] = float_ptr[i * 4 + 2];
				KCL::uint32 raw_int = KCL::Image::Float3_To_RGB9E5(rgb);

				uint8_t *byte_ptr = (uint8_t*)&raw_int;

				rgbe5_cubemap_data[0].push_back(byte_ptr[0]);
				rgbe5_cubemap_data[0].push_back(byte_ptr[1]);
				rgbe5_cubemap_data[0].push_back(byte_ptr[2]);
				rgbe5_cubemap_data[0].push_back(byte_ptr[3]);
			}

			std::vector<std::vector<uint8_t> > flipped_rgbe5_cubemap_data;
			flipped_rgbe5_cubemap_data.resize(1);

			uint32_t* data = (uint32_t*)rgbe5_cubemap_data[0].data();

			for (int y = h - 1; y > -1; --y)
			{
				for (int x = 0; x < int(w); ++x)
				{
					uint32_t raw_data = data[y * w + x];

					uint8_t* byte_ptr = (uint8_t*)&raw_data;

					flipped_rgbe5_cubemap_data[0].push_back(byte_ptr[0]);
					flipped_rgbe5_cubemap_data[0].push_back(byte_ptr[1]);
					flipped_rgbe5_cubemap_data[0].push_back(byte_ptr[2]);
					flipped_rgbe5_cubemap_data[0].push_back(byte_ptr[3]);
				}
			}

			PVRHeaderV3 header;
			header.m_pixel_format = PVR3_PIXEL_FORMAT_R9G9B9E5;
			header.m_width = w;
			header.m_height = h;
			header.m_num_mipmaps = 1;
			header.m_num_faces = 1;

			KCL::File file((std::string() + /*KCL::File::GetScenePath() +*/ "/convert/" + CAM_ID_STR[OVR_CAM_ID] + ".pvr").c_str(), KCL::Write, KCL::RWDir, true);
			if (!file.Opened())
			{
				INFO("Can not save cubemap: %s", (std::string() + /*KCL::File::GetScenePath() +*/ "/convert/" + CAM_ID_STR[OVR_CAM_ID] + ".pvr").c_str());
			}
			else
			{
				INFO("file saved: %s", (std::string() + /*KCL::File::GetScenePath() +*/ "/convert/" + CAM_ID_STR[OVR_CAM_ID] + ".pvr").c_str());
			}

			file.Write(&header, PVRTEX3_HEADERSIZE, 1);
			file.Write(flipped_rgbe5_cubemap_data[0].data(), 1, flipped_rgbe5_cubemap_data[0].size());
			file.Close();


			//KCL::Image::saveTga((KCL::File::GetScenePath() + "/convert/" + CAM_ID_STR[OVR_CAM_ID] + ".tga").c_str(), w, h, &data[0], KCL::Image_RGBA8888, true);
		}

		INFO((std::string(CAM_ID_STR[OVR_CAM_ID]) + ".tga").c_str());

		OVR_CAM_ID++;

		if (OVR_CAM_ID == OVR_MAX_CAMS)
			exit(0);

		OVR_CAM_ID = OVR_CAM_ID % OVR_MAX_CAMS;
#endif
#endif
	}

	void gen_cubemap(NGL_attachment_descriptor* ad)
	{
#ifdef OVR_GEN
		NGL_texture_descriptor texture_layout;
		{
			texture_layout.m_name = "cubemap renderer target";
			texture_layout.m_type = NGL_TEXTURE_2D;
			texture_layout.m_filter = NGL_NEAREST;
			texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

			texture_layout.m_size[0] = 4096;
			texture_layout.m_size[1] = 4096;
			//texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
			texture_layout.m_format = NGL_R16_G16_B16_A16_FLOAT;
			texture_layout.m_is_renderable = true;
			texture_layout.SetAllClearValue(0.0f);
			texture_layout.m_num_levels = 2;

			m_cubemap_texture = 0;
			nglGenTexture(m_cubemap_texture, texture_layout, nullptr);
		}

#ifdef OVR_DISPLAY_RESULT
		ad->m_attachment = m_final_texture;// IsEditorMode() ? m_debug_texture : 0;
#else
		ad->m_attachment = m_cubemap_texture;
#endif
#endif
	}

	void set_sharpen_values(KCL::Vector4D &m_sharpen_filter)
	{
#ifdef OVR_GEN
		m_sharpen_filter.set(0.0f, 0.0f, 0.0f, 0.0f);
#endif
	}
}

#endif

#endif
