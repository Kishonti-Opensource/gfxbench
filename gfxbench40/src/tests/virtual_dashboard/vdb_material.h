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

#include "opengl/glb_texture.h"


#include <string>
#include <vector>
#include "render_statistics_defines.h"


#ifdef TEXTURE_COUNTING
#include <set>
#endif


class Shader;
class GLB_ogg_decoder;

namespace VDB
{
	class Image2D;
	class MaterialFactory;

	class Material : public KRL::Material
	{
		friend class KCL::Material;
	public:
		/*override*/ void postInit();
        /*override*/ void preInit( KCL::uint32 &texture_num, int type, int pass_type);

		Shader* CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error);

		~Material();

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
	};
}

#endif
