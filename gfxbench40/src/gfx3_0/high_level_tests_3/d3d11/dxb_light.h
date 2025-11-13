/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DXB_LIGHT_H
#define DXB_LIGHT_H

#include <kcl_light2.h>
#include "d3d11/DX.h"

namespace DXB
{
	class Light : public KCL::Light
	{
		friend class KCL::Light;
		friend class DXBLightFactory;

	public:
		void NextQueryObject();
		
		ID3D11Query* GetCurrentQueryObject();
		ID3D11Query* GetPreviousQueryObject();
        void SetQueryObjects(ID3D11Query* queries[]);

		bool IsPreviousQueryObjectInitialized();

        static const int QUERY_COUNT = 4;

	protected:
		Light ( const std::string& light_name, Node *parent, Object *owner);

	private:
		KCL::uint32 m_current_query_index;
 		bool m_query_initialized[QUERY_COUNT];
		KCL::uint32 GetPreviousQueryIndex();
        ID3D11Query* m_query_objects[QUERY_COUNT];
	};

	class DXBLightFactory : public KCL::LightFactory
	{
	public:
		virtual KCL::Light *Create(const std::string& light_name, KCL::Node *parent, KCL::Object *owner);
	};
}

#endif
