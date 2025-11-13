/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_lightshaft_util.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_light.h"
#include "common/gfxb_shadow_map.h"
#include "common/gfxb_gauss_blur_helper.h"

using namespace GFXB;

bool LightshaftUtil::Init(KCL::uint32 viewport_width, KCL::uint32 viewport_height)
{
	ShaderFactory *shader_factory = ShaderFactory::GetInstance();

	// Dither values with Bayer matrix
	const float dither_pattern_straight[16] =
	{
		0.0f, 0.5f, 0.125f, 0.625f,
		0.75f, 0.22f, 0.875f, 0.375f,
		0.1875f, 0.6875f, 0.0625f, 0.5625,
		0.9375f, 0.4375f, 0.8125f, 0.3125
	};

	const float dither_pattern_flipped[64] =
	{
		0.625f, 0.125f, 0.5f, 0.0f,
		0.375f, 0.875f, 0.22f, 0.75f,
		0.5625, 0.0625f, 0.6875f, 0.1875f,
		0.3125, 0.8125f, 0.4375f, 0.9375f,

		0.125f, 0.5f, 0.0f, 0.625f,
		0.875f, 0.22f, 0.75f, 0.375f,
		0.0625f, 0.6875f, 0.1875f, 0.5625,
		0.8125f, 0.4375f, 0.9375f, 0.3125,

		0.5f, 0.0f, 0.625f, 0.125f,
		0.22f, 0.75f, 0.375f, 0.875f,
		0.6875f, 0.1875f, 0.5625, 0.0625f,
		0.4375f, 0.9375f, 0.3125, 0.8125f,

		0.0f, 0.625f, 0.125f, 0.5f,
		0.75f, 0.375f, 0.875f, 0.22f,
		0.1875f, 0.5625, 0.0625f, 0.6875f,
		0.9375f, 0.3125, 0.8125f, 0.4375f
	};

	m_dither_offset = ((4 - ((int)(viewport_height / 2) % 4)) % 4) * 16;

	const std::vector<float> dither_pattern = (nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE) == NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP)
		? std::vector<float>(dither_pattern_flipped + m_dither_offset, dither_pattern_flipped + m_dither_offset + 16)
		: std::vector<float>(dither_pattern_straight, dither_pattern_straight + 16);

	std::string dither_pattern_str = GaussBlurHelper::GaussFloatListToString(dither_pattern,false);

	m_slice_spot_shader = shader_factory->AddDescriptor(
		ShaderDescriptor("slice.vert", "lightshaft.frag").AddDefineString("BAYER_ARRAY",dither_pattern_str.c_str()).AddDefineInt("LIGHTSHAFT_MAX_VERTICES", num_vertices));

	m_slice_box_shader = shader_factory->AddDescriptor(
		ShaderDescriptor("slice.vert", "lightshaft.frag").AddDefineString("BAYER_ARRAY", dither_pattern_str.c_str()).AddDefine("IS_BOX").AddDefineInt("LIGHTSHAFT_MAX_VERTICES", num_vertices));

	m_cone_spot_shader = shader_factory->AddDescriptor(ShaderDescriptor("cone.vert", "lightshaft.frag").AddDefineString("BAYER_ARRAY", dither_pattern_str.c_str()));
	m_cone_box_shader = shader_factory->AddDescriptor(ShaderDescriptor("box.vert", "lightshaft.frag").AddDefineString("BAYER_ARRAY", dither_pattern_str.c_str()).AddDefine("IS_BOX"));

	std::vector<uint32_t> indices(num_vertices);
	for (unsigned c = 0; c < num_tris; ++c)
	{
		//tris
		indices[c * 3 + 0] = c * 3 + 0;
		indices[c * 3 + 1] = c * 3 + 2;
		indices[c * 3 + 2] = c * 3 + 1;

		//lines
		//indices[c * 3 + 0] = c * 3 + 0;
		//indices[c * 3 + 1] = c * 3 + 1;
		//indices[c * 3 + 2] = c * 3 + 2;
	}
	dummy_index_buf = 0;
	if (!nglGenIndexBuffer(dummy_index_buf, NGL_R32_UINT, num_vertices, indices.data()))
	{
		assert(0);
		return false;
	}
	return true;
}

float* get_bayer_data()
{
	static std::vector<float> bayer_array(64);
	bayer_array[0] = 0.0;
	bayer_array[1] = 32.0;
	bayer_array[2] = 8.0;
	bayer_array[3] = 40.0;
	bayer_array[4] = 2.0;
	bayer_array[5] = 34.0;
	bayer_array[6] = 10.0;
	bayer_array[7] = 42.0;
	bayer_array[8] = 48.0;
	bayer_array[9] = 16.0;
	bayer_array[10] = 56.0;
	bayer_array[11] = 24.0;
	bayer_array[12] = 50.0;
	bayer_array[13] = 18.0;
	bayer_array[14] = 58.0;
	bayer_array[15] = 26.0;
	bayer_array[16] = 12.0;
	bayer_array[17] = 44.0;
	bayer_array[18] = 4.0;
	bayer_array[19] = 36.0;
	bayer_array[20] = 14.0;
	bayer_array[21] = 46.0;
	bayer_array[22] = 6.0;
	bayer_array[23] = 38.0;
	bayer_array[24] = 60.0;
	bayer_array[25] = 28.0;
	bayer_array[26] = 52.0;
	bayer_array[27] = 20.0;
	bayer_array[28] = 62.0;
	bayer_array[29] = 30.0;
	bayer_array[30] = 54.0;
	bayer_array[31] = 22.0;
	bayer_array[32] = 3.0;
	bayer_array[33] = 35.0;
	bayer_array[34] = 11.0;
	bayer_array[35] = 43.0;
	bayer_array[36] = 1.0;
	bayer_array[37] = 33.0;
	bayer_array[38] = 9.0;
	bayer_array[39] = 41.0;
	bayer_array[40] = 51.0;
	bayer_array[41] = 19.0;
	bayer_array[42] = 59.0;
	bayer_array[43] = 27.0;
	bayer_array[44] = 49.0;
	bayer_array[45] = 17.0;
	bayer_array[46] = 57.0;
	bayer_array[47] = 25.0;
	bayer_array[48] = 15.0;
	bayer_array[49] = 47.0;
	bayer_array[50] = 7.0;
	bayer_array[51] = 39.0;
	bayer_array[52] = 13.0;
	bayer_array[53] = 45.0;
	bayer_array[54] = 5.0;
	bayer_array[55] = 37.0;
	bayer_array[56] = 63.0;
	bayer_array[57] = 31.0;
	bayer_array[58] = 55.0;
	bayer_array[59] = 23.0;
	bayer_array[60] = 61.0;
	bayer_array[61] = 29.0;
	bayer_array[62] = 53.0;
	bayer_array[63] = 21.0;
	return bayer_array.data();
}

void LightshaftUtil::RenderSpotLightShaft(KCL::uint32 job, KCL::Camera2* cam, KCL::Camera2* light_cam, KCL::Matrix4x4 light_inv_view_proj, KCL::Matrix4x4 light_model, GFXB::Shapes* m_shapes, const void** p)
{
	KCL::Vector3D origin;
	KCL::Vector3D near_plane;
	final_vertices.clear();
	final_vertices.reserve(num_vertices);
	memset(final_vertices.data(), 0, num_vertices * sizeof(KCL::Vector4D));
	std::vector<uint16_t> edgeList;
	std::vector<KCL::Vector3D> points;
	edgeList.reserve(num_vertices);
	{
		std::vector<KCL::Vector2D> tcs;
		std::vector<uint16_t> indices;
		KCL::Mesh3::CreateCylinder(points, tcs, indices, 15, 1, &edgeList);
	}

	KCL::Vector4D plane = cam->GetCullPlane(KCL::CULLPLANE_NEAR);

	CutSlice(light_inv_view_proj, plane, points, final_vertices, near_plane, origin, edgeList);
	CreateSlice(final_vertices, plane);

	KCL::Matrix4x4 vp = cam->GetViewProjection();

	p[UNIFORM_VP] = vp.v;
	p[UNIFORM_MVP] = (light_inv_view_proj * vp).v;
	//p[UNIFORM_MVP] = light_model.v;

	if (final_vertices.size() > num_vertices)
	{
		INFO("Error: Ligthshaft to many generated vertices: %d (max %d)", final_vertices.size(), num_vertices);
		assert(0);
	}

	p[UNIFORM_VERTEX_ARRAY] = final_vertices.data();

	p[UNIFORM_BAYER_ARRAY] = get_bayer_data();

	KCL::Vector4D frustum_planes[6];
	for (int c = 0; c < 6; ++c)
	{
		frustum_planes[c] = light_cam->GetCullPlane(c);
	}

	p[UNIFORM_FRUSTUM_PLANES] = frustum_planes;
	if (final_vertices.size() > 0)
	{
		//inv viewproj is not needed for this
		p[UNIFORM_MODEL] = light_model.v;


		nglDraw(job, NGL_TRIANGLES, m_slice_spot_shader, 0, 0, dummy_index_buf, NGL_FRONT_SIDED, p);
	}

	p[UNIFORM_MODEL] = light_inv_view_proj.v;

	nglDraw(job, NGL_TRIANGLES, m_cone_spot_shader, 1, &m_shapes->m_cylinder_vbid, m_shapes->m_cylinder_ibid, NGL_FRONT_SIDED, p);
	//nglDraw( job, NGL_TRIANGLES, m_cone_spot_shader, 1, &m_shapes->m_cylinder_vbid, m_shapes->m_cylinder_ibid, NGL_TWO_SIDED, p );
}


void LightshaftUtil::RenderBoxLightShaft(KCL::uint32 job, KCL::Camera2 *camera, Shapes *m_shapes, Light *light, const void **p)
{
	KCL::Vector3D origin;
	KCL::Vector3D near_plane;
	final_vertices.clear();
	final_vertices.reserve(num_vertices);
	std::vector<uint16_t> edgeList;
	std::vector<KCL::Vector3D> points;
	edgeList.reserve(num_vertices);
	{
		std::vector<KCL::Vector2D> tcs;
		std::vector<uint16_t> indices;
		KCL::Mesh3::CreateCube(points, tcs, indices, &edgeList);
	}

	const KCL::Vector4D &plane = camera->GetCullPlane(KCL::CULLPLANE_NEAR);

	KCL::Matrix4x4 &light_model = light->m_uniform_shape_world_pom;

	CutSlice(light_model, plane, points, final_vertices, near_plane, origin, edgeList);
	CreateSlice(final_vertices, plane);

	if (m_is_warmup)
	{
		final_vertices.clear();
		final_vertices.resize(num_vertices, KCL::Vector4D());
	}

	if( final_vertices.size() > 0)
	{
		final_vertices.resize(num_vertices, KCL::Vector4D(0, 0, 0, 0));
		//inv viewproj is not needed for this
		KCL::Matrix4x4 m;
		p[UNIFORM_MODEL] = m.v;
		p[UNIFORM_VERTEX_ARRAY] = final_vertices.data();
		nglDraw(job, NGL_TRIANGLES, m_slice_box_shader, 0, 0, dummy_index_buf, NGL_FRONT_SIDED, p);
	}

	p[UNIFORM_MODEL] = light_model.v;
	nglDraw(job, NGL_TRIANGLES, m_cone_box_shader, 1, &m_shapes->m_cube_vbid, m_shapes->m_cube_ibid, NGL_FRONT_SIDED, p);
}
