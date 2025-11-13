/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_MESH_SHAPE_H
#define GFXB5_MESH_SHAPE_H

#include "kcl_mesh.h"

namespace GFXB
{
	struct _basic_vertex
	{
		KCL::Vector3D m_pos;
		KCL::Vector2D m_tc;
	};

	struct _bezier_vertex
	{
		KCL::Vector3D m_patch;
	};

	struct _basic_vertex_with_normal_tangent : public _basic_vertex
	{
		KCL::Vector3D m_n;
		KCL::Vector3D m_t;
	};

	struct _basic_vertex_with_normal_tangent_color : public _basic_vertex_with_normal_tangent
	{
		KCL::Vector3D m_col;
	};

	struct _basic_vertex_with_bone : public _basic_vertex_with_normal_tangent
	{
		KCL::Vector4D m_bone_index;
		KCL::Vector4D m_bone_weight;
	};

	class Mesh3 : public KCL::Mesh3
	{
	public:
		enum Flags
		{
			FLAG_NONE =					0,
			FLAG_CREATE_SHADOW_BUFFER = 1 << 0,
			FLAG_SHADOW_UV0 =			1 << 1,
		};

		Mesh3(const char *name) : KCL::Mesh3(name)
		{
			m_vbid = 0;
			m_ibid = 0;
			m_shadow_vbid = 0;
		}

		bool UploadMesh(int flags = 0);

		KCL::uint32 m_vbid;
		KCL::uint32 m_ibid;
		KCL::uint32 m_shadow_vbid;

	private:
		struct MeshLayout
		{
			bool has_position;
			bool has_normal;
			bool has_tangent;
			bool has_color;
			bool has_texcoord0;
			bool has_texcoord1;
			bool has_texcoord2;
			bool has_texcoord3;
			bool has_bones;

			MeshLayout()
			{
				has_position = false;
				has_normal = false;
				has_tangent = false;
				has_color = false;
				has_texcoord0 = false;
				has_texcoord1 = false;
				has_texcoord2 = false;
				has_texcoord3 = false;
				has_bones = false;
			}
		};

		bool UploadVertexBuffer(const MeshLayout &layout, KCL::uint32 &buffer);
		bool UploadIndexBuffer();
	};
}

#endif