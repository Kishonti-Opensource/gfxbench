/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_LENSFLARE_H
#define GLB_LENSFLARE_H

#include "kcl_math3d.h"
#include "kcl_camera2.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_texture.h"
#include "map"
#include "string"

namespace GLB
{


class Lensflare {

public:
	Lensflare() ;
	virtual ~Lensflare() ;

	KCL::uint32 m_depth_texture ;
	KCL::int32 m_color_texture ;

	void Init(int witdh, int height) ;
	void Execute() ;
	void Render() ;



	KCL::Camera2 *m_camera ;
private:

	KCL::Vector3D m_light_dir ;

	struct Flare
	{
		float size ;
		float distance ;
		GLB::GLBTexture* texture ; 
	};

	Flare* m_flares ;

	std::map<std::string,GLB::GLBTexture* > m_texture_pool ;

	GLB::GLBTexture* m_lens_dirt ;

	GLB::GLBShader2 *m_compute_oc ;
	GLB::GLBShader2 *m_lensflare_shader ;

	KCL::uint32 m_viewport_width, m_viewport_height ;
	KCL::uint32 m_flare_count ;

	KCL::Vector4D m_screenspace_sun_pos ;

	KCL::uint32 m_depth_sampler ;
	KCL::uint32 m_atomic_counter ;

	KCL::uint32 m_lensflare_vbo ;
	KCL::uint32 m_lensflare_vao ;
	KCL::uint32 m_offsets_buffer ;

	KCL::uint32 m_lensflare_fbo ;

	KCL::uint32 m_offset_count ;
};


}


#endif // GLB_LENSFLARE