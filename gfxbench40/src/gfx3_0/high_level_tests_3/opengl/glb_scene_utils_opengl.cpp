/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_scene.h"

#include <kcl_light2.h>
#include <kcl_actor.h>
#include <kcl_room.h>

#include "glb_mesh.h"
#include "glb_material.h"
#include "opengl/glb_image.h"

#include "opengl/shader.h"
#include "platform.h"
#include "cubemap.h"
#include "shadowmap.h"
#include "opengl/glb_opengl_state_manager.h"

using namespace KCL;
using namespace GLB;

GLfloat unitCube[24] = {
/*unitCube[0]  =*/ -1.0f, //;
/*unitCube[1]  =*/ -1.0f, //;
/*unitCube[2]  =*/ -1.0f, //; //vertex_0

/*unitCube[3]  =*/  1.0f, //;
/*unitCube[4]  =*/ -1.0f, //;
/*unitCube[5]  =*/ -1.0f, //; //vertex_1

/*unitCube[6]  =*/  1.0f, //;
/*unitCube[7]  =*/  1.0f, //;
/*unitCube[8]  =*/ -1.0f, //; //vertex_2

/*unitCube[9]  =*/ -1.0f, //;
/*unitCube[10] =*/  1.0f, //;
/*unitCube[11] =*/ -1.0f, //; //vertex_3

/*unitCube[12] =*/  1.0f, //;
/*unitCube[13] =*/ -1.0f, //;
/*unitCube[14] =*/  1.0f, //; //vertex_4

/*unitCube[15] =*/ -1.0f, //;
/*unitCube[16] =*/ -1.0f, //;
/*unitCube[17] =*/  1.0f, //; //vertex_5

/*unitCube[18] =*/ -1.0f, //;
/*unitCube[19] =*/  1.0f, //;
/*unitCube[20] =*/  1.0f, //; //vertex_6

/*unitCube[21] =*/  1.0f, //;
/*unitCube[22] =*/  1.0f, //;
/*unitCube[23] =*/  1.0f  //; //vertex_7
};

static GLubyte line_indices[24] =
{
	0,1,
	1,2,
	2,3,
	3,0,
	4,5,
	5,6,
	6,7,
	7,4,
	3,6,
	2,7,
	0,5,
	1,4
};


GLubyte triangle_indices[36] =
{
	0,1,2, 0,2,3,
	1,4,7, 1,7,2,
	4,5,6, 4,6,7,
	5,0,3, 5,3,6,
	3,2,7, 3,7,6,
	5,4,1, 5,1,0
};

void DrawAABB(const AABB& aabb, Shader *s)
{
	GLfloat vertices[24];
	vertices[0]  = aabb.GetMinVertex().x;   vertices[1]  = aabb.GetMinVertex().y;   vertices[2]  = aabb.GetMinVertex().z; //vertex_0
	vertices[3]  = aabb.GetMaxVertex().x;   vertices[4]  = aabb.GetMinVertex().y;   vertices[5]  = aabb.GetMinVertex().z; //vertex_1
	vertices[6]  = aabb.GetMaxVertex().x;   vertices[7]  = aabb.GetMaxVertex().y;   vertices[8]  = aabb.GetMinVertex().z; //vertex_2
	vertices[9]  = aabb.GetMinVertex().x;   vertices[10] = aabb.GetMaxVertex().y;   vertices[11] = aabb.GetMinVertex().z; //vertex_3
	vertices[12] = aabb.GetMaxVertex().x;   vertices[13] = aabb.GetMinVertex().y;   vertices[14] = aabb.GetMaxVertex().z; //vertex_4
	vertices[15] = aabb.GetMinVertex().x;   vertices[16] = aabb.GetMinVertex().y;   vertices[17] = aabb.GetMaxVertex().z; //vertex_5
	vertices[18] = aabb.GetMinVertex().x;   vertices[19] = aabb.GetMaxVertex().y;   vertices[20] = aabb.GetMaxVertex().z; //vertex_6
	vertices[21] = aabb.GetMaxVertex().x;   vertices[22] = aabb.GetMaxVertex().y;   vertices[23] = aabb.GetMaxVertex().z; //vertex_7

	OpenGLStateManager::Commit();

	glVertexAttribPointer(s->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, 0, 0, vertices);

	glFrontFace( GL_CW);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (const void*)(triangle_indices));
	glFrontFace( GL_CCW);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, (const void*)line_indices);
}


void GLB_Scene_ES2::RenderVisibleAABBs( bool actors, bool rooms, bool room_meshes)
{
	int random_seed = 0;
	
	OpenGLStateManager::GlEnableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);

	if( actors)
	{
		for( KCL::uint32 j=0; j<m_actors.size(); j++)
		{
			glUniform3f(m_shader->m_uniform_locations[GLB::uniforms::background_color],
				Math::randomf( &random_seed),
				Math::randomf( &random_seed),
				Math::randomf( &random_seed));

			DrawAABB( m_actors[j]->m_aabb, m_shader);
		}
	}

	OpenGLStateManager::GlDepthMask( 0);
	{
		for( KCL::uint32 j=0; j<m_rooms.size(); j++)
		{
			if( rooms)
			{
				glUniform3f(m_shader->m_uniform_locations[GLB::uniforms::background_color],
				Math::randomf( &random_seed),
				Math::randomf( &random_seed),
				Math::randomf( &random_seed));

			DrawAABB( m_rooms[j]->m_aabb, m_shader);
			}

			if( room_meshes)
			{
				for( KCL::uint32 k=0; k<m_rooms[j]->m_meshes.size(); k++)
				{
					glUniform3f(m_shader->m_uniform_locations[GLB::uniforms::background_color],
						Math::randomf( &random_seed),
						Math::randomf( &random_seed),
						Math::randomf( &random_seed));

					DrawAABB( m_rooms[j]->m_meshes[k]->m_aabb, m_shader);
				}
			}
		}
	}
	OpenGLStateManager::GlDepthMask( 1);

	glUniform3f(m_shader->m_uniform_locations[GLB::uniforms::background_color], 0.5, 0.5, 0.5);

	DrawAABB( m_shadow_focus_aabb, m_shader);

	OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);
}


void GLB_Scene_ES2::RenderPortals()
{
	int random_seed = 0;
	
	OpenGLStateManager::GlEnableVertexAttribArrayInstantCommit(m_shader->m_attrib_locations[attribs::in_position]);

	for( KCL::uint32 j=0; j<m_portals.size(); j++)
	{
		XPortal *p = m_portals[j];

		glUniform3f(m_shader->m_uniform_locations[GLB::uniforms::background_color],
			Math::randomf( &random_seed),
			Math::randomf( &random_seed),
			Math::randomf( &random_seed));

		glVertexAttribPointer(m_shader->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, 0, 0, p->m_points[0].v);

			OpenGLStateManager::Commit();
#ifdef __glew_h__
			glDrawArrays(GL_POLYGON, 0, p->m_points.size());
#endif

			Vector3D v[2];
			v[0] = p->m_points[0];
			v[1] = p->m_points[0] + Vector3D( p->m_plane.v);

			glVertexAttribPointer(m_shader->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, 0, 0, v->v);

			glDrawArrays(GL_LINES, 0, 2);

	}

	OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);
}


void GLB_Scene_ES2::renderSkeleton( Node* node)
{
	glUniform3f(m_shader->m_uniform_locations[GLB::uniforms::background_color], 1, 1, 1);

	OpenGLStateManager::GlEnableVertexAttribArrayInstantCommit(m_shader->m_attrib_locations[attribs::in_position]);

	if( node->m_parent)
	{
		Vector3D v[2];

		v[0].set( node->m_parent->m_world_pom.v[12], node->m_parent->m_world_pom.v[13], node->m_parent->m_world_pom.v[14]);
		v[1].set( node->m_world_pom.v[12], node->m_world_pom.v[13], node->m_world_pom.v[14]);

		glVertexAttribPointer(m_shader->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, 0, 0, v);
		glDrawArrays( GL_LINES, 0, 2);
	}
	for( KCL::uint32 i=0; i<node->m_children.size(); i++)
	{
		renderSkeleton( node->m_children[i]);
	}
	OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[attribs::in_position]);
}


void Render_Frustum_Of_Camera_From_Eye(const Camera2& camera, const Camera2& eye, Shader *s)
{
	Matrix4x4 invMVP;
	
	if(!Matrix4x4::Invert4x4(camera.GetViewProjection(), invMVP))
	{
		return;
	}

	OpenGLStateManager::GlEnableVertexAttribArrayInstantCommit(s->m_attrib_locations[attribs::in_position]);
	glVertexAttribPointer(s->m_attrib_locations[attribs::in_position], 3, GL_FLOAT, 0, 0, unitCube);

	
	glUniformMatrix4fv( s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, (invMVP * eye.GetViewProjection() ).v);
		

	glUniform3f(s->m_uniform_locations[GLB::uniforms::background_color], 1, 0, 0); // background color

	glFrontFace( GL_CW);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, (const void*)(triangle_indices));
	glFrontFace( GL_CCW);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, (const void*)line_indices);

	OpenGLStateManager::GlDisableVertexAttribArray(s->m_attrib_locations[attribs::in_position]);
}
