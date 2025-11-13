/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#ifndef KRL_MATERIAL_H
#define KRL_MATERIAL_H

#include <kcl_base.h>
#include <kcl_object.h>
#include <kcl_material.h>

#include <string>
#include <vector>
#include <kcl_material.h>
class Shader;

namespace KRL {

	class Material : public KCL::Material
	{
	public:
		Material( const char *name);

		virtual void SetDefaults();
		virtual KCL::KCL_Status InitShaders( const char* path,  const std::string &max_joint_num_per_mesh);

		virtual Shader* CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error) = 0;

		
		Shader* m_shaders[2][3];
	};

}

#endif
