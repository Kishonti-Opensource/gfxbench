/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MATERIAL_H
#define MATERIAL_H

#include <kcl_base.h>
#include <kcl_object.h>
#include <kcl_material.h>

#include "krl_material.h"

#include "glb_planarmap.h"
#include "opengl/glb_texture.h"


#include <string>
#include <vector>
#include "render_statistics_defines.h"


#ifdef TEXTURE_COUNTING
#include <set>
#endif


class Shader;
class GLB_ogg_decoder;

namespace GLB
{
	class Image2D;
	class MaterialFactory;

	class Material : public KRL::Material
	{
		friend class KCL::Material;
		friend class MaterialFactory;
	public:
		PlanarMap *m_planar_map;

		/*override*/ void postInit();
        /*override*/ void preInit( KCL::uint32 &texture_num, int type, int pass_type);

		Shader* CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error);

		virtual void LoadVideo( const char *filename);
		virtual void PlayVideo( float time_in_sec);
		virtual void DecodeVideo();
		
		~Material();

		GLB_ogg_decoder *m_ogg_decoder;

		#ifdef TEXTURE_COUNTING
		static void TextureCounter(std::set<KCL::uint32> &textureCounter);
		static void NullTextureCounter();
		#endif
		
		KCL::int32 m_ubo_id;
		KCL::int32 m_ubo_offset;

	protected:
		Material( const char *name);
	private:
		Material(const Material&);
		Material& operator=(const Material&);
        
        void preInit( int type) {}
		float m_video_time_in_sec;
	};


	class MaterialFactory : public KCL::MaterialFactory
	{
	public:
		MaterialFactory(int scene_version) : m_scene_version(scene_version) {}
		virtual KCL::Material *Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner);

	private:
		int m_scene_version;
	};

}

#endif
