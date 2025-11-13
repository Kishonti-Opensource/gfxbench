/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_light.h"


    
MetalRender::Light::Light ( const std::string& light_name, KCL::Node *parent, KCL::Object *owner) : KCL::Light(light_name, parent, owner)
{
}

   
KCL::Light *MetalRender::MTLLightFactory::Create(const std::string& light_name, KCL::Node *parent, KCL::Object *owner)
{
    return new MetalRender::Light(light_name,parent,owner);
}

