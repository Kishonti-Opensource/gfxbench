/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_FACTORIES_H
#define MTL_FACTORIES_H

#include "kcl_texture.h"
#include "kcl_material.h"
#include "kcl_mesh.h"
#include "kcl_planarmap.h"
#include "kcl_particlesystem2.h"
#include "krl_scene.h"

#include "../gfxbench/global_test_environment.h"

namespace MetalRender
{


class Mesh3Factory : public KCL::Mesh3Factory
{
    public:
        virtual KCL::Mesh3 *Create(const char* name);
};
    

    

class PlanarMapFactory
{
public:
    KCL::PlanarMap *New(int w, int h, const char *name);
};
    
class EmitterFactory
{
public:
	KCL::_emitter *New(const std::string &name, KCL::ObjectType type, KCL::Node *parent, KCL::Object *owner);
};

KRL_Scene* CreateMTLScene30(const GlobalTestEnvironment* const gte);
KRL_Scene* CreateMTLScene27(const GlobalTestEnvironment* const gte);
KRL_Scene* CreateMTLScene31(const GlobalTestEnvironment* const gte);
KRL_Scene* CreateMTLScene40(const GlobalTestEnvironment* const gte);
    
}


#endif // MTL_FACTORIES_H
