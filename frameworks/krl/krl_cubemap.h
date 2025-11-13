/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KRL__CUBEMAP_H__
#define __KRL__CUBEMAP_H__

#include <kcl_math3d.h>
#include <kcl_os.h>

#include <vector>
#include <cfloat>

namespace KCL
{
	class Texture;
}


class KRL_CubeEnvMap
{
public:
	virtual ~KRL_CubeEnvMap() {}
	KCL::Texture* GetTexture() const { return m_texture; }
	const KCL::Vector3D& GetPosition() const { return m_position; }
	const KCL::Vector3D* GetAmbientColors() const { return m_ambient_colors; }
	void SetPosition(const KCL::Vector3D pos) { m_position = pos; }
	void SetAmbientColors(const KCL::Vector3D c[6]) { memcpy( m_ambient_colors->v, c->v, sizeof( KCL::Vector3D) * 6); }

protected:
	KRL_CubeEnvMap() : m_texture( 0) {}
	KRL_CubeEnvMap(const KRL_CubeEnvMap&);
	KRL_CubeEnvMap& operator=(const KRL_CubeEnvMap&);

	KCL::Texture* m_texture;
	KCL::Vector3D m_position;
	KCL::Vector3D m_ambient_colors[6];
};

class KRL_ParaboloidMap
{
public:
	virtual ~KRL_ParaboloidMap() {}
	KCL::Texture* GetTexture() const { return m_texture; }
	const KCL::Vector3D& GetPosition() const { return m_position; }
	const KCL::Vector3D* GetAmbientColors() const { return m_ambient_colors; }
	void SetPosition(const KCL::Vector3D pos) { m_position = pos; }

protected:
	KRL_ParaboloidMap() : m_texture( 0) {}
	KRL_ParaboloidMap(const KRL_ParaboloidMap&);
	KRL_ParaboloidMap& operator=(const KRL_ParaboloidMap&);

	KCL::Texture* m_texture;
	KCL::Vector3D m_position;
	KCL::Vector3D m_ambient_colors[2];
};


#endif //__KRL__CUBEMAP_H__
