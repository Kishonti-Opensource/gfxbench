/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_XROOM_H
#define KCL_XROOM_H

#include <kcl_base.h>
#include <kcl_aabb.h>
#include <kcl_camera2.h>
#include <kcl_mesh.h>
#include <kcl_planarmap.h>

#include <string>
#include <vector>

/*class Camera2;
struct Mesh;
struct XRoom;
class PlanarMap;*/

namespace KCL
{
	class XRoom; // forward
	class XPortal;
	struct MeshInstanceOwner2;
    class CullingAlgorithm;
	class EnvProbe;

    // Results of the OVERLAP function
    enum OverlapResult
    {
        OVERLAP_OUTSIDE = -1,
        OVERLAP_INTERSECT = 0,
        OVERLAP_INSIDE = 1,
    };


	class XRoomConnection
	{
	public:
		XRoom *m_a;
		XRoom *m_b;
		XPortal *m_portal;
		bool m_enabled;

		XRoomConnection( XRoom *a,XRoom *b) : m_a( a), m_b( b), m_portal( 0), m_enabled( true)
		{
		}

		XRoom* Other( XRoom* r)
		{
			return r == m_a ? m_b : m_a;
		}

		void AddPortal();
	};


	class XPortal
	{
	public:
		struct Edge
		{
			KCL::Vector3D m_edge_vector;
			void CreateEdgeVector( const KCL::Vector3D &P1, const KCL::Vector3D &P0);
			void CreatePlane( bool eye_side, const KCL::Vector3D &eye, const KCL::Vector3D &point, KCL::Vector4D &plane);
		};

		std::string m_name;
		std::vector<KCL::Vector3D> m_points;
		KCL::Vector4D m_plane;

		//TODO: integrate with XRoomConnection
		union
		{
			XRoom *m_rooms[2];
			struct
			{
				XRoom *m_front;
				XRoom *m_back;
			};
		};

		XPortal( const char* name);
		void Init();
		void ConnectRooms( XRoom *r0, XRoom *r1);
		XRoom* Other( XRoom* r)
		{
			return r == m_front ? m_back : m_front;
		}
	};


	class XRoom : public Object
	{
	public:
		KCL::uint32 m_pvs_index;
		std::vector<Mesh*> m_meshes;
		std::vector<Actor*> m_actors;
		std::vector<EnvProbe*> m_probes;
		std::vector<PlanarMap*> m_planar_maps;
		std::vector<XRoomConnection*> m_connections;
		std::vector<KCL::Vector4D> m_planes;

		AABB m_aabb;
		KCL::uint32 m_frame_when_touched;

		static KCL::uint32 m_counter;
		XRoom( const char* name);
		~XRoom();

		static void EnablePortals( bool yes);
		static void FrustumCull( std::vector<XRoom*> &visible_rooms, std::vector<PlanarMap*> &visible_planar_maps, std::vector<Mesh*> visible_meshes[3], std::vector< std::vector<KCL::Mesh*> > &visible_instances, XRoom* r, Camera2* c, const KCL::Vector4D *planes, KCL::uint32 num, XRoom* camera_room, bool **pvs, CullingAlgorithm *screenSizeLimit);
		static void FrustumCull2( std::vector<XRoom*> &visible_rooms, std::vector<PlanarMap*> &visible_planar_maps, std::vector<Mesh*> visible_meshes[3], std::vector<KCL::MeshInstanceOwner2*> &visible_mios, XRoom* r, Camera2* c, const KCL::Vector4D *planes, KCL::uint32 num, XRoom* camera_room, bool **pvs, CullingAlgorithm *screenSizeLimit);
		static void Query( std::vector<XRoom*> &result, XRoom* room, const AABB& aabb);
		static void Query( std::vector<Mesh*> &result, XRoom* room, const AABB& aabb);
        static OverlapResult OVERLAP( const KCL::Vector4D *planes, KCL::uint32 num, KCL::AABB *aabb);
		static OverlapResult OVERLAP(const KCL::Vector4D *planes, KCL::uint32 num, const KCL::Vector3D &pos);
	};
}

#endif
