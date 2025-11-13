/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DXB_MATERIAL_H
#define DXB_MATERIAL_H

#include <kcl_base.h>
#include <kcl_object.h>
#include <kcl_material.h>
#include "d3d11/dxb_texture.h"

#include "krl_material.h"
#include "glb_kcl_adapter.h"
#include "d3d11/dxb_planarmap.h"

#include <string>
#include <vector>

class Shader;

namespace DXB
{
	class Image2D;
	class MaterialFactory;

	class Material : public KRL::Material
	{
		friend class KCL::Material;
		friend class MaterialFactory;

	private:
		static Shader* m_last_shader;
		static const  UINT m_textureArraySlot = 15;
		_ogg_decoder* m_video;
		DXB::DXBTexture* m_videoTexture;
		DXB::PlanarMap* m_planar_map;

	public:

		/*override*/ void postInit();
		void preInit(KCL::uint32 &texture_num,  int type, int pass_type);
		Shader* CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error);

		virtual void LoadVideo( const char *filename);
		virtual void PlayVideo( float time_in_sec);

		inline PlanarMap* getPlanarMap() const	{ return m_planar_map; }
		inline void setPlanarMap(DXB::PlanarMap* planarMap)	{ m_planar_map = planarMap; }

		~Material();
	protected:
		Material( const char *name);
	private:
		Material(const Material&);
		Material& operator=(const Material&);
	};


	class MaterialFactory : public KCL::MaterialFactory
	{
	public:
		MaterialFactory(int scene_version) : m_scene_version(scene_version) {}
		virtual KCL::Material *Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner);

	private:
		int m_scene_version;
	};

}//!namespace

#endif
