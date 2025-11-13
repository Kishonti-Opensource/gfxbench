/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VAO_STORAGE_H
#define VAO_STORAGE_H


#include "glb_mesh.h"
#include <kcl_mesh.h>
#include "opengl/glb_shader_common.h"
#include <map>

class VaoStorage {
private:
	struct VaoKey {
		KCL::Mesh3 *m3 ;
		GLB::GLBShaderCommon *s ;

		VaoKey(KCL::Mesh3 *m3, GLB::GLBShaderCommon *s) ;
		bool operator<(const VaoKey &other) const ;
	};

public:
	VaoStorage() ;
	virtual ~VaoStorage() ;

	KCL::uint32 get(KCL::Mesh3 *m, GLB::GLBShaderCommon *s) ;
	void clear() ;

private:
	void CreateVAO(GLB::Mesh3 *glb_m3, GLB::GLBShaderCommon *s, unsigned int &vao) ;

	std::map<VaoKey,KCL::uint32> m_vao_storage ;
};



#endif // VAO_STORAGE_H