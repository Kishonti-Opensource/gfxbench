/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_node.h>

#include <kcl_aabb.h>
#include <kcl_animation4.h>
#include <kcl_light2.h>
#include <kcl_actor.h>

using namespace KCL;

KCL::Node::Node( const std::string &name, ObjectType type, Node* parent, Object *owner) : Object( name, type), m_owner( owner)
{
	m_parent = 0;
	m_scaleTrack = 0;
	m_translation.set( 0,0,0);
	m_scale.set( 1,1,1);
	m_angle = 0;
	m_visible = true;
	m_position_track = 0;
	m_orientation_track = 0;
	m_visibility_track = 0;
	m_alpha_track = 0;
	m_secondary_anim_control_track = 0;
	m_alpha = 0.0f;
	m_weight = 0.0f;
	m_secondary_anim_control = 0.0f;
	m_enable_animation = true;
	m_secondary_animation_enabled = false;
	m_secondary_anim_control_begin_time = 0.0f;

	int seed = 0;
	size_t len = m_name.length();
	for (size_t i = 0; i < len; i++) {
		seed = 31 * seed + m_name[len - i - 1];
        seed %= 1000000;
	}

	m_secondary_animation_radius_factor = 1.0f + 1.0f * Math::randomf(&seed);
	m_secondary_animation_radius_threshold = 1.f + 3.0f * Math::randomf(&seed);
	m_secondary_animation_rotation_factor = 2.f + 2.0f * Math::randomf(&seed);

	m_secondary_animation_rotation_axis.x = -1.f + 2.0f * Math::randomf(&seed);
	m_secondary_animation_rotation_axis.y = -1.f + 2.0f * Math::randomf(&seed);
	m_secondary_animation_rotation_axis.z = -1.f + 2.0f * Math::randomf(&seed);
	m_secondary_animation_rotation_axis.normalize();

	if( parent)
	{
		parent->m_children.push_back( this);
		m_parent = parent;
		m_guid = parent->m_guid + "|" + m_guid;
	}
	if (parent == 0 && m_owner)//xactor's name
	{
		//m_guid = m_owner->GetGuid();
	}

	if( m_owner)
	{
		if( m_owner->m_type == ACTOR)
		{
			m_owner_actor->AddObject( this);
		}
		if( m_owner->m_type == ROOM)
		{
		}
	}
}


KCL::Node::~Node()
{
	delete m_position_track;
	delete m_orientation_track;
	delete m_scaleTrack;
	delete m_visibility_track;
	delete m_alpha_track;
	delete m_secondary_anim_control_track;

	if( m_owner)
	{
		if( m_owner->m_type == ACTOR)
		{
			m_owner_actor->RemoveObject( this);
		}
	}

	while( m_children.size())
	{
		KCL::Node *n = m_children[0];
		RemoveChild( n);
		delete n;
	}
}


bool KCL::Node::LoadPositionTrack()
{
	return KCL::ReadAnimation(m_position_track, m_name + std::string("_position_track"));
}


bool KCL::Node::LoadOrientationTrack()
{
	return KCL::ReadAnimation(m_orientation_track, m_name + std::string("_orientation_track"));
}


bool Node::LoadVisibilityTrack()
{
	return ReadAnimation(m_visibility_track, m_name + std::string("_visibility_track"));
}


bool Node::LoadAlphaTrack()
{
	return ReadAnimation(m_alpha_track, m_name + std::string("_alpha_track"));
}


bool Node::LoadSecondaryAnimCcontrolTrack()
{
	bool b = ReadAnimation(m_secondary_anim_control_track, m_name + std::string("_secondary_anim_control_track"));

	if (b)
	{
		Vector4D v;
		float tb = 0.0f;

		while (1)
		{
			float i = _key_node::Get(v, m_secondary_anim_control_track, m_secondary_anim_control_begin_time, tb);
			
			if (v.x > 0.0f)
			{
				break;
			}

			if (i == -1.0f)
			{
				break;
			}

			m_secondary_anim_control_begin_time += 1.0f / 25.0f;
		}
	}

	return b;
}


std::string KCL::Node::GetParameterFilename() const
{
	std::string fname;

	switch (m_type)
	{
		case KCL::LIGHT:
			fname = "lights/";
			break;
		case KCL::MATERIAL:
			fname = "materials/";//TODO engine_materials???
			break;
		case KCL::MESH:
			fname = "meshes/";
			break;
		case KCL::EMITTER1:
		case KCL::EMITTER2:
		case KCL::EMITTER4:
		case KCL::EMITTER5:
			fname = "emitters/";
			break;
        case KCL::UNKNOWN:
        case KCL::ACTOR:
        case KCL::NODE:
        case KCL::EFFECT:
        case KCL::KEYFRAMESEQUENCE:
        case KCL::ANIMATIONTRACK:
        case KCL::ROOM:
            break;
	}
	return fname + m_name;
}


void KCL::Node::Serialize(JsonSerializer& s)
{
	Object::Serialize(s);
}


void KCL::Node::animate (bool prev, AnimateParameter &par)
{
	KCL::Matrix4x4 local = m_local_pom;

	if (m_scaleTrack && m_scaleTrack->isAnimated (par.time) != KeyframeSequence::PRE_ANIMATION)
	{
		KCL::Vector3D scale = m_scaleTrack->getScaleKeyframe (par.time);
		local.scale (scale);
	}
	else
	{
		local.scale (m_scale);
	}

	if( m_visibility_track)
	{
		Vector4D vis;
		float t = par.time / 1000.0f;
		float tb = 0.0f;


		_key_node::Get( vis, m_visibility_track, t, tb);

		m_visible = vis.x > 0.9f;
	}
	else
	{
		m_visible = par.visible;

		if( m_type == LIGHT)
		{
			Light *l = (Light *)this;
			if( l->m_is_flickering)
			{
				static int s = 0;
				float f = Math::randomf( &s);
				if( f < 0.25f)
				{
					m_visible = false;
				}
				else
				{
					m_visible = true;
				}
			}
		}
	}
	if( m_alpha_track)
	{
		Vector4D v;
		float t = par.time / 1000.0f;
		float tb = 0.0f;

		_key_node::Get( v, m_alpha_track, t, tb);

		m_alpha = v.x;
	}

	if (m_secondary_anim_control_track)
	{
		Vector4D v;
		float t = par.time / 1000.0f;
		float tb = 0.0f;

		_key_node::Get(v, m_secondary_anim_control_track, t, tb);

		m_secondary_anim_control = v.x * (par.time / 1000.0f - m_secondary_anim_control_begin_time);
	}

	KCL::Matrix4x4 m;
	float t = par.time / 1000.0f;
	float tb = 0.0f;

	if (m_position_track)
	{
		Vector4D position;
		KCL::Vector3D p;

		_key_node::Get(position, m_position_track, t, tb);
		p.set(position.x, position.y, position.z);
		m.translate(p);
	}
	else
	{
		m.v[12] = local.v[12];
		m.v[13] = local.v[13];
		m.v[14] = local.v[14];
	}

	if( m_orientation_track)
	{
		Vector4D orientation;
		_key_node::Get( orientation, m_orientation_track, t, tb);

		float angle;
		KCL::Vector3D axis;

		KCL::Quaternion q;
		q.set( orientation.w, orientation.x, orientation.y, orientation.z);
		q.toAngleAxis (angle, axis);

		m.rotate (-angle, axis);
	}
	else
	{
		m.v[0] = local.v[0];
		m.v[1] = local.v[1];
		m.v[2] = local.v[2];

		m.v[4] = local.v[4];
		m.v[5] = local.v[5];
		m.v[6] = local.v[6];

		m.v[8] = local.v[8];
		m.v[9] = local.v[9];
		m.v[10] = local.v[10];
	}


	local = m;

	if (m_secondary_animation_enabled && m_parent->m_secondary_anim_control > 0.0f)
	{
		float radius = Min(m_secondary_animation_radius_factor * m_parent->m_secondary_anim_control, m_secondary_animation_radius_threshold);

		KCL::Matrix4x4 ss;
		ss.rotate(m_secondary_animation_rotation_factor * m_parent->m_secondary_anim_control * 60.0f, m_secondary_animation_rotation_axis);
		ss.translate(KCL::Vector3D(radius, radius, radius));

		local = ss * local;
	}

	KCL::Matrix4x4 old_parent = par.parent;
	bool old_visible = par.visible;

	if( !par.visible)
	{
		m_visible = false;
	}

	if( prev)
	{
		m_prev_world_pom = local * par.parent;
		par.parent = m_prev_world_pom;
	}
	else
	{
		m_world_pom = local * par.parent;
		par.parent = m_world_pom;
	}

	par.visible = m_visible;

	for( KCL::uint32 i=0; i<m_children.size(); i++)
	{
		m_children[i]->animate( prev, par);
	}

	par.parent = old_parent;
	par.visible = old_visible;
}


void KCL::Node::CreateFlickeringAnimation( KCL::uint32 keyframeCount, KCL::uint32 keyframeTime, float magnitude)
{
	if( m_scaleTrack)
	{
		return;
	}

	int random_seed = 0;
	KCL::uint32 componentCount = 3;

	m_scaleTrack = new AnimationTrack;
	m_scaleTrack->m_property = AnimationTrack::SCALE;

	m_scaleTrack->m_keyframeSequence = new KeyframeSequence;
	m_scaleTrack->m_keyframeSequence->m_keyframes = new float[keyframeCount * componentCount];
	m_scaleTrack->m_keyframeSequence->m_times = new int[keyframeCount];
	m_scaleTrack->m_keyframeSequence->m_keyframeCount = keyframeCount;
	m_scaleTrack->m_keyframeSequence->m_componentCount = componentCount;

	for (KCL::uint32 i = 0; i < keyframeCount; i++)
	{
		int time = i * keyframeTime;

		float rx = Math::randomf( &random_seed);
		rx = rx * magnitude + (1.0f - magnitude);
		float f[3] = { rx, 1.0f, rx};

		m_scaleTrack->m_keyframeSequence->setKeyframe( i, time, f);
	}
}

KCL::Node* KCL::Node::SearchNodePartialMatch(Node* node, const std::string &name)
{
    if( node->m_name.find(name) != std::string::npos)
	{
		return node;
	}
	else
	{
		for( KCL::uint32 i=0; i<node->m_children.size(); i++)
		{
			Node *n = SearchNodePartialMatch( node->m_children[i], name);
			if( n)
			{
				return n;
			}
		}
	}
	return 0;
}


KCL::Node* KCL::Node::SearchNodeByGuid(KCL::Node* node, const std::string &guid)
{
	if (node->m_guid == guid)
	{
		return node;
	}
	else
	{
		for (KCL::uint32 i = 0; i<node->m_children.size(); i++)
		{
			Node *n = SearchNodeByGuid(node->m_children[i], guid);
			if (n)
			{
				return n;
			}
		}
	}
	return 0;
}


KCL::Node* KCL::Node::SearchNode(Node* node, const std::string &name)
{
	if( node->m_name == name)
	{
		return node;
	}
	else
	{
		for( KCL::uint32 i=0; i<node->m_children.size(); i++)
		{
			Node *n = SearchNode( node->m_children[i], name);
			if( n)
			{
				return n;
			}
		}
	}
	return 0;
}


void KCL::Node::SearchNode( std::vector<Node*> &result, Node* node, ObjectType type)
{
	if( node->m_type == type)
	{
		result.push_back( node);
	}
	for( KCL::uint32 i=0; i<node->m_children.size(); i++)
	{
		SearchNode( result, node->m_children[i], type);
	}
}


void KCL::Node::RemoveChild( Node* node)
{
	std::vector<KCL::Node*> new_children;

	for( uint32 i=0; i<m_children.size(); i++)
	{
		if( m_children[i] != node)
		{
			new_children.push_back( m_children[i]);
		}
		else
		{
			node->m_parent = 0;
		}
	}

	m_children = new_children;

}

void KCL::Node::GetGlobalPOM(Matrix4x4& out)
{
	out = m_local_pom;
	KCL::Node* it = m_parent;
	while(it)
	{
		out = out * it->m_local_pom;
		it = it->m_parent;
	}
}



void KCL::Node::ShowUsAsLines( std::vector<KCL::Vector4D> &result, Node *node)
{
	result.push_back( KCL::Vector4D( &node->m_parent->m_world_pom.v[12]));
	result.push_back( KCL::Vector4D( &node->m_world_pom.v[12]));

	for( KCL::uint32 i=0; i<node->m_children.size(); i++)
	{
		ShowUsAsLines( result, node->m_children[i]);
	}
}


void KCL::Node::CollectNodes(std::vector<KCL::Node*> &result, Node *node)
{
	result.push_back(node);

	for (KCL::uint32 i = 0; i<node->m_children.size(); i++)
	{
		CollectNodes(result, node->m_children[i]);
	}
}
