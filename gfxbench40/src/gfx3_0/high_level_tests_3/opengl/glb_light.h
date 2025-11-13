/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DXB_LIGHT_H
#define DXB_LIGHT_H

#include <kcl_light2.h>

namespace GLB
{
	class Light : public KCL::Light
	{
		friend class KCL::Light;
		friend class GLBLightFactory;

	public:
		void NextQueryObject();
		
		KCL::uint32 GetCurrentQueryObject();
		KCL::uint32 GetPreviousQueryObject();

		bool IsPreviousQueryObjectInitialized();

        static const int QUERY_COUNT = 4;
        KCL::uint32 m_query_objects[QUERY_COUNT];

		KCL::int32 m_ubo_handle;

	protected:
		Light ( const std::string& light_name, Node *parent, Object *owner);

	private:
		KCL::uint32 m_current_query_index;
 		bool m_query_initialized[QUERY_COUNT];
		KCL::uint32 GetPreviousQueryIndex();
	};

	class GLBLightFactory : public KCL::LightFactory
	{
	public:
		virtual KCL::Light *Create(const std::string& light_name, KCL::Node *parent, KCL::Object *owner);
	};

}

#endif
