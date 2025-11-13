/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_room_setup.h"
#include "gfxb_scene5.h"

#include "common/gfxb_mesh.h"
#include "common/gfxb_tools.h"

#include "kcl_envprobe.h"

#include <vector>
#include <set>

using namespace GFXB;

#define VERBOSE_INIT 0

void RoomSetup::SetupRooms(Scene5 *scene)
{
	std::vector<KCL::XRoom*> &rooms = scene->m_rooms;
	std::vector<std::vector<KCL::Mesh*>> &part_meshes = scene->GetPartitionMeshes();

	if (rooms.empty() == false)
	{
		for (size_t i = 0; i < rooms[0]->m_meshes.size(); i++)
		{
			((Mesh*)rooms[0]->m_meshes[i])->CalculateCenter();
		}
	}

	if (rooms.size() < 2 || part_meshes.empty())
	{
		return;
	}

	// Build up the connections
	for (size_t room_id = 1; room_id < rooms.size() - 1; room_id++)
	{
		for (size_t mesh_id = 0; mesh_id < part_meshes[room_id].size(); mesh_id++)
		{
			KCL::Mesh *mesh = part_meshes[room_id][mesh_id];

			for (size_t room_id2 = room_id + 1; room_id2 < rooms.size(); room_id2++)
			{
				KCL::Mesh *conn_mesh = FindConnection(part_meshes[room_id2], mesh);
				if (conn_mesh == nullptr)
				{
					continue;
				}

				KCL::XRoom *room1 = rooms[room_id];
				KCL::XRoom *room2 = rooms[room_id2];

				KCL::XRoomConnection *connection = new KCL::XRoomConnection(room1, room2);

				scene->m_room_connections.push_back(connection);

				room1->m_connections.push_back(connection);
				room2->m_connections.push_back(connection);

				bool is_portal = mesh->m_name.find("portal") != std::string::npos;
				if (is_portal)
				{
					// Create the portal
					KCL::XPortal *portal = new KCL::XPortal(mesh->m_name.c_str());
					scene->m_portals.push_back(portal);

					for (size_t i = 0; i < mesh->m_mesh->m_vertex_attribs3[0].size(); i++)
					{
						portal->m_points.push_back(mesh->m_mesh->m_vertex_attribs3[0][i]);
					}

					portal->Init();

					connection->m_portal = portal;
					portal->ConnectRooms(room1, room2);
				}
			}
		}
	}

#if VERBOSE_INIT
	for (size_t room_id = 1; room_id < rooms.size() - 1; room_id++)
	{
		INFO("%s", rooms[room_id]->m_name.c_str());
		for (size_t conn_id = 0; conn_id < rooms[room_id]->m_connections.size(); conn_id++)
		{
			KCL::XRoomConnection *conn = rooms[room_id]->m_connections[conn_id];
			INFO("%s -> %s %s", conn->m_a->m_name.c_str(), conn->m_b->m_name.c_str(), conn->m_portal != nullptr ? "portal" : "visible");
		}
	}
#endif

	// Create the partition planes
	std::vector<std::set<KCL::Vector4D>> plane_set;
	plane_set.resize(rooms.size());
	for (size_t room_id = 1; room_id < rooms.size(); room_id++)
	{
		for (size_t mesh_id = 0; mesh_id < part_meshes[room_id].size(); mesh_id++)
		{
			KCL::Mesh3 *mesh3 = part_meshes[room_id][mesh_id]->m_mesh;

			size_t index_count = mesh3->m_vertex_indices[0].size();
			for (size_t index_id = 0; index_id < index_count; index_id = index_id + 3)
			{
				KCL::uint16 index0 = mesh3->m_vertex_indices[0][index_id + 0];
				KCL::uint16 index1 = mesh3->m_vertex_indices[0][index_id + 1];
				KCL::uint16 index2 = mesh3->m_vertex_indices[0][index_id + 2];

				const KCL::Vector3D &v0 = mesh3->m_vertex_attribs3[0][index0];
				const KCL::Vector3D &v1 = mesh3->m_vertex_attribs3[0][index1];
				const KCL::Vector3D &v2 = mesh3->m_vertex_attribs3[0][index2];

				KCL::Vector3D n = KCL::Vector3D::cross(v1 - v0, v2 - v0);
				n.normalize();

				float d = -KCL::Vector3D::dot(n, v1);
				KCL::Vector4D plane(n, d);

				plane_set[room_id].insert(-plane);
			}
		}
	}


	for (size_t room_id = 1; room_id < rooms.size(); room_id++)
	{
		for (auto it = plane_set[room_id].begin(); it != plane_set[room_id].end(); it++)
		{
			rooms[room_id]->m_planes.push_back(*it);
		}

		if (rooms[room_id]->m_planes.empty())
		{
			INFO("Warning! Room does not have cull planes: %s", rooms[room_id]->m_name.c_str());
		}
	}


	// Put the meshes to the rooms
	for (size_t mesh_id = 0; mesh_id < rooms[0]->m_meshes.size(); mesh_id++)
	{
		KCL::Mesh *mesh = rooms[0]->m_meshes[mesh_id];

		for (size_t room_id = 1; room_id < rooms.size(); room_id++)
		{
			KCL::XRoom *room = rooms[room_id];
			const std::vector<KCL::Vector4D> &room_planes = room->m_planes;

			if (room_planes.empty())
			{
				continue;
			}

			KCL::OverlapResult overlap_result = KCL::XRoom::OVERLAP(room_planes.data(), (KCL::uint32)room_planes.size(), &mesh->m_aabb);
			if (overlap_result != KCL::OVERLAP_OUTSIDE)
			{
				rooms[room_id]->m_meshes.push_back(mesh);
			}
		}
	}

	// PVS
	SetupPVS(scene);

#if VERBOSE_INIT
	INFO("Rooms:");
	for (size_t i = 0; i < rooms.size(); i++)
	{
		INFO("Room: %s - mesh count: %d", rooms[i]->m_name.c_str(), (int)rooms[i]->m_meshes.size());
	}
#endif
}


void RoomSetup::GetVertices(const KCL::Mesh3 *mesh3, std::vector<KCL::Vector3D> &vertices)
{
	size_t index_count = mesh3->m_vertex_indices[0].size();

	vertices.clear();
	vertices.reserve(index_count);
	for (size_t index_id = 0; index_id < index_count; index_id = index_id + 3)
	{
		KCL::uint16 index0 = mesh3->m_vertex_indices[0][index_id + 0];
		KCL::uint16 index1 = mesh3->m_vertex_indices[0][index_id + 1];
		KCL::uint16 index2 = mesh3->m_vertex_indices[0][index_id + 2];

		vertices.push_back(mesh3->m_vertex_attribs3[0][index0]);
		vertices.push_back(mesh3->m_vertex_attribs3[0][index1]);
		vertices.push_back(mesh3->m_vertex_attribs3[0][index2]);
	}
}


KCL::Mesh *RoomSetup::FindConnection(const std::vector<KCL::Mesh*> &partition_meshes, const KCL::Mesh *mesh)
{
	if (mesh->m_name.find("unvisible") != std::string::npos)
	{
		// unvisible plane has no connection...
		return nullptr;
	}

	std::vector<std::string> submeshes;
	Tools::SplitString(mesh->m_name, '|', submeshes);

	if (submeshes.size() < 2)
	{
		INFO("RoomSetup::FindConnection - Illegal mesh name: %s", mesh->m_name.c_str());
		return nullptr;
	}

	std::string name_to_find = submeshes[submeshes.size() - 2];

	for (size_t i = 0; i < partition_meshes.size(); i++)
	{
		KCL::Mesh *test_mesh = partition_meshes[i];

		if (test_mesh->m_name.find("unvisible") != std::string::npos)
		{
			continue;
		}

		Tools::SplitString(test_mesh->m_name, '|', submeshes);

		if (submeshes.size() < 2)
		{
			INFO("RoomSetup::FindConnection - Illegal mesh name: %s", test_mesh->m_name.c_str());
			continue;
		}

		if (submeshes[submeshes.size() - 2] == name_to_find)
		{
			return test_mesh;
		}
	}

	return nullptr;
}


void RoomSetup::SetupPVS(Scene5 *scene)
{
	// Reset the PVS matrix
	ResetPVS(scene);

	char buff[BUFF_SIZE] = { 0 };

	KCL::AssetFile pvs_file("pvs.txt");

	if (pvs_file.Opened())
	{
		while (!pvs_file.eof())
		{
			pvs_file.Gets(buff, BUFF_SIZE);

			std::stringstream ss;

			std::string command;
			std::string room_name0;
			std::string room_name1;

			bool connect_value = false;

			ss << buff;

			ss >> command;

			if (command.empty() || command.find("//") == 0)
			{
				// Basic comment support
				continue;
			}

			if (command == "reset")
			{
				INFO("Reset PVS");

				ResetPVS(scene);
				continue;
			}

			if (command == "connect")
			{
				ss >> room_name0;

				if (room_name0 == "all")
				{
					INFO("Connect all rooms");

					ClearPVS(scene, true);
					continue;
				}

				ss >> room_name1;

				connect_value = true;
			}
			else if (command == "disconnect")
			{
				ss >> room_name0;

				if (room_name0 == "all")
				{
					INFO("Disconnect all rooms");

					ClearPVS(scene, false);
					continue;
				}

				ss >> room_name1;

				connect_value = false;
			}

			KCL::XRoom *room0 = FindRoom(scene, room_name0);

			KCL::XRoom *room1 = FindRoom(scene, room_name1);

			if (!room0)
			{
				INFO("RoomSetup::SetupPVS - Can not find room: %s", room_name0.c_str());
				continue;
			}

			if (!room1)
			{
				INFO("RoomSetup::SetupPVS - Can not find room: %s", room_name1.c_str());
				continue;
			}

#if VERBOSE_INIT
			INFO("%s rooms: %s - %s", connect_value ? "Connect" : "Disconnect", room_name0.c_str(), room_name1.c_str());
#endif

			scene->m_pvs[room0->m_pvs_index][room1->m_pvs_index] = connect_value;

			scene->m_pvs[room1->m_pvs_index][room0->m_pvs_index] = connect_value;
		}
	}
	else
	{
		INFO("RoomSetup::SetupPVS - Can not open pvs.txt!");

		ClearPVS(scene, true);
	}

#if VERBOSE_INIT
	// Print the layout of the rooms
	std::vector<KCL::XRoom*> &rooms = scene->m_rooms;

	std::stringstream ss;
	ss << ' ';
	for (size_t i = 0; i < rooms.size(); i++)
	{
		ss << ' ' << i;
	}

	INFO("%s", ss.str().c_str());

	for (size_t i = 0; i < rooms.size(); i++)
	{
		ss.str("");
		ss << i;

		for (size_t j = 0; j < rooms.size(); j++)
		{
			ss << ' ' << (scene->m_pvs[rooms[i]->m_pvs_index][rooms[j]->m_pvs_index] ? '+' : '-');
		}

		INFO("%s", ss.str().c_str());
	}
#endif
}


void RoomSetup::ResetPVS(Scene5 *scene)
{
	std::vector<KCL::XRoom*> &rooms = scene->m_rooms;

	for (size_t i = 0; i < rooms.size(); i++)
	{
		for (size_t j = 0; j < rooms.size(); j++)
		{
			KCL::XRoom *room0 = rooms[i];

			KCL::XRoom *room1 = rooms[j];

			scene->m_pvs[room0->m_pvs_index][room1->m_pvs_index] = (i == j);
		}
	}
}


void RoomSetup::ClearPVS(Scene5 *scene, bool clear_value)
{
	std::vector<KCL::XRoom*> &rooms = scene->m_rooms;

	for (size_t i = 0; i < rooms.size(); i++)
	{
		for (size_t j = 0; j < rooms.size(); j++)
		{
			KCL::XRoom *room0 = rooms[i];

			KCL::XRoom *room1 = rooms[j];

			scene->m_pvs[room0->m_pvs_index][room1->m_pvs_index] = clear_value;
		}
	}
}


KCL::XRoom *RoomSetup::FindRoom(Scene5 *scene, const std::string &name)
{
	for (size_t i = 0; i < scene->m_rooms.size(); i++)
	{
		if (scene->m_rooms[i]->m_name == name)
		{
			return scene->m_rooms[i];
		}
	}

	return nullptr;
}


void RoomSetup::PlaceProbes(Scene5 *scene)
{
	std::vector<KCL::XRoom*> &rooms = scene->m_rooms;

	if (rooms.empty())
	{
		return;
	}

	for (size_t i = 0; i < rooms.size(); i++)
	{
		rooms[i]->m_probes.clear();
	}

	rooms[0]->m_probes = scene->m_probes;

	{
		for (size_t probe_id = 0; probe_id < scene->m_probes.size(); probe_id++)
		{
			KCL::EnvProbe *probe = scene->m_probes[probe_id];

			for (size_t room_id = 1; room_id < rooms.size(); room_id++)
			{
				KCL::XRoom *room = rooms[room_id];

				const std::vector<KCL::Vector4D> &room_planes = room->m_planes;

				if (room_planes.empty())
				{
					continue;
				}

				KCL::OverlapResult overlap_result = KCL::XRoom::OVERLAP(room_planes.data(), (KCL::uint32)room_planes.size(), &probe->m_aabb);

				if (overlap_result != KCL::OVERLAP_OUTSIDE)
				{
					rooms[room_id]->m_probes.push_back(probe);
				}
			}
		}
	}

#if VERBOSE_INIT
	for (size_t i = 0; i < rooms.size(); i++)
	{
		INFO("Room: %s - probe count: %d", rooms[i]->m_name.c_str(), (int)rooms[i]->m_probes.size());
	}
#endif
}


void RoomSetup::CleanUp(Scene5 *scene)
{
	// Remove meshes from outher rooms as SceneHandler destroyes them from Room0
	for (size_t i = 1; i < scene->m_rooms.size(); i++)
	{
		scene->m_rooms[i]->m_meshes.clear();
	}
}