/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_ANIMATION4_H
#define KCL_ANIMATION4_H

#include <kcl_math3d.h>
#include <kcl_io.h>

#include <stdio.h>
#include <vector>

namespace GLB
{
	class Resource;
}

namespace KCL
{
	enum _key_interpolation_type
	{
		KIT_NONE = 0,
		KIT_FINISHED,
		KIT_LRP1,
		KIT_LRP3,
		KIT_SLRP4,
		KIT_BEZIER1,
		KIT_MAX
	};


	struct _key_data
	{
		_key_interpolation_type m_interpolation_type;

		float m_time;
		float m_time_length;
		float m_inv_time_length;

		Vector4D m_data;
		Vector4D m_next_data;

		float m_out_tangent;
		float m_next_in_tangent;
	};


	struct _key_node
	{
		float m_divider;
		_key_node* m_children[2];
		_key_data *m_data;

		_key_node();
		~_key_node();

		static _key_node* Search( _key_node *root, float time);
		static float Get( Vector4D &result, _key_node *root, float &time, float &time_base, bool repeat = false);
		static void Read( _key_node *&node, AssetFile &io);

		static void PrepareData(std::vector<_key_data> &data);
		static void Create(_key_node *&node, std::vector<_key_data> &data, uint32 offset, uint32 size);
		static void Save(_key_node *node, File &io);

	};

	bool ReadAnimation(_key_node *&animation, const std::string &filename);

}//namespace KCL

#endif
