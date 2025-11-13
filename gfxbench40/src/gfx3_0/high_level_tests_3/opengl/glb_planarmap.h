/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DXB_PLANARMAP_H
#define DXB_PLANARMAP_H

#include <kcl_base.h>
#include <kcl_planarmap.h>

#include <vector>
#include <float.h>

#include <kcl_os.h>

namespace GLB
{
	class PlanarMap : public KCL::PlanarMap
	{
		friend class KCL::PlanarMap;
	public:
		~PlanarMap();

		void Bind();	
		void Unbind();

		unsigned int GetTextureId() const { return m_tboid; }

	protected:
		unsigned int m_tboid;
		unsigned int m_fboid;
		unsigned int m_rboid;

		PlanarMap( int w, int h, const char *name);
	};
}

#endif //GLB_PLANARMAP_H
