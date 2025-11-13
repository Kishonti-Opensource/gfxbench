/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_LIGHTSHAFT_UTIL_H
#define GFXB_LIGHTSHAFT_UTIL_H

#include "common/gfxb_scene_base.h"
#include "common/gfxb_factories.h"
#include "common/gfxb_shader.h"

namespace GFXB
{
	class Shapes;
	class Light;

	class LightshaftUtil
	{
		KCL::uint32 m_slice_spot_shader;
		KCL::uint32 m_slice_box_shader;
		KCL::uint32 m_cone_spot_shader;
		KCL::uint32 m_cone_box_shader;
		uint32_t dummy_index_buf;
		static const unsigned num_tris = 80;
		static const unsigned num_vertices = num_tris * 3;
		int m_dither_offset;

		std::vector<KCL::Vector4D> final_vertices;

	public:
		LightshaftUtil()
		{
			m_slice_spot_shader = 0;
			m_cone_spot_shader = 0;
			m_slice_box_shader = 0;
			m_cone_box_shader = 0;
			dummy_index_buf = 0;
			m_is_warmup = false;
		}

		bool Init(KCL::uint32 viewport_width, KCL::uint32 viewport_height);

		void SetWarmup(bool value)
		{
			m_is_warmup = value;
		}

		void RenderSpotLightShaft(KCL::uint32 job, KCL::Camera2* cam, KCL::Camera2* light_cam, KCL::Matrix4x4 light_inv_view_proj, KCL::Matrix4x4 light_model, GFXB::Shapes* m_shapes, const void** p);
		void RenderBoxLightShaft(KCL::uint32 job, KCL::Camera2 *camera, Shapes *m_shapes, Light *light, const void **p);

	protected:

		void CutSlice(const KCL::Matrix4x4& transform, const KCL::Vector4D &plane, std::vector<KCL::Vector3D>& points, std::vector<KCL::Vector4D>& vertices, KCL::Vector3D& plane_normal, KCL::Vector3D& origin, const std::vector<uint16_t>& edge_list)
		{
			std::vector<KCL::uint16> tmp_indices;
			std::vector<float> d(points.size());

			uint16_t base_idx;

			// transform to the light space
			//for( int i = 0; i < SLICE_CONE_EDGES + 1; i++ )
			for (unsigned i = 0; i < points.size(); i++)
			{
				KCL::Vector3D tmp;
				mult4x4(transform, KCL::Vector3D(points[i].v), tmp);
				points[i] = tmp;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Quick hack - replace code later! surface clipped by front plane with CPU generated geometry
			// [this is prone to visualization bugs as HW clipper and CPU generated geometry is bound to be different]

			// Find out if we intersect front plane (i.e. camera is inside light volume)
			// compute distances
			float min_z = FLT_MAX;
			float max_z = -FLT_MAX;
			for (unsigned i = 0; i < points.size(); i++)
			{
				KCL::Vector4D v(points[i].x, points[i].y, points[i].z, 1.0f);
				d[i] = KCL::Vector4D::dot(plane, v);
				if (d[i] < min_z)
					min_z = d[i];
				if (d[i] > max_z)
					max_z = d[i];
			}

			if (min_z > 0)
			{
				vertices.resize(0);
				memset(vertices.data(), 0, vertices.capacity() * sizeof(KCL::Vector4D));
				return; // Exit if no intersection
			}

			// compute slice distances
			//float step_dist = 1.0f;
			//float count = 0; //(max_z - min_z) / step_dist;

			//count = 1; // we always do only single slice // (max_z - min_z);

			//step_dist = 1.0f;

			// slice
			float dist = 0.0f; // set distance (from near clip plane) where we want to clip out geometry - should really be (ZERO+epsilon)
			{
				//we only want slices, so this is 0
				base_idx = 0;
				uint16_t num_new_vertices = 0;

				for (uint32_t i = 0; i < edge_list.size(); i += 2)
				{
					//edge start
					uint32_t i0 = edge_list[i + 0];
					//edge end
					uint32_t i1 = edge_list[i + 1];

					//signed distance of edge start from plane
					float d0 = d[i0];
					//signed distance of edge end from plane
					float d1 = d[i1];

					//if edge ends are not on the same side
					//then slice!
					if ((d0 > dist) != (d1 > dist))
					{
						//move both by epsilon?
						d0 -= dist;
						d1 -= dist;

						//ratio of d1 to both
						float ii = (-d1) / (d0 - d1);

						//vec4f &v = vertices->Push_back();
						//LRP3( &v, ii, &points[i1], &points[i0]);

						KCL::Vector3D v;
						//move point along edge to near plane
						v = KCL::Vector3D::interpolate(KCL::Vector3D(points[i1].v), KCL::Vector3D(points[i0].v), ii);
						//v.col = colors[iter%4];
						//v = scrollStrength;

						vertices.push_back(KCL::Vector4D(v, 1.0));

						if (num_new_vertices >= 2)
						{
							tmp_indices.push_back(base_idx);
							tmp_indices.push_back((uint16_t)(vertices.size() - 2));
							tmp_indices.push_back((uint16_t)(vertices.size() - 1));
						}

						num_new_vertices++;
					}
				}
				if (num_new_vertices)
				{
					plane_normal.set(plane.v);
					origin.x = vertices[base_idx].x;
					origin.y = vertices[base_idx].y;
					origin.z = vertices[base_idx].z;

					//NOTE: since this would just reorder vertices, but leave indices untouched, would produce buggy geometry in some cases - not needed here
					//std::stable_sort( &m_vertices[base_idx], &m_vertices[base_idx] + num_new_vertices, sorter);
				}
			}
		}

		static bool AngleSorter(const KCL::Vector4D& a, const KCL::Vector4D& b)
		{
			return a.w < b.w;
		}

		void Orthogonal(KCL::Vector3D &src, const KCL::Vector3D &d)
		{
			int i = fabs(d.x) > fabs(d.y) ? (fabs(d.x) > fabs(d.z) ? 0 : 2) : (fabs(d.y) > fabs(d.z) ? 1 : 2);
			src.v[i] = d.v[(i + 1) % 3];
			src.v[(i + 1) % 3] = -d.v[i];
			src.v[(i + 2) % 3] = 0;
		}


		void CreateSlice(std::vector<KCL::Vector4D>& vertices, const KCL::Vector4D& plane)
		{
			if (vertices.size() < 1)
				return;

			KCL::Vector3D up;
			KCL::Vector3D planexyz(plane.v);
			Orthogonal(up, planexyz);
			up.normalize();

			KCL::Vector3D right = KCL::Vector3D::cross(up, planexyz).normalize();

			std::vector<KCL::Vector4D> projected_vertices(vertices.size());

			KCL::Vector4D center;

			for (unsigned c = 0; c < vertices.size(); ++c)
			{
				center = center + vertices[c];
			}
			center = center / (float)vertices.size();

			for (unsigned c = 0; c < vertices.size(); ++c)
			{
				KCL::Vector3D projected_vertex;
				KCL::Vector3D tmp;

				tmp = KCL::Vector3D(vertices[c].v) - KCL::Vector3D(center.v);

				projected_vertex.x = KCL::Vector3D::dot(right, tmp);
				projected_vertex.y = KCL::Vector3D::dot(up, tmp);
				projected_vertex.z = 0.0f;

				KCL::Vector3D normxyz = projected_vertex.normalize();

				projected_vertex = KCL::Vector3D(vertices[c].v);

				projected_vertices[c] = KCL::Vector4D(projected_vertex, atan2f(normxyz.y, normxyz.x));
			}

			std::stable_sort(projected_vertices.begin(), projected_vertices.end(), AngleSorter);

			std::vector<KCL::Vector4D> polygonized;
			polygonized.reserve(num_vertices);

			for (unsigned c = 0; c < projected_vertices.size() - 1; c += 1)
			{
				polygonized.push_back(projected_vertices[c]);
				polygonized.push_back(projected_vertices[c + 1]);
				polygonized.push_back(center);
			}

			if (projected_vertices.size() > 0)
			{
				polygonized.push_back(projected_vertices.back());
				polygonized.push_back(projected_vertices.front());
				polygonized.push_back(center);
			}

			vertices.clear();
			for (unsigned c = 0; c < polygonized.size(); ++c)
			{
				vertices.push_back(polygonized[c]);
				vertices.back().w = 1.0f;
			}
		}

		bool m_is_warmup;

	};
}

#endif