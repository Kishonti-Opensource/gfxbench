/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_MESH_H
#define GFXB_MESH_H

#include <kcl_mesh.h>
#include <vector>

namespace GFXB
{
	class Mesh3;

	class Mesh : public KCL::Mesh
	{
	public:
		Mesh(const std::string &name, KCL::Node *parent, KCL::Object *owner);

		KCL::Vector3D m_center;

		std::vector<KCL::uint32> m_cull_frames;

		std::vector<KCL::Vector4D> m_debug_frustum_ids;

		void CalculateCenter();

		void ClearFrustumIds()
		{
			for (size_t i = 0; i < m_debug_frustum_ids.size(); i++)
			{
				m_debug_frustum_ids[i].x = 0;
			}
		}

		inline Mesh3 *GetMesh3(const KCL::Vector3D &view_pos, float lod1_distance2, float lod2_distance2, KCL::uint32 *lod_index = nullptr)
		{
			KCL::uint32 selected_lod = 0;
			KCL::Mesh3 *result = m_mesh_variants[0];

			if (m_mesh_variants[1] || m_mesh_variants[2])
			{
				//const float d2 = KCL::Vector3D::distance2(view_pos, KCL::Vector3D(&m_world_pom.v[12]));
				const float d2 = KCL::Vector3D::distance2(view_pos, m_center);

				if (m_mesh_variants[2] && d2 > lod2_distance2)
				{
					selected_lod = 2;
					result = m_mesh_variants[2];
				}
				else if (m_mesh_variants[1] && d2 > lod1_distance2)
				{
					selected_lod = 1;
					result = m_mesh_variants[1];
				}
			}

			if (lod_index)
			{
				*lod_index = selected_lod;
			}

			return  (Mesh3*)result;
		}
	};
}

#endif
