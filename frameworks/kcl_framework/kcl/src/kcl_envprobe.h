/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_ENVPROBE_H
#define KCL_ENVPROBE_H

#include <kcl_math3d.h>
#include <kcl_serializable.h>
#include <kcl_aabb.h>

namespace KCL
{
	class EnvProbe : public Serializable
	{
	public:
		EnvProbe();
		virtual ~EnvProbe();

		virtual void Serialize(JsonSerializer& s);
		virtual std::string GetParameterFilename() const;

		KCL::AABB m_aabb;
		KCL::Vector3D m_pos;
		KCL::Vector3D m_half_extent;
		KCL::Vector3D m_sampling_pos;

		KCL::uint32 m_index;

		KCL::uint32 m_frame_counter;
	};
}

#endif
