/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_room.h>

#include <kcl_material.h>

using namespace KCL;

static int OVERLAP( const KCL::Vector4D *planes, KCL::uint32 num, KCL::AABB *aabb);
static bool IsRoomInPath( KCL::XRoom *r);


KCL::uint32 KCL::XRoom::m_counter = 1;
static std::vector<KCL::XRoom*> s_frustum_cull_path;
static bool portals_enabled = true;


void KCL::XPortal::Edge::CreateEdgeVector( const KCL::Vector3D &P1, const KCL::Vector3D &P0)
{
	m_edge_vector = P1 - P0;
	Vector3D::normalize( m_edge_vector);
}


void KCL::XPortal::Edge::CreatePlane( bool eye_side, const KCL::Vector3D &eye, const KCL::Vector3D &point, KCL::Vector4D &plane)
{
	Vector3D normal;
	Vector3D t = eye - point;
	t.normalize();

	if( eye_side)
	{
		normal = Vector3D::cross( m_edge_vector, t);
	}
	else
	{
		normal = Vector3D::cross( t, m_edge_vector);
	}
	normal.normalize();
	plane.set( normal.x, normal.y, normal.z, -Vector3D::dot( point, normal));
}


KCL::XPortal::XPortal( const char* name): m_name( name), m_front( 0), m_back( 0)
{
}


void KCL::XPortal::Init()
{
	KCL::Vector3D normal;
	KCL::Vector3D e0 = m_points[1] - m_points[0];
	KCL::Vector3D e1 = m_points[2] - m_points[0];

	Vector3D::normalize( e0);
	Vector3D::normalize( e1);

	normal = Vector3D::cross( e0, e1);
	Vector3D::normalize( normal);

	m_plane.set(
		normal.x,
		normal.y,
		normal.z,
		-Vector3D::dot( normal, m_points[0])
		);

}


void KCL::XPortal::ConnectRooms( XRoom *r0, XRoom *r1)
{
	if( !r0 || !r1)
	{
		return;
	}
	KCL::Vector3D he0, c0;
	KCL::Vector3D he1, c1;

	r0->m_aabb.CalculateHalfExtentCenter( he0, c0);
	r1->m_aabb.CalculateHalfExtentCenter( he1, c1);

	float d0 = c0.x * m_plane.x + c0.y * m_plane.y + c0.z * m_plane.z + m_plane.w;
	float d1 = c1.x * m_plane.x + c1.y * m_plane.y + c1.z * m_plane.z + m_plane.w;

	if( (d0 > 0.0f) && (d1 > 0.0f))
	{
		return;
	}
	if( d0 > 0.0f)
	{
		m_front = r0;
		m_back = r1;
	}
	else
	{
		m_front = r1;
		m_back = r0;
	}
}


KCL::XRoom::XRoom( const char* name): Object( name, ROOM), m_frame_when_touched( 0)
{
}


KCL::XRoom::~XRoom()
{
	for( KCL::uint32 i=0; i<m_meshes.size(); i++)
	{
		delete m_meshes[i];
	}
}


bool Inside( Vector3D &p, const KCL::Vector4D &plane)
{
	float d = plane.x * p.x + plane.y * p.y + plane.z * p.z + plane.w;

	if( d > 0.0f)
	{
		return false;
	}
	else
	{
		return true;
	}
}


Vector3D Intersect( Vector3D &p0, Vector3D &p1, const KCL::Vector4D &plane)
{
	float d0 = plane.x * p0.x + plane.y * p0.y + plane.z * p0.z + plane.w;
	float d1 = plane.x * p1.x + plane.y * p1.y + plane.z * p1.z + plane.w;

	float f = d0 / ( d0 - d1);

	Vector3D result = Vector3D::interpolate( p0, p1, f);
	return result;

}


void KCL::XRoom::FrustumCull( std::vector<XRoom*> &visible_rooms, std::vector<PlanarMap*> &visible_planar_maps, std::vector<Mesh*> visible_meshes[3], std::vector< std::vector<KCL::Mesh*> > &visible_instances, XRoom* r, Camera2* camera, const KCL::Vector4D *planes, KCL::uint32 num, XRoom* camera_room, bool **pvs, CullingAlgorithm *culling_algorithm)
{
	std::vector<KCL::MeshInstanceOwner2*> visible_mios;

	m_counter++;
	s_frustum_cull_path.clear();
	FrustumCull2( visible_rooms, visible_planar_maps, visible_meshes, visible_mios, r, camera, planes, num, camera_room, pvs, culling_algorithm);

	visible_instances.resize( visible_mios.size());

    int front_counter = 0;
    int back_counter = (int)visible_mios.size() - 1;
	for( size_t i=0; i<visible_mios.size(); i++)
	{
        if (visible_mios[i]->m_visible_instances[0]->m_material->m_is_tesselated)
        {
            visible_instances[front_counter++] = visible_mios[i]->m_visible_instances;
        }
        else
        {
		    visible_instances[back_counter--] = visible_mios[i]->m_visible_instances;
        }
	}
}


void KCL::XRoom::FrustumCull2( std::vector<XRoom*> &visible_rooms, std::vector<PlanarMap*> &visible_planar_maps, std::vector<Mesh*> visible_meshes[3], std::vector<KCL::MeshInstanceOwner2*> &visible_mios, XRoom* r, Camera2* camera, const KCL::Vector4D *planes, KCL::uint32 num, XRoom* camera_room, bool **pvs, CullingAlgorithm *culling_algorithm)
{
	s_frustum_cull_path.push_back( r);

	visible_rooms.push_back( r);

    bool reflection_pass = (culling_algorithm && (culling_algorithm->m_type == CullingAlgorithm::CA_REFL) && (!culling_algorithm->m_force_cast));

	for( KCL::uint32 i=0; i<r->m_meshes.size(); i++)
	{
		Mesh* m = r->m_meshes[i];

        //if( reflection_pass && (!m->m_material->m_is_reflection_caster || !(m->m_flags & KCL::Mesh::OF_REFLECTION_CASTER)) )
        if (reflection_pass && !(m->m_flags & KCL::Mesh::OF_REFLECTION_CASTER))
        {
            continue;
        }

		//NOTE: shadow culling uses a different function, thus we can early out in the shadow-only case
        if( m->m_material->m_is_shadow_only)
		{
			continue;
		}

		if( m->m_frame_when_rendered == m_counter)
		{
			continue;
		}

		m->m_frame_when_rendered = m_counter;

        OverlapResult result = OVERLAP( planes, num, &m->m_aabb);

        if( result != OVERLAP_OUTSIDE)
		{
            if ( culling_algorithm && culling_algorithm->CullMesh( camera, m))
            {
                continue;
            }

			if( m->m_mio2)
			{
				if( m->m_mio2->m_frame_when_rendered != m_counter)
				{
					m->m_mio2->m_frame_when_rendered = m_counter;

					m->m_mio2->m_visible_instances.clear();

                    visible_mios.push_back( m->m_mio2);
				}

				m->m_mio2->m_visible_instances.push_back( m);
			}
			else
			{
				visible_meshes[m->m_material->m_is_decal ? 2 : m->m_material->m_is_transparent].push_back( m);
			}
		}
	}

	for( KCL::uint32 i=0; i<r->m_planar_maps.size(); i++)
	{
		PlanarMap* pm = r->m_planar_maps[i];

		if( pm->m_frame_when_rendered == m_counter)
		{
			continue;
		}

		pm->m_frame_when_rendered = m_counter;

		OverlapResult result = OVERLAP( planes, num, &pm->m_aabb);

		if( result != OVERLAP_OUTSIDE)
		{
			visible_planar_maps.push_back( pm);
		}
	}

	for( KCL::uint32 i=0; i<r->m_connections.size(); i++)
	{
		XRoomConnection *rc = r->m_connections[i];

		if( portals_enabled && rc->m_portal)
		{
			std::vector<Vector3D> new_points1;
			std::vector<Vector3D> new_points2;
			std::vector<Vector4D> new_planes;
			XPortal *p = rc->m_portal;

			XRoom *other = rc->m_portal->Other( r);

			if( IsRoomInPath( other))
			{
				continue;
			}

			new_points1 = p->m_points;

			for( KCL::uint32 k=0; k<num; k++)
			{
				const Vector4D &plane = planes[k];

				for( KCL::uint32 j=0; j<new_points1.size(); j++)
				{
					Vector3D &P0 = new_points1[j];
					Vector3D &P1 = new_points1[j == new_points1.size() - 1 ? 0 : j + 1];

					if( Inside( P0, plane))
					{
						new_points2.push_back( P0);
						if( !Inside( P1, plane))
						{
							Vector3D rr = Intersect( P0, P1, plane);
							new_points2.push_back( rr);
						}
					}
					else
					{
						if( Inside( P1, plane))
						{
							Vector3D rr = Intersect( P0, P1, plane);
							new_points2.push_back( rr);
						}
					}
				}

				new_points1 = new_points2;
				new_points2.clear();
			}

			float eye_side = p->m_plane.x * camera->GetEye().x + p->m_plane.y * camera->GetEye().y + p->m_plane.z * camera->GetEye().z + p->m_plane.w;

			for( KCL::uint32 j=0; j<new_points1.size(); j++)
			{
				Vector4D new_plane;
				XPortal::Edge e;
				Vector3D &P0 = new_points1[j];
				Vector3D &P1 = new_points1[j == new_points1.size() - 1 ? 0 : j + 1];

				e.CreateEdgeVector( P1, P0);
				e.CreatePlane( eye_side > 0.0f, camera->GetEye(), P0, new_plane);

				new_planes.push_back( new_plane);
			}

			if( new_planes.size())
			{
				FrustumCull2( visible_rooms, visible_planar_maps, visible_meshes, visible_mios, other, camera, &new_planes[0], (int)new_planes.size(), camera_room, pvs, culling_algorithm);
			}
		}
		else
		{
			XRoom *other = rc->Other( r);

			if( IsRoomInPath( other))
			{
				continue;
			}

			if( pvs)
			{
				if( !pvs[camera_room->m_pvs_index][other->m_pvs_index])
				{
					continue;
				}
			}

            OverlapResult result = OVERLAP( planes, num, &other->m_aabb);
			if( result != OVERLAP_OUTSIDE)
			{
				FrustumCull2( visible_rooms, visible_planar_maps, visible_meshes, visible_mios, other, camera, planes, num, camera_room, pvs, culling_algorithm);
			}
		}
	}

	s_frustum_cull_path.pop_back();
}


void KCL::XRoom::Query( std::vector<Mesh*> &result, XRoom* room, const AABB& aabb)
{
	if( !room)
	{
		return;
	}
	bool actual_cache_index = 0;
	std::vector<XRoom*> cache[2];

	cache[0].push_back( room);

	while( cache[actual_cache_index].size())
	{
		for( KCL::uint32 i=0; i<cache[actual_cache_index].size(); i++)
		{
			XRoom *n = cache[actual_cache_index][i];

			if( n->m_frame_when_touched == m_counter)
			{
				continue;
			}

			if( !AABB::Intersect( aabb, n->m_aabb))
			{
				continue;
			}

			for( KCL::uint32 j=0; j<n->m_meshes.size(); j++)
			{
				Mesh* m = n->m_meshes[j];

				if( AABB::Intersect( aabb, m->m_aabb))
				{
					result.push_back( m);
				}
			}

			n->m_frame_when_touched = m_counter;

			for( KCL::uint32 j=0; j<n->m_connections.size(); j++)
			{
				XRoom *nn = n->m_connections[j]->Other( n);

				cache[!actual_cache_index].push_back( nn);
			}
		}
		cache[actual_cache_index].clear();
		actual_cache_index = !actual_cache_index;
	}
	m_counter++;
}


void KCL::XRoom::Query( std::vector<XRoom*> &result, XRoom* room, const AABB& aabb)
{
	if( !room)
	{
		return;
	}
	bool actual_cache_index = 0;
	std::vector<XRoom*> cache[2];

	cache[0].push_back( room);

	while( cache[actual_cache_index].size())
	{
		for( KCL::uint32 i=0; i<cache[actual_cache_index].size(); i++)
		{
			XRoom *n = cache[actual_cache_index][i];

			if( n->m_frame_when_touched == m_counter)
			{
				continue;
			}

			if( !AABB::Intersect( aabb, n->m_aabb))
			{
				continue;
			}

			result.push_back( n);

			n->m_frame_when_touched = m_counter;

			for( KCL::uint32 j=0; j<n->m_connections.size(); j++)
			{
				XRoom *nn = n->m_connections[j]->Other( n);

				cache[!actual_cache_index].push_back( nn);
			}
		}
		cache[actual_cache_index].clear();
		actual_cache_index = !actual_cache_index;
	}
	m_counter++;
}


void KCL::XRoom::EnablePortals( bool yes)
{
	portals_enabled = yes;
}


OverlapResult KCL::XRoom::OVERLAP( const KCL::Vector4D *planes, KCL::uint32 num, KCL::AABB *aabb)
{
	KCL::Vector3D vmin, vmax;
    OverlapResult result = OVERLAP_INSIDE;
	const KCL::Vector4D *plane = planes;
	KCL::int32 i = num;

	while(i--)
	{
		float d;

		if( plane->x > 0)
		{
			vmin.x = aabb->GetMinVertex().x;
			vmax.x = aabb->GetMaxVertex().x;
		}
		else
		{
			vmin.x = aabb->GetMaxVertex().x;
			vmax.x = aabb->GetMinVertex().x;
		}
		if( plane->y > 0)
		{
			vmin.y = aabb->GetMinVertex().y;
			vmax.y = aabb->GetMaxVertex().y;
		}
		else
		{
			vmin.y = aabb->GetMaxVertex().y;
			vmax.y = aabb->GetMinVertex().y;
		}
		if( plane->z > 0)
		{
			vmin.z = aabb->GetMinVertex().z;
			vmax.z = aabb->GetMaxVertex().z;
		}
		else
		{
			vmin.z = aabb->GetMaxVertex().z;
			vmax.z = aabb->GetMinVertex().z;
		}

		d = plane->x * vmin.x + plane->y * vmin.y + plane->z * vmin.z + plane->w;
		if( d>0)
		{
            return OVERLAP_OUTSIDE;
		}

		d = plane->x * vmax.x + plane->y * vmax.y + plane->z * vmax.z + plane->w;
		if( d>=0)
		{
			//NOTE: nem szabad itt return, mert meg kesobb lehet outside
            result = OVERLAP_INTERSECT;
		}
		plane++;
	}
	return result;
}


OverlapResult KCL::XRoom::OVERLAP(const KCL::Vector4D *planes, KCL::uint32 num, const KCL::Vector3D &pos)
{
	OverlapResult result = OVERLAP_INSIDE;
	const KCL::Vector4D *plane = planes;
	KCL::int32 i = num;

	while (i--)
	{
		float d = plane->x * pos.x + plane->y * pos.y + plane->z * pos.z + plane->w;
		if (d>0.0f)
		{
			return OVERLAP_OUTSIDE;
		}
		if (d == 0.0f)
		{
			//NOTE: nem szabad itt return, mert meg kesobb lehet outside
			result = OVERLAP_INTERSECT;
		}
		plane++;
	}
	return result;
}


bool IsRoomInPath( KCL::XRoom *r)
{
	for( KCL::uint32 i=0; i<s_frustum_cull_path.size(); i++)
	{
		if( s_frustum_cull_path[i] == r)
		{
			return true;
		}
	}
	return false;
}
