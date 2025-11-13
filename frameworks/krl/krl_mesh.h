/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KRL_MESH_H
#define KRL_MESH_H


#include <kcl_base.h>
#include <kcl_mesh.h>

#include <map>
#include <set>

namespace KRL
{
	class Material;

	class Mesh3 : public KCL::Mesh3
	{
	public:
		struct InstanceData
		{
			KCL::Matrix4x4 mv;
			KCL::Matrix4x4 inv_mv;
		};
		std::map<KCL::Material*, std::vector<InstanceData> > m_instances;
		std::set<KCL::Material*> m_is_rendered;
		bool m_is_instance_data_updated;
	protected:
		Mesh3(const char *name) : KCL::Mesh3(name) {}
	};
}

#endif
