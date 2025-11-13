/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_ACTOR_H
#define KCL_ACTOR_H

#include <kcl_aabb.h>
#include <kcl_node.h>
#include <kcl_light2.h>
#include <kcl_mesh.h>
#include <kcl_particlesystem.h>

#include <vector>
#include <string>

namespace KCL
{
	class Actor : public Object
	{
	public:
		Vector3D m_aabb_bias;
		Vector3D m_aabb_scale;
		AABB m_aabb;
		Node *m_root;
		bool m_is_shadow_caster;
		_key_node *m_shader_track;

        KCL::uint32 m_flags; //to restrict rendering for some passes, etc.

		std::vector<Mesh*> m_meshes;
		std::vector<Light*> m_lights;
		std::vector<KCL::Node*> m_emitters;

		int material_idx;
		Actor( const std::string &name, bool shadow_caster);
		~Actor();
		void CalculateAABB();
		void AddObject(KCL::Object* obj);
		void RemoveObject(KCL::Object* obj);
		void ComplementFire(KCL::FactoryBase *f);

	};
}


#endif
