/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_LIGHT_H
#define MTL_LIGHT_H

#include <kcl_light2.h>
#include <Metal/Metal.h>
#include <mtl_lensflare_30.h>


typedef uint64_t LensflareQueryResultType ;

namespace MetalRender
{
    
	class Light : public KCL::Light
	{
		friend class KCL::Light;
        friend class MTLLightFactory;
	public:
		
        static const int QUERY_COUNT = 4;
        
        KCL::uint32 m_query_objects[QUERY_COUNT];
        
	protected:
		Light ( const std::string& light_name, Node *parent, Object *owner);

	private:
        
	};
    
    class MTLLightFactory : public KCL::LightFactory
    {
    public:
        virtual KCL::Light *Create(const std::string& light_name, KCL::Node *parent, KCL::Object *owner);
    };
}

#endif // MTL_LIGHT_H
