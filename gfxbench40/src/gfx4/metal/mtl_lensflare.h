/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_LENSFLARE_H
#define MTL_LENSFLARE_H

#include "kcl_math3d.h"
#include "kcl_camera2.h"

#include <Metal/Metal.h>

#include "map"
#include "string"

namespace MetalRender
{

class Pipeline;
class Texture;
class DynamicDataBuffer;

class Lensflare {

public:
	Lensflare(id<MTLDevice> device) ;
	virtual ~Lensflare() ;

	id<MTLTexture> m_depth_texture ;
	id<MTLTexture> m_color_texture ;

	void Init(int witdh, int height, bool portrait_mode) ;
	void Execute(id<MTLCommandBuffer> command_buffer, DynamicDataBuffer * ddb);
	void Render(id<MTLCommandBuffer> command_buffer, DynamicDataBuffer * ddb);



	KCL::Camera2 *m_camera ;
private:

	KCL::Vector3D m_light_dir ;

	struct Flare
	{
		float size ;
		float distance ;
		Texture* texture ;
	};

	Flare* m_flares ;

	std::map<std::string,Texture* > m_texture_pool ;

	Texture* m_lens_dirt ;

	Pipeline *m_compute_oc ;
	Pipeline *m_lensflare_shader ;

	KCL::uint32 m_viewport_width, m_viewport_height ;
	KCL::uint32 m_flare_count ;
	bool m_portrait_mode;

	KCL::Vector4D m_screenspace_sun_pos ;

	id<MTLBuffer> m_vertex_data ;
	id<MTLBuffer> m_offsets_buffer ;
	id<MTLBuffer> m_atomic_counter ;

	KCL::uint32 m_offset_count ;

	id<MTLDevice> m_device;
};


}


#endif // MTL_LENSFLARE