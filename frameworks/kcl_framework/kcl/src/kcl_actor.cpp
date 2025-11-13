/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_actor.h>
#include "kcl_particlesystem2.h"

using namespace KCL;

KCL::Actor::Actor( const std::string &name, bool shadow_caster): Object( name, ACTOR), m_is_shadow_caster( shadow_caster), m_shader_track( 0), m_flags(0)
{
	m_root = 0;
	m_aabb_bias.set( 0.1f, 0.1f, 0.1f);
	m_aabb_scale.set( 1.2f, 1.2f, 1.2f);
}


KCL::Actor::~Actor()
{
	delete m_shader_track;
	delete m_root;
}

void KCL::Actor::AddObject(KCL::Object* obj)
{
	if( obj->m_type == MESH)
	{
		m_meshes.push_back( (Mesh*)obj);
	}
	else if( obj->m_type == LIGHT)
	{
		m_lights.push_back( (Light*)obj);
	}
	else if (obj->m_type == EMITTER1 || obj->m_type == EMITTER2 || obj->m_type == EMITTER4 || obj->m_type == EMITTER5)
	{
		m_emitters.push_back( (Node*)obj);
	}
}


void KCL::Actor::RemoveObject( KCL::Object* obj )
{
	if(obj->m_type == KCL::LIGHT)
	{
		KCL::Light* l = static_cast<KCL::Light*>(obj);

		std::vector<KCL::Light*>::iterator it = m_lights.begin();
		std::vector<KCL::Light*>::iterator itEnd = m_lights.end();
		while(it != itEnd)
		{
			if(*it == l)
			{
				m_lights.erase(it);
				break;
			}
			++it;
		}
	}
	else if(obj->m_type == KCL::MESH)
	{
		KCL::Mesh* m = static_cast<KCL::Mesh*>(obj);

		std::vector<KCL::Mesh*>::iterator it = m_meshes.begin();
		std::vector<KCL::Mesh*>::iterator itEnd = m_meshes.end();
		while(it != itEnd)
		{
			if(*it == m)
			{
				m_meshes.erase(it);
				break;
			}
			++it;
		}
	}
#if 0
	else if(obj->m_type == KCL::EMITTER)
	{
		KCL::AnimatedEmitter* m = static_cast<KCL::AnimatedEmitter*>(obj);

		std::vector<KCL::AnimatedEmitter*>::iterator it = m_emitters.begin();
		std::vector<KCL::AnimatedEmitter*>::iterator itEnd = m_emitters.end();
		while(it != itEnd)
		{
			if(*it == m)
			{
				m_emitters.erase(it);
				break;
			}
			++it;
		}
	}
#endif
}

void KCL::Actor::CalculateAABB()
{
	if(!m_root)
	{
		return;
	}

	bool actual_cache_index = 0;
	std::vector<Node*> cache[2];

	m_aabb.Reset();

	cache[0].push_back( m_root);

	while( cache[actual_cache_index].size())
	{
		for( KCL::uint32 i=0; i<cache[actual_cache_index].size(); i++)
		{
			Node *n = cache[actual_cache_index][i];

			KCL::Vector3D v( &n->m_world_pom.v[12]);

			if( v.length2() > 1.0f)
			{
//TODO: may cause performance change -> visibility change
				//if( m_skinned_meshes.size())
				{
					if( n->m_type != MESH)
					{
						m_aabb.Merge( KCL::Vector3D( n->m_world_pom.v[12], n->m_world_pom.v[13], n->m_world_pom.v[14]));
					}
				}
				//else
				{
					if( n->m_position_track || n->m_scaleTrack || n->m_name.find("BB") != std::string::npos)
					{
						m_aabb.Merge( KCL::Vector3D( n->m_world_pom.v[12], n->m_world_pom.v[13], n->m_world_pom.v[14]));
					}
				}

				if( m_name.find( "cave") != std::string::npos || m_name.find( "decal") != std::string::npos)
				{
					m_aabb.Merge( KCL::Vector3D( n->m_world_pom.v[12], n->m_world_pom.v[13], n->m_world_pom.v[14]));
				}
			}

			if( m_name.find( "track") != std::string::npos)
			{
				m_aabb.Merge( KCL::Vector3D( n->m_world_pom.v[12], n->m_world_pom.v[13], n->m_world_pom.v[14]));
			}

			for( KCL::uint32 j=0; j<n->m_children.size(); j++)
			{
				Node *nn = n->m_children[j];

				cache[!actual_cache_index].push_back( nn);
			}
		}
		cache[actual_cache_index].clear();
		actual_cache_index = !actual_cache_index;
	}
	m_aabb.BiasAndScale( m_aabb_bias, m_aabb_scale);
}


void KCL::Actor::ComplementFire(KCL::FactoryBase *f)
{
	std::vector<Node*> fires;

	Node::SearchNode( fires, m_root, EMITTER2);

	for( uint32 i=0;  i<fires.size(); i++)
	{
		_emitter *fire = (_emitter*)fires[i];

#if 0
		{
			_emitter *smoke = new _emitter();
			smoke->m_name = "smoke";
			smoke->m_local_pom.v[13] = 7;//emitter->m_focus_distance - 1;
			smoke->InitAsSmoke();

			if( fire->m_spawning_rate_animated)
			{
				std::string fn = path + std::string( "animations/" + fire->m_rate_track_name);

				AssetFile f(fn);
				if( !f.GetLastError())
				{
					_key_node::Read( smoke->m_spawning_rate_animated, f);
					f.Close();
				}
			}

			m_emitters.push_back( smoke);
		}
#endif

		{
			assert(f);
			_emitter *soot = (KCL::_emitter*)f->Create( "soot", fire, this);
			soot->InitAsSoot();

			if( fire->HasRateAnimation())
			{
				AssetFile f(std::string("animations/" + fire->m_rate_track_name));
				if( f.Opened())
				{
					_key_node* spawning_rate_animated = 0;
					
					_key_node::Read( spawning_rate_animated, f);

					soot->Set_Spawning_rate_animated( spawning_rate_animated);

					f.Close();
				}
			}
		}
	}
}
