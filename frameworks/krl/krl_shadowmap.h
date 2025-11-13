/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KRL__SHADOWMAP_H__
#define __KRL__SHADOWMAP_H__

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <kcl_camera2.h>

#include <vector>

class KRL_ShadowMap
{
public:
	virtual ~KRL_ShadowMap(){}

	KCL::Camera2 m_camera;
	KCL::Matrix4x4 m_matrix;
	std::vector<KCL::Mesh*> m_caster_meshes[2];
	std::vector<KCL::Mesh*> m_receiver_meshes[2];

	virtual void Bind() = 0;
	virtual void Unbind() = 0;
	virtual void Clear() = 0;
	virtual const int Size() const = 0;
	virtual unsigned int GetTextureId() const = 0;
};


#endif //__KRL__SHADOWMAP_H__
