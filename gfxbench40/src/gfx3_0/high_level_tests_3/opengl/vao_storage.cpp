/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined HAVE_GLES3 || defined HAVE_GLEW
#include "vao_storage.h"

#include "opengl/glb_opengl_state_manager.h"

using namespace KCL ;

VaoStorage::VaoKey::VaoKey(KCL::Mesh3 *m3, GLB::GLBShaderCommon *s) {
	this->m3 = m3 ;
	this->s = s ;
}

bool VaoStorage::VaoKey::operator<(const VaoKey & other) const {
	if (this->m3 == other.m3) return ( s < other.s) ;

	return (this->m3 < other.m3) ;
}


VaoStorage::VaoStorage() {
}

VaoStorage::~VaoStorage() {
	clear();
}

void VaoStorage::clear()
{
	std::map<VaoKey,KCL::uint32>::iterator it ;
	for(it = m_vao_storage.begin() ; it != m_vao_storage.end() ; it++) {
		glDeleteVertexArrays( 1, &(it->second)) ;
	}
	m_vao_storage.clear() ;
}

KCL::uint32 VaoStorage::get(KCL::Mesh3 *m, GLB::GLBShaderCommon *s) {
	std::map<VaoKey,KCL::uint32>::iterator it = m_vao_storage.find(VaoKey(m,s)) ;
	if (it != m_vao_storage.end()) {
		return it->second ;
	}
	else
	{
		GLB::Mesh3* glb_m3 = dynamic_cast<GLB::Mesh3*>(m) ;

		KCL::uint32 r ;
		CreateVAO( glb_m3, s, r);

		m_vao_storage[VaoKey(m,s)] = r ;
		return r ;
	}
}

void VaoStorage::CreateVAO(GLB::Mesh3 *glb_m3, GLB::GLBShaderCommon *s, unsigned int &vao) {
	if( !glb_m3->m_instanceVBO)
	{
		glGenBuffers(1, &glb_m3->m_instanceVBO);
	}

	if( s)
	{
		glGenVertexArrays( 1, &vao);

		glBindVertexArray( vao);
		glBindBuffer( GL_ARRAY_BUFFER, glb_m3->m_vbo);

		for( unsigned int l=0; l<14; l++)
		{
			if( s->m_attrib_locations[l] > -1)
			{
				glEnableVertexAttribArray( s->m_attrib_locations[l]);

				glVertexAttribPointer(
					s->m_attrib_locations[l],
					glb_m3->m_vertex_attribs[l].m_size,
					glb_m3->m_vertex_attribs[l].m_type,
					glb_m3->m_vertex_attribs[l].m_normalized,
					glb_m3->m_vertex_attribs[l].m_stride,
					glb_m3->m_vertex_attribs[l].m_data
					);

				//glVertexAttribDivisor( s->m_attrib_locations[l], 0);
			}
		}


		if (s->m_attrib_locations[GLB::attribs::in_instance_mv0] > -1 && s->m_attrib_locations[GLB::attribs::in_instance_inv_mv] > -1)
		{
			glBindBuffer(GL_ARRAY_BUFFER, glb_m3->m_instanceVBO);

			for( int i=0; i<4; i++)
			{
				glEnableVertexAttribArray(s->m_attrib_locations[GLB::attribs::in_instance_mv0 + i]); //instance pom
				glEnableVertexAttribArray(s->m_attrib_locations[GLB::attribs::in_instance_inv_mv] + i); //instance pom

				glVertexAttribPointer(s->m_attrib_locations[GLB::attribs::in_instance_mv0 + i], 4, GL_FLOAT, 0, sizeof(KRL::Mesh3::InstanceData), (const void*)(i * 4 * sizeof(float)));
				glVertexAttribPointer(s->m_attrib_locations[GLB::attribs::in_instance_inv_mv] + i, 4, GL_FLOAT, 0, sizeof(KRL::Mesh3::InstanceData), (const void*)(sizeof(KCL::Matrix4x4) + i * 4 * sizeof(float)));

				//glVertexAttribDivisor( s->m_attrib_locations[GLB::attribs::in_instance_mv0 + i], 1);
				//glVertexAttribDivisor( s->m_attrib_locations[GLB::attribs::in_instance_inv_mv]+i, 1);
			}
		}


		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, glb_m3->m_ebo[0].m_buffer);
	}

	glBindVertexArray( 0);
	glBindBuffer( GL_ARRAY_BUFFER, 0);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
}
#endif
