/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_PLANARMAP_H
#define GLB_PLANARMAP_H

#include <kcl_base.h>
#include <kcl_planarmap.h>

#include <vector>
#include <float.h>

#include <kcl_os.h>

#include "d3d11/fbo3.h"

namespace DXB
{
	class PlanarMap : public KCL::PlanarMap
	{
		friend class KCL::PlanarMap;

	public:
		~PlanarMap();

		void Bind();	
		void Unbind();
		void Clear();

		unsigned int GetTextureId() const { return m_tboid; }
		
		GLB::FBO *m_FBO;

	protected:
		unsigned int m_tboid;
		unsigned int m_fboid;
		unsigned int m_rboid;

		PlanarMap( int w, int h, const char* name);
	};
}

#endif GLB_PLANARMAP_H
