/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_CUBEMAP_H
#define KCL_CUBEMAP_H

#include <kcl_math3d.h>
#include <assert.h>
#include <string>

namespace KCL
{

class CubeEnvMapDescriptor
{
public:
	CubeEnvMapDescriptor() {};
	virtual ~CubeEnvMapDescriptor() {}

	std::string m_name;
	KCL::Vector3D m_position;
	KCL::Vector3D m_ambient_colors[6];
};

}

#endif