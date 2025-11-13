/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_scene_handler.h"

#include <kcl_math3d.h>
#include <kcl_os.h>
#include "ng/stringutil.h"
#include <tinyxml.h>

using namespace std;

namespace KCL
{
	class SceneXML : public TiXmlVisitor
	{
	public:
		SceneXML(SceneHandler* scene) : m_scene(scene), m_is_valid(-1)
		{
		}
	private:

		bool VisitEnter( const TiXmlElement& e, const TiXmlAttribute* a)
		{
			string v = e.Value();

			if( m_is_valid == -1)
			{
				if( v == "GLB_SCENE_SETTINGS")
				{
					m_is_valid = 1;
					return true;
				}
				else
				{
					return false;
				}
			}

			m_scene->Load(&e);
			return true;
		}

		KCL::SceneHandler* m_scene;
		int m_is_valid;
	};
}

KCL::Vector3D XML_to_Vec3( const TiXmlElement *element)
{
	KCL::Vector3D result;
	if(sscanf(element->GetText(), "%f%f%f", &result.x, &result.y, &result.z) != 3)
	{
		result.set(0, 0, 0);
	}
	return result;
}


void KCL::SceneHandler::Load( const TiXmlElement *element)
{
	string v = element->Value();

	if( element->GetText())
	{
		if( v == "background_color")
		{
			m_background_color = XML_to_Vec3( element);
		}
		else if( v == "light_dir")
		{
			m_light_dir = XML_to_Vec3( element);
            m_light_dir_orig = m_light_dir;
		}
		else if( v == "fog_density")
		{
			m_fog_density = ng::atof( element->GetText());
		}
		else if( v == "torches_enabled")
		{
			string b = element->GetText();
			m_torches_enabled = b == "true";
		}
		else if( v == "camera_near")
		{
			m_camera_near = ng::atof(element->GetText());
		}
		else if( v == "camera_position")
		{
			m_camera_position = XML_to_Vec3( element);
		}
		else if( v == "light_color")
		{
			m_light_color = XML_to_Vec3( element);
		}
		else if( v == "animation_multiplier")
		{
			m_animation_multiplier = ng::atof(element->GetText());
		}
		else if( v == "shadow_method")
		{
			string b = element->GetText();

			m_shadow_method_str = b;
		}
		else if( v == "soft_shadow_enabled")
		{
			string b = element->GetText();
			m_soft_shadow_enabled = b == "true";
		}
		else if( v == "color_texture_is_srgb")
		{
			string b = element->GetText();
			m_color_texture_is_srgb = b == "true";
		}
		else if( v == "mblur_enabled")
		{
			string b = element->GetText();
			m_mblur_enabled = b == "true";
		}
		else if( v == "depth_of_field_enabled")
		{
			string b = element->GetText();
			m_depth_of_field_enabled = b == "true";
		}
		else if( v == "fps_cam_fov")
		{
			float tmp = ng::atof(element->GetText());
			if(tmp > 0 && tmp <= 179)
			{
				m_fps_cam_fov = tmp;
			}
		}
		else if( v == "envmap_size")
		{
			int tmp = atoi( element->GetText());
			if(tmp > 0)
			{
				m_fboEnvMap_size = tmp;
			}
		}
		else if( v == "shadowmap_size")
		{
			int tmp = atoi( element->GetText());
			if(tmp > 0)
			{
				m_fboShadowMap_size = tmp;
			}
		}
		else if( v == "play_time")
		{
			int tmp = atoi( element->GetText());
			if(tmp > 0)
			{
				m_play_time = tmp;
			}
		}
		else if( v == "datapath")
		{
			KCL::AssetFile::AddScenePath( element->GetText());
		}
		else if( v == "secondary_datapath")
		{
			KCL::AssetFile::AddScenePath(element->GetText());
			m_secondary_path = element->GetText();
		}
		else if( v == "num_shadows")
		{
			m_num_shadow_maps = atoi( element->GetText());
		}
		else if ( v == "tier")
		{
			m_tier_level_name = element->GetText();
		}
	}
}


void KCL::SceneHandler::LoadParams( const char* xml_scene_settings_fileName)
{
	AssetFile xmlfile(xml_scene_settings_fileName);
	
	if(!xmlfile.GetLastError())
	{
		SceneXML sceneXML(this);
		TiXmlDocument doc;
		doc.LoadFile( xmlfile.GetBuffer(), xmlfile.GetLength());
		doc.Accept( &sceneXML);
	}
	else
	{
		INFO("scene xml is missing...");
	}
}
