/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_NODE_H
#define KCL_NODE_H

#include <kcl_base.h>
#include <kcl_object.h>
#include <kcl_aabb.h>
#include <kcl_animationtrack.h>
#include <kcl_animation4.h>

#include <vector>

class JsonSerializer;
namespace KCL
{
	struct AnimateParameter
	{
		AnimateParameter (const KCL::Matrix4x4 &m, int t) : parent (m), time(t), visible( true)
		{
		}

		KCL::Matrix4x4 parent;
		int time;
		bool visible;
	};


	class Node : public Object
	{
	public:
		Node( const std::string &name, ObjectType type, Node* parent, Object *owner);
		virtual ~Node();

	private:
		Node(const Node& rhs);
		Node& operator=(const Node& rhs);

	public:
		std::string GetParameterFilename() const;

		void Serialize(JsonSerializer& s);

		Node *m_parent;
		std::vector<Node*> m_children;
	
		KCL::Matrix4x4 m_invert_base_pose;
		KCL::Matrix4x4 m_local_pom;
		KCL::Matrix4x4 m_world_pom;
		KCL::Vector3D m_translation;
		KCL::Matrix4x4 m_prev_world_pom;
		KCL::Vector3D m_scale;
		KCL::Vector3D m_axis;
		float m_angle;
		bool m_visible;
		float m_alpha;
		float m_secondary_anim_control;

		AnimationTrack *m_scaleTrack;
		_key_node *m_position_track;
		_key_node *m_orientation_track;
		_key_node *m_visibility_track;
		_key_node *m_alpha_track;
		_key_node *m_secondary_anim_control_track;

		bool LoadPositionTrack();
		bool LoadOrientationTrack();
		bool LoadVisibilityTrack();
		bool LoadAlphaTrack();
		bool LoadSecondaryAnimCcontrolTrack();

		bool m_secondary_animation_enabled;
		float m_secondary_animation_radius_factor;
		float m_secondary_animation_radius_threshold;
		float m_secondary_animation_rotation_factor;
		KCL::Vector3D m_secondary_animation_rotation_axis;
		float m_secondary_anim_control_begin_time;

		union
		{
			KCL::Object* m_owner; //for serialization, and to remove binding from actor upon deletion (lights...)
			KCL::Actor* m_owner_actor; //for serialization, and to remove binding from actor upon deletion (lights...)
			KCL::XRoom* m_owner_room; //for serialization, and to remove binding from actor upon deletion (lights...)
		};

		//gfx5 data
		KCL::Vector3D m_diff_pos;
		KCL::Matrix4x4 m_diff_rot;
		bool m_enable_animation;
		float m_weight;

		void animate( bool prev, AnimateParameter &par);
		void CreateFlickeringAnimation( KCL::uint32 keyframeCount, KCL::uint32 keyframeTime, float magnitude);
		void RemoveChild( Node* node);

		void GetGlobalPOM(Matrix4x4& out);

		static Node* SearchNode(Node* node, const std::string &name);
		static Node* SearchNodeByGuid(Node* node, const std::string &guid);
        static Node* SearchNodePartialMatch(Node* node, const std::string &name);
		static void SearchNode( std::vector<Node*> &result, Node* node, ObjectType type);
		static void ShowUsAsLines(std::vector<KCL::Vector4D> &result, Node *node);
		static void CollectNodes(std::vector<KCL::Node*> &result, Node *node);
	};
}

#endif
