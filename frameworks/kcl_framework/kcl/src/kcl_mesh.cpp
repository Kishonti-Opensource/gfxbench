/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_mesh.h>

#include <kcl_material.h>
#include <kcl_math3d.h>
#include <kcl_actor.h>
#include <map>

#ifndef EPSILON
#define EPSILON 0.0001f
#endif

#ifndef M_PI
#define M_PI 3.141592654f
#endif


KCL::Mesh::Mesh() : Node( "", MESH, 0, 0),
	m_vertexCenter(0.0f, 0.0f, 0.0f),
	m_lod_idx(0),
	m_mesh(NULL),
	m_material(NULL),
	m_lightmap( NULL),
	m_user_data( NULL),
	m_offset1( 0),
	m_is_motion_blurred( false),
	m_primitive_count( 0),
	m_room(NULL),
	m_frame_when_rendered(0),
	m_mio2( 0),
	m_flags(0)
{
	InitMembers();
}


KCL::Mesh::Mesh(const std::string &name, Node *parent, Object *owner) : Node(name, MESH, parent, owner),
	m_vertexCenter(0.0f, 0.0f, 0.0f),
	m_lod_idx(0),
	m_mesh(NULL),
	m_material(NULL),
	m_lightmap( NULL),
	m_user_data( NULL),
	m_offset1( 0),
	m_is_motion_blurred( false),
	m_primitive_count( 0),
	m_room(NULL),
	m_frame_when_rendered(0),
	m_mio2( 0),
	m_flags(0)
{
	InitMembers();
}


/*virtual*/ void KCL::Mesh::SetGuid(const std::string& guid)
{
	m_guid += "|" + guid;
}


void KCL::Mesh::InitMembers()
{
	m_mesh_variants[0] = 0;
	m_mesh_variants[1] = 0;
	m_mesh_variants[2] = 0;

	m_envmap_id = 0;

	m_ubo_handle = -1;
	m_indirect_draw_id = -1;
	m_num_visible_instances = 0;

	m_offset0[0] = 0;
	m_offset0[1] = 0;
}


KCL::Mesh::~Mesh()
{

}


void KCL::Mesh::CalculateStaticAABB()
{
	m_aabb.Reset();

	for(size_t i=0; i<m_mesh->m_vertex_attribs3[0].size(); ++i)
	{
		Vector3D v = mult4x3( m_world_pom, m_mesh->m_vertex_attribs3[0][i]);
		m_aabb.Merge( v);

		m_vertexCenter += v;
	}

	m_vertexCenter /= m_mesh->m_vertex_attribs3[0].size();

	KCL::Vector3D c, he;
	m_aabb.CalculateHalfExtentCenter( he, c);

	for( int i=0; i<3; i++)
	{
		if( he.v[i] < 0.01f)
		{
			he.v[i] = 0.01f;
		}
	}

	m_aabb.SetWithHalfExtentCenter( he, c);
}


void KCL::Mesh::GetPositionsInWorld( std::vector<Vector3D> &result)
{
	result.resize( m_mesh->m_vertex_attribs3[0].size());
	
	for(size_t i=0; i<m_mesh->m_vertex_attribs3[0].size(); ++i)
	{
		Vector3D v = mult4x3( m_world_pom, m_mesh->m_vertex_attribs3[0][i]);
		result[i] = v;
	}
}


void KCL::Mesh::GetNormalsInWorld( std::vector<Vector3D> &result)
{
	KCL::Matrix4x4 inv_model;

	//inv_model = Matrix4x4::Invert4x3( m_world_pom);
	inv_model = m_world_pom;

	result.resize( m_mesh->m_vertex_attribs3[1].size());
	
	for(size_t i=0; i<m_mesh->m_vertex_attribs3[1].size(); ++i)
	{
		Vector3D v = mult3x3( inv_model, m_mesh->m_vertex_attribs3[1][i]);
		result[i] = v;
	}
}

void KCL::Mesh::GetTangentsInWorld( std::vector<Vector3D> &result)
{
	KCL::Matrix4x4 inv_model;

	//inv_model = Matrix4x4::Invert4x3( m_world_pom);
	inv_model = m_world_pom;

	result.resize( m_mesh->m_vertex_attribs3[2].size());
	
	for(size_t i=0; i<m_mesh->m_vertex_attribs3[2].size(); ++i)
	{
		Vector3D v = mult3x3( inv_model, m_mesh->m_vertex_attribs3[2][i]);
		result[i] = v;
	}
}

void KCL::Mesh::GetMatrixF16( std::vector<float> &result)
{
	// for every float
	for( uint32 j=0; j<16; j++)
	{
		result.push_back( m_world_pom[j] );
	}
}

void KCL::Mesh::SetMaterials( KCL::Material *p, KCL::Material *s)
{
	m_materials[0] = p;
	m_materials[1] = s;
	m_material = m_materials[0];
}


void KCL::Mesh::SetActiveMaterial( int idx)
{
	m_material = m_materials[idx];
}


KCL::uint32 KCL::Mesh3::MAX_BONES = 0;


void KCL::Mesh3::InitBoneData()
{
	m_node_matrices.resize(12 * MAX_BONES);
	m_prev_node_matrices.resize(12 * MAX_BONES);
	m_node_matrices2.resize(MAX_BONES);
}


void KCL::Mesh3::CalculateNormals()
{
	std::vector<KCL::Vector3D>& vertex = m_vertex_attribs3[0];
	std::vector<KCL::Vector3D>& normal = m_vertex_attribs3[1];
	const KCL::uint16 *indices = &m_vertex_indices[0][0];

	normal.resize(m_vertex_attribs3[0].size());

	for (size_t i = 0; i < normal.size(); i++)
	{
		normal[i].set(0.0f, 0.0f, 0.0f);
	}

	for (KCL::uint32 i = 0; i < m_vertex_indices[0].size() - 2; i += 3)
	{
		KCL::uint16 i1 = indices[i];
		KCL::uint16 i2 = indices[i + 1];
		KCL::uint16 i3 = indices[i + 2];

		const Vector3D &v1 = vertex[i1];
		const Vector3D &v2 = vertex[i2];
		const Vector3D &v3 = vertex[i3];

		KCL::Vector3D e0, e1;

		e0 = v2 - v1;
		e1 = v3 - v1;
		
		e0.normalize();
		e1.normalize();

		KCL::Vector3D n = KCL::Vector3D::cross(e0, e1);
		n.normalize();
		
		normal[i1] += n;
		normal[i2] += n;
		normal[i3] += n;
	}

	for (KCL::uint32 i = 0; i < normal.size(); i++)
	{
		Vector3D& n = normal[i];

		n.normalize();
	}
}


void KCL::Mesh3::CalculateTangents()
{
	std::vector<KCL::Vector3D>& vertex = m_vertex_attribs3[0];
	std::vector<KCL::Vector3D>& normal = m_vertex_attribs3[1];
	std::vector<KCL::Vector3D>& tangent = m_vertex_attribs3[2];
	std::vector<KCL::Vector2D>& texcoord = m_vertex_attribs2[0];
	const KCL::uint16 *indices = &m_vertex_indices[0][0];

	if( !normal.size())
	{
		return;
	}

	if( tangent.size())
	{
		return;
	}

	tangent.resize( m_vertex_attribs3[0].size());
	Vector3D *tan1 = new Vector3D[m_vertex_attribs3[0].size()];

	for (KCL::uint32 i = 0; i < m_vertex_indices[0].size() - 2; i+=3)
	{
		KCL::uint16 i1 = indices[i];
		KCL::uint16 i2 = indices[i+1];
		KCL::uint16 i3 = indices[i+2];

		const Vector3D &v1 = vertex[i1];
		const Vector3D &v2 = vertex[i2];
		const Vector3D &v3 = vertex[i3];

		const Vector2D &w1 = texcoord[i1];
		const Vector2D &w2 = texcoord[i2];
		const Vector2D &w3 = texcoord[i3];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = (s1 * t2 - s2 * t1);
		if( r == 0.0f)
		{
			r = 1.0f;
		}

		r = 1.0f / r;

		Vector3D sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;
	}

	for (KCL::uint32 i = 0; i < m_vertex_attribs3[0].size(); i++)
	{
		const Vector3D& n = normal[i];
		const Vector3D& t = tan1[i];

		// Gram-Schmidt orthogonalize
		tangent[i] = (t - n * Vector3D::dot(n, t)).normalize();
	}

	delete [] tan1;
}


void KCL::Mesh3::ConvertToBillboard( float left, float right, float bottom, float top, float z_offset)
{
	m_vertex_indices[0].push_back( 0);
	m_vertex_indices[0].push_back( 1);
	m_vertex_indices[0].push_back( 2);

	m_vertex_indices[0].push_back( 0);
	m_vertex_indices[0].push_back( 2);
	m_vertex_indices[0].push_back( 3);

	m_vertex_attribs3[0].push_back(Vector3D( left, bottom, z_offset));
	m_vertex_attribs3[0].push_back(Vector3D( right, bottom, z_offset));
	m_vertex_attribs3[0].push_back(Vector3D( right, top, z_offset));
	m_vertex_attribs3[0].push_back(Vector3D( left, top, z_offset));

	m_vertex_attribs2[0].push_back(Vector2D(0.0f, 1.0f));
	m_vertex_attribs2[0].push_back(Vector2D(1.0f, 1.0f));
	m_vertex_attribs2[0].push_back(Vector2D(1.0f, 0.0f));
	m_vertex_attribs2[0].push_back(Vector2D(0.0f, 0.0f));

	m_vertex_attribs2[1].push_back(Vector2D(-1.0f, -1.0f));
	m_vertex_attribs2[1].push_back(Vector2D( 1.0f, -1.0f));
	m_vertex_attribs2[1].push_back(Vector2D( 1.0f,  1.0f));
	m_vertex_attribs2[1].push_back(Vector2D(-1.0f,  1.0f));
}


void KCL::Mesh3::ConvertToParticle( KCL::uint32 num)
{
	for( KCL::uint32 i=0; i<num; i++)
	{
		m_vertex_indices[0].push_back( i*4 + 0);
		m_vertex_indices[0].push_back( i*4 + 1);
		m_vertex_indices[0].push_back( i*4 + 2);

		m_vertex_indices[0].push_back( i*4 + 0);
		m_vertex_indices[0].push_back( i*4 + 2);
		m_vertex_indices[0].push_back( i*4 + 3);

		m_vertex_attribs3[0].push_back(Vector3D( i, i, i));
		m_vertex_attribs3[0].push_back(Vector3D( i, i, i));
		m_vertex_attribs3[0].push_back(Vector3D( i, i, i));
		m_vertex_attribs3[0].push_back(Vector3D( i, i, i));

		m_vertex_attribs2[0].push_back(Vector2D(0.0f, 1.0f));
		m_vertex_attribs2[0].push_back(Vector2D(1.0f, 1.0f));
		m_vertex_attribs2[0].push_back(Vector2D(1.0f, 0.0f));
		m_vertex_attribs2[0].push_back(Vector2D(0.0f, 0.0f));

		m_vertex_attribs2[1].push_back(Vector2D(-1.0f, -1.0f));
		m_vertex_attribs2[1].push_back(Vector2D( 1.0f, -1.0f));
		m_vertex_attribs2[1].push_back(Vector2D( 1.0f,  1.0f));
		m_vertex_attribs2[1].push_back(Vector2D(-1.0f,  1.0f));
	}
}


void KCL::Mesh3::ConvertToSpark()
{
	//QUAD x2
	KCL::Vector3D top_left_1    ( -1.00,  0.00,  0.00); // idx 0
	KCL::Vector3D top_right_1   (  1.00,  0.00,  0.00); // idx 1
	KCL::Vector3D bottom_left_1 ( -1.00, -1.00,  0.00); // idx 2
	KCL::Vector3D bottom_right_1(  1.00, -1.00,  0.00); // idx 3
	
	KCL::Vector3D top_left_2    (  0.00,  0.00, -1.00); // idx 4
	KCL::Vector3D top_right_2   (  0.00,  0.00,  1.00); // idx 5
	KCL::Vector3D bottom_left_2 (  0.00, -1.00, -1.00); // idx 6
	KCL::Vector3D bottom_right_2(  0.00, -1.00,  1.00); // idx 7

	m_vertex_attribs3[0].push_back( top_left_1     );
	m_vertex_attribs3[0].push_back( top_right_1    );
	m_vertex_attribs3[0].push_back( bottom_left_1  );
	m_vertex_attribs3[0].push_back( bottom_right_1 );

	m_vertex_attribs3[0].push_back( top_left_2     );
	m_vertex_attribs3[0].push_back( top_right_2    );
	m_vertex_attribs3[0].push_back( bottom_left_2  );
	m_vertex_attribs3[0].push_back( bottom_right_2 );

	m_vertex_indices[0].push_back( 0 );
	m_vertex_indices[0].push_back( 1 );
	m_vertex_indices[0].push_back( 2 );
	m_vertex_indices[0].push_back( 1 );
	m_vertex_indices[0].push_back( 2 );
	m_vertex_indices[0].push_back( 3 );

	m_vertex_indices[0].push_back( 4 );
	m_vertex_indices[0].push_back( 5 );
	m_vertex_indices[0].push_back( 6 );
	m_vertex_indices[0].push_back( 5 );
	m_vertex_indices[0].push_back( 6 );
	m_vertex_indices[0].push_back( 7 );
}


void KCL::Mesh3::DeleteVertexAttribs()
{
	for( KCL::uint32 k=0; k<2; k++)
	{
		m_vertex_attribs2[k].clear();
	}
	for( KCL::uint32 k=0; k<4; k++)
	{
		m_vertex_attribs3[k].clear();
	}
	for( KCL::uint32 k=0; k<1; k++)
	{
		m_vertex_attribs4[k].clear();
	}
}


KCL::Mesh3::Mesh3( const char *name): m_name( name), m_instanceCount(1), m_num_patch_vertices( 3)
{
	m_vbo = 0;
	for( KCL::uint32 i=0; i<2; i++)
	{
		m_ebo[i].m_buffer = 0;
		m_ebo[i].m_offset = 0;
	}

	m_uv0_scale.set( 1.0f, 1.0f, 1.0f);
	memset(m_index_counts, 0, sizeof(m_index_counts));
}


KCL::Mesh3::~Mesh3()
{
}


void KCL::Mesh3::UpdateNodeMatrices()
{
	Matrix4x4 B[2];
	KCL::uint32 j = 0;
	float *dst[2] = 
	{
		&m_node_matrices[0],
		&m_prev_node_matrices[0],
	};

	std::vector<Node*>::iterator i = m_nodes.begin();

	while( i != m_nodes.end() )
	{
		B[0] = (*i)->m_invert_base_pose * (*i)->m_world_pom;
		B[1] = (*i)->m_invert_base_pose * (*i)->m_prev_world_pom;

		// TODO duplan van vegrehajtva
		m_node_matrices2[j] = B[0];

		int idx = j * 12;

		for( uint32 k=0; k<2; k++)
		{
			dst[k][idx  ] = B[k].v11; 
			dst[k][idx+1] = B[k].v21; 
			dst[k][idx+2] = B[k].v31;
			dst[k][idx+3] = B[k].v41; 

			dst[k][idx+4] = B[k].v12; 
			dst[k][idx+5] = B[k].v22; 
			dst[k][idx+6] = B[k].v32;
			dst[k][idx+7] = B[k].v42; 

			dst[k][idx+8] = B[k].v13; 
			dst[k][idx+9] = B[k].v23; 
			dst[k][idx+10] = B[k].v33;
			dst[k][idx+11] = B[k].v43;
		}

		i++;
		j++;
	}
}


void KCL::Mesh::DeleteUnusedAttribs()
{
	if( m_material && m_mesh)
	{
		if( !m_material->m_textures[KCL::Material::LIGHTMAP2])
		{
			m_mesh->m_vertex_attribs2[1].clear();
		}
		if( !m_material->m_textures[KCL::Material::BUMP])
		{
			m_mesh->m_vertex_attribs3[1].clear();
			m_mesh->m_vertex_attribs3[2].clear();
		}
	}
}


void KCL::Mesh3::ConvertToLensFlare()
{
	const int num_elems = 5;

	Vector2D sizes[ ] = // [ num_elems ]
	{
		Vector2D( 10.0f, 0.5f),
		Vector2D( 1.0f, 1.0f),
		Vector2D( 0.5f, 0.5f),
		Vector2D( 1.25f, 1.25f),
		Vector2D( 1.8f, 1.8f)
	};

	Vector2D texcoords[] = // [ 4 * num_elems ]
	{
		Vector2D( 0.0f, 0.0f),
		Vector2D( 0.25f, 0.0f),
		Vector2D( 0.0f, 1.0f),
		Vector2D( 0.25f, 1.0f),

		Vector2D( 0.75f, 0.0f),
		Vector2D( 1.0f, 0.0f),
		Vector2D( 0.75f, 1.0f),
		Vector2D( 1.0f, 1.0f),

		Vector2D( 0.5f, 0.0f),
		Vector2D( 0.75f, 0.0f),
		Vector2D( 0.5f, 1.0f),
		Vector2D( 0.75f, 1.0f),

		Vector2D( 0.5f, 0.0f),
		Vector2D( 0.75f, 0.0f),
		Vector2D( 0.5f, 1.0f),
		Vector2D( 0.75f, 1.0f),

		Vector2D( 0.75f, 0.0f),
		Vector2D( 1.0f, 0.0f),
		Vector2D( 0.75f, 1.0f),
		Vector2D( 1.0f, 1.0f)
	};

	for( KCL::uint32 i=0; i<num_elems; i++)
	{
		float f0 = i / (num_elems - 1.0f);
		if( i)
		{
			f0 += 0.1f;
		}
		//float f1 = (i + 1) / (num_elems - 1.0f);

		m_vertex_attribs3[0].push_back( Vector3D( sizes[i].x * -.1f, sizes[i].y * -.1f, f0 ));
		m_vertex_attribs3[0].push_back( Vector3D( sizes[i].x *  .1f, sizes[i].y * -.1f, f0 ));
		m_vertex_attribs3[0].push_back( Vector3D( sizes[i].x * -.1f, sizes[i].y *  .1f, f0 ));
		m_vertex_attribs3[0].push_back( Vector3D( sizes[i].x *  .1f, sizes[i].y *  .1f, f0 ));

		m_vertex_attribs2[0].push_back( texcoords[4 * i + 0]);
		m_vertex_attribs2[0].push_back( texcoords[4 * i + 1]);
		m_vertex_attribs2[0].push_back( texcoords[4 * i + 2]);
		m_vertex_attribs2[0].push_back( texcoords[4 * i + 3]);
	}

	for( KCL::uint32 i=0; i<num_elems; i++)
	{
		m_vertex_indices[0].push_back( i * 4 + 0);
		m_vertex_indices[0].push_back( i * 4 + 1);
		m_vertex_indices[0].push_back( i * 4 + 2);

		m_vertex_indices[0].push_back( i * 4 + 2);
		m_vertex_indices[0].push_back( i * 4 + 1);
		m_vertex_indices[0].push_back( i * 4 + 3);

	}
}

void KCL::Mesh3::AnimateMeshPositionsInWorld( std::vector<Vector3D> &result )
{
	
	for( uint32 i=0; i<m_vertex_attribs3[0].size(); i++)
	{
		Matrix4x4 final_matrix;
		Vector3D &position = m_vertex_attribs3[0][i];
		Vector4D &weights = m_vertex_attribs4[0][i];
		uint8 *matrix_indices = &m_vertex_matrix_indices[i * 4];

		memset( final_matrix.v, 0, 64);

		//for(uint32 k = 0; k < 2; ++k)
		for( uint32 j=0; j<4; j++)
		{
			//Matrix4x4 m = m_node_matrices2[matrix_indices[j]] * weights.v[j];
			//final_matrix += m;

			for( uint32 k=0; k<16; k++)
			{
				final_matrix.v[k] += m_node_matrices2[matrix_indices[j]].v[k] * weights.v[j];
			}

		}
		
		Vector3D v = mult4x3( final_matrix, position);
		v = position;

		result.push_back( v);
	}
}

void KCL::Mesh3::AnimateMeshNormalsInWorld( std::vector<Vector3D> &result )
{
	
	for( uint32 i=0; i<m_vertex_attribs3[1].size(); i++)
	{
		Matrix4x4 final_matrix;
		Vector3D &normal = m_vertex_attribs3[1][i];
		Vector4D &weights = m_vertex_attribs4[0][i];
		uint8 *matrix_indices = &m_vertex_matrix_indices[i * 4];

		memset( final_matrix.v, 0, 64);

		//for(uint32 k = 0; k < 2; ++k)
		for( uint32 j=0; j<4; j++)
		{
			//Matrix4x4 m = m_node_matrices2[matrix_indices[j]] * weights.v[j];
			//final_matrix += m;

			for( uint32 k=0; k<16; k++)
			{
				final_matrix.v[k] += m_node_matrices2[matrix_indices[j]].v[k] * weights.v[j];
			}

		}
		
		Vector3D n = mult3x3( final_matrix, normal);

		result.push_back( n );
	}
}

void KCL::Mesh3::GetAnimationDataStatic( unsigned int offset, std::vector<float>& bone_weights, std::vector<unsigned int>& bone_indices )
{
	for( uint32 i=0; i<m_vertex_attribs3[1].size(); i++)
	{
		bone_weights.push_back( m_vertex_attribs4[0][i].x );
		bone_weights.push_back( m_vertex_attribs4[0][i].y );
		bone_weights.push_back( m_vertex_attribs4[0][i].z );
		bone_weights.push_back( m_vertex_attribs4[0][i].w );

		bone_indices.push_back( offset + m_vertex_matrix_indices[i*4  ] );
		bone_indices.push_back( offset + m_vertex_matrix_indices[i*4+1] );
		bone_indices.push_back( offset + m_vertex_matrix_indices[i*4+2] );
		bone_indices.push_back( offset + m_vertex_matrix_indices[i*4+3] );
	}
}

void KCL::Mesh3::GetAnimationDataDynamic( std::vector<float>& bone_matrices )
{
	// every matrix
	for( uint32 i=0; i<MAX_BONES; i++)
	{
		// every float
		for( uint32 j=0; j<16; j++)
		{
			bone_matrices.push_back( m_node_matrices2[i][j] );
		}
	}
}


void KCL::MeshInstanceOwner::Instance()
{
	std::vector<KCL::Vector3D> va3[4];
	std::vector<KCL::Vector2D> va2[4];
	std::vector<KCL::uint16> vi[2];
	KCL::Mesh3* mesh= m_mesh->m_mesh;

	size_t va2_size = mesh->m_vertex_attribs2[1].size();

	for( uint32 j=0; j<4; j++)
	{
		va3[j] = mesh->m_vertex_attribs3[j];
		va2[j] = mesh->m_vertex_attribs2[j];
	}
	for( uint32 j=0; j<2; j++)
	{
		vi[j] = mesh->m_vertex_indices[j];
		m_vertex_indices_length[j] =  static_cast<KCL::uint32>(mesh->getIndexCount(j));
	}

	for( KCL::uint32 i=1; i<m_instances.size(); i++)
	{
		for( uint32 j=0; j<4; j++)
		{
			mesh->m_vertex_attribs3[j].insert( mesh->m_vertex_attribs3[j].end(), va3[j].begin(), va3[j].end());
			mesh->m_vertex_attribs2[j].insert( mesh->m_vertex_attribs2[j].end(), va2[j].begin(), va2[j].end());
		}

		for( uint32 j=0; j<2; j++)
		{
			mesh->m_vertex_indices[j].insert( mesh->m_vertex_indices[j].end(), vi[j].begin(), vi[j].end());
		}
	}

	for( KCL::uint32 i=0; i<m_instances.size(); i++)
	{
		for( KCL::uint32 j=0; j<va3[0].size(); j++)
		{
			KCL::Vector3D &orig_v = va3[0][j];
			KCL::Vector3D &new_v = mesh->m_vertex_attribs3[0][j + i * va3[0].size()];

			KCL::mult4x3( m_instances[i]->m_world_pom, orig_v, new_v);
		}
		for( KCL::uint32 k = 0; k < 2; k++)
		{
			for( size_t j = 0; j < vi[k].size(); j++)
			{
				mesh->m_vertex_indices[k][j + i * vi[k].size()] += static_cast<KCL::uint16>(i * va3[0].size());
			}
			m_instances[i]->m_offset0[k] = static_cast<KCL::uint32>(i * vi[k].size());
		}

		m_instances[i]->m_offset1 = i;
	}

	for( uint32 j=0; j<2; j++)
	{
		m_original_vertex_indices[j] = mesh->m_vertex_indices[j];
		m_current_vertex_indices[j] = mesh->m_vertex_indices[j];
	}

//TODO expression result unused:
//	for( KCL::uint32 i=0; i<m_instances.size(); i++)
//	{
//		mesh->m_vertex_attribs3[2];
//	}

	
	int wm, hm;
	GetLightMapSizeMultiplier(wm, hm);
	for( size_t i = 0; i < m_instances.size(); i++)
	{
		for (size_t j = 0; j < va2_size; j++)
		{
			size_t idx = i*va2_size+j;
			KCL::Vector2D &v2d = mesh->m_vertex_attribs2[1][idx];
			float u = v2d.x;
			float v = v2d.y;

			ConvertLightmapUV((int)i,1,1,u,v);
			v2d.x = u / wm;
			v2d.y = v / hm;
		}
	}


	m_prev_visibility_mask.resize( m_instances.size());
	m_visibility_mask.resize( m_instances.size());
}

void KCL::MeshInstanceOwner::GetLightMapSizeMultiplier(int &widthmul, int &heightmul)
{
	widthmul = 1;
	heightmul = 1;

	int sqrt_n = sqrt( (float)m_instances.size());

	do 
	{
		widthmul <<= 1;
		heightmul <<= 1;
	}
	while( sqrt_n >>= 1);
}

void KCL::MeshInstanceOwner::ConvertLightmapUV(int k, float w, float h, float &u, float &v)
{
	int wm, hm;
	GetLightMapSizeMultiplier(wm, hm);
	int row = k / wm;
	int col = k % wm;
	u = ( col * w + u ); 
	v = ( row * h + v ); 
}


void KCL::MeshInstanceOwner::Update()
{
	if( memcmp( &m_visibility_mask[0], &m_prev_visibility_mask[0], m_instances.size()) == 0)
	{
		m_is_need_update = false;
		memset( &m_visibility_mask[0], 0, m_instances.size());
		return;
	}
	//KCL::Mesh3* mesh = m_mesh->m_mesh;


	for( KCL::uint32 j=0; j<2; j++)
	{
		memset( &m_current_vertex_indices[j][0], 0, m_original_vertex_indices[j].size() * 2);
	
		for( uint32 i=0; i<m_visible_instances.size(); i++)
		{
			int offset = m_visible_instances[i]->m_offset0[j];
			memcpy( &m_current_vertex_indices[j][offset], &m_original_vertex_indices[j][offset], m_vertex_indices_length[j] * 2);
		}
	}

	m_prev_visibility_mask = m_visibility_mask;

	memset( &m_visibility_mask[0], 0, m_instances.size());
	m_is_need_update = true;
}


bool KCL::MeshInstanceOwner::IsNeedUpdate()
{
	return m_is_need_update;
}


KCL::MeshInstanceOwner::~MeshInstanceOwner()
{
	delete m_mesh;
}


KCL::MeshInstanceOwner2::MeshInstanceOwner2()
{
	m_mesh = NULL;
	m_material = NULL;
	m_frame_when_rendered = 0;
	m_indirect_draw_id = -1;
}


struct CVertex
{
	KCL::Vector2D m_va2[2];
	KCL::Vector3D m_va3[3];
	float BoneWeights[4];
	KCL::uint16 BoneIndices[4];
};

class CPartition  
{  
	friend class KCL::Mesh3;
private:  
	std::vector< KCL::uint16 > BoneIndices;
	std::vector< CVertex > _Vertices;
	std::vector< KCL::uint16 > _Indices;
	std::map< KCL::uint16, KCL::uint16 > _IndicesMap;

private:  
	KCL::uint16 _AddVertex( CVertex& Vertex, KCL::uint16 VertexIndex );  
	KCL::int16 GetBoneRemap( KCL::uint16 BoneIndex );  
public:  
	bool AddPrimitive( KCL::uint8 VerticesCount, CVertex Vertices[], KCL::uint16 VerticesIndices[], KCL::uint32 MaxBonesPerPartition );  
};  


KCL::uint16 CPartition::_AddVertex( CVertex& Vertex, KCL::uint16 VertexIndex )  
{  
	KCL::uint16 Index;  

	std::map<KCL::uint16, KCL::uint16>::iterator iter = _IndicesMap.find( VertexIndex );
	if ( iter != _IndicesMap.end() )  
	{  
		Index = iter->second;
		_Indices.push_back( Index );  
	}  
	else  
	{  
		CVertex VertexPartitioned;
		VertexPartitioned = Vertex;  

		for ( KCL::uint32 iBone = 0; iBone < 4; iBone++ )  
		{  
			if ( Vertex.BoneWeights[iBone] == 0 )  
				continue;  

			VertexPartitioned.BoneIndices[iBone] = GetBoneRemap( Vertex.BoneIndices[iBone] );  
		}

		Index = static_cast<KCL::uint16>(_Vertices.size());
		_Indices.push_back( Index );
		_Vertices.push_back( VertexPartitioned );
		_IndicesMap[VertexIndex] = Index;
	}  

	return Index;  
}  


bool CPartition::AddPrimitive( KCL::uint8 VerticesCount, CVertex Vertices[], KCL::uint16 VerticesIndices[], KCL::uint32 MaxBonesPerPartition )  
{  
	KCL::uint16 BonesToAdd[32];  
	KCL::uint32 BonesToAddCount = 0;  

	for ( KCL::uint32 iVertex = 0; iVertex < VerticesCount; iVertex++ )  
	{  
		for ( KCL::uint32 iBone = 0; iBone < 4; iBone++ )  
		{  
			if ( Vertices[iVertex].BoneWeights[iBone] > 0 )  
			{  
				KCL::uint32 BoneIndex = Vertices[iVertex].BoneIndices[iBone];  
				bool NeedToAdd = true;  
				for ( KCL::uint32 iBoneToAdd = 0; iBoneToAdd < BonesToAddCount; iBoneToAdd++ )  
				{  
					if ( BonesToAdd[iBoneToAdd] == BoneIndex )  
					{  
						NeedToAdd = false;  
						break;  
					}  
				}  
				if ( NeedToAdd )  
				{  
					BonesToAdd[BonesToAddCount] = BoneIndex;  
					KCL::int16 BoneRemap = GetBoneRemap( BoneIndex );  
					BonesToAddCount += ( BoneRemap == -1 ? 1 : 0 );  
				}  
			}  
		}  
	}  

	if ( ( BoneIndices.size() + BonesToAddCount ) > MaxBonesPerPartition )  
	{  
		return false;  
	}  

	for ( KCL::uint32 iBone = 0; iBone < BonesToAddCount; iBone++ )  
	{  
		BoneIndices.push_back( BonesToAdd[iBone] );  
	}  

	for ( KCL::uint32 iVertex = 0; iVertex < VerticesCount; iVertex++ )  
	{  
		_AddVertex( Vertices[iVertex], VerticesIndices[iVertex] );  
	}  

	return true;  
}


KCL::int16 CPartition::GetBoneRemap( KCL::uint16 BoneIndex )  
{  
	for ( KCL::uint32 iBone = 0; iBone < BoneIndices.size(); iBone++ )  
	{  
		if ( BoneIndices[iBone] == BoneIndex )  
		{  
			return iBone;  
		}  
	}  
	return -1;  
}  


void KCL::Mesh3::AdjustMeshToJointNum( KCL::Mesh3Factory &factory, std::vector<Mesh3*> &result, uint32 max_joint_num)
{
	std::vector< CPartition > Skinned_PartitionList;
	std::vector< CPartition >&  PartitionBuilders = Skinned_PartitionList;  

	size_t IndexEnd = m_vertex_indices[0].size() / 3;  

	for ( KCL::uint32 iIndex = 0; iIndex < IndexEnd; iIndex++)  
	{  
		CVertex PrimitiveVertices[3];
		KCL::uint16 PrimitiveVerticesIndices[3];

		for( int k=0; k<3; k++)
		{
			KCL::uint16 idx = m_vertex_indices[0][iIndex*3 + k];

			PrimitiveVerticesIndices[k] = idx;

			PrimitiveVertices[k].m_va3[0] = m_vertex_attribs3[0][idx];
			PrimitiveVertices[k].m_va3[1] = m_vertex_attribs3[1][idx];
			PrimitiveVertices[k].m_va3[2] = m_vertex_attribs3[2][idx];

			PrimitiveVertices[k].m_va2[0] = m_vertex_attribs2[0][idx];

			PrimitiveVertices[k].BoneWeights[0] = m_vertex_attribs4[0][idx].x;
			PrimitiveVertices[k].BoneWeights[1] = m_vertex_attribs4[0][idx].y;
			PrimitiveVertices[k].BoneWeights[2] = m_vertex_attribs4[0][idx].z;
			PrimitiveVertices[k].BoneWeights[3] = m_vertex_attribs4[0][idx].w;

			PrimitiveVertices[k].BoneIndices[0] = m_vertex_matrix_indices[idx*4 + 0];
			PrimitiveVertices[k].BoneIndices[1] = m_vertex_matrix_indices[idx*4 + 1];
			PrimitiveVertices[k].BoneIndices[2] = m_vertex_matrix_indices[idx*4 + 2];
			PrimitiveVertices[k].BoneIndices[3] = m_vertex_matrix_indices[idx*4 + 3];
		}

		bool Added = false;  

		for ( KCL::uint32 iBonePartition = 0; iBonePartition < PartitionBuilders.size(); iBonePartition++ )  
		{  
			CPartition& PartitionBuilder = PartitionBuilders[iBonePartition];  

			if ( PartitionBuilder.AddPrimitive( 3, PrimitiveVertices, PrimitiveVerticesIndices, max_joint_num) )  
			{  
				Added = true;  
				break;  
			}  
		}  

		if ( !Added )  
		{  
			CPartition PartitionBuilder;

			PartitionBuilder.AddPrimitive( 3, PrimitiveVertices, PrimitiveVerticesIndices, max_joint_num );  

			PartitionBuilders.push_back( PartitionBuilder);
		}  
	}  

	for ( KCL::uint32 iPartition = 0; iPartition < PartitionBuilders.size(); iPartition++ )  
	{  
		CPartition& Partition = PartitionBuilders[iPartition];

		if ( Partition._Vertices.size() && Partition._Indices.size() )  
		{  
			char tmp[512];

			sprintf( tmp, "_%u_joints_%u", iPartition, max_joint_num);

			KCL::Mesh3* new_mesh = factory.Create( (m_name + tmp).c_str());
			new_mesh->InitBoneData();

			for( KCL::uint32 k=0; k<Partition.BoneIndices.size(); k++)
			{
				KCL::uint8 vmi = Partition.BoneIndices[k];
				new_mesh->m_nodes.push_back( m_nodes[vmi]);
			}

			for( KCL::uint32 idx=0; idx<Partition._Vertices.size(); idx++)
			{
				new_mesh->m_vertex_attribs2[0].push_back( Partition._Vertices[idx].m_va2[0]);

				new_mesh->m_vertex_attribs3[0].push_back( Partition._Vertices[idx].m_va3[0]);
				new_mesh->m_vertex_attribs3[1].push_back( Partition._Vertices[idx].m_va3[1]);
				new_mesh->m_vertex_attribs3[2].push_back( Partition._Vertices[idx].m_va3[2]);

				new_mesh->m_vertex_attribs4[0].push_back( KCL::Vector4D( Partition._Vertices[idx].BoneWeights[0], Partition._Vertices[idx].BoneWeights[1], Partition._Vertices[idx].BoneWeights[2], Partition._Vertices[idx].BoneWeights[3]));

				new_mesh->m_vertex_matrix_indices.push_back( Partition._Vertices[idx].BoneIndices[0]);
				new_mesh->m_vertex_matrix_indices.push_back( Partition._Vertices[idx].BoneIndices[1]);
				new_mesh->m_vertex_matrix_indices.push_back( Partition._Vertices[idx].BoneIndices[2]);
				new_mesh->m_vertex_matrix_indices.push_back( Partition._Vertices[idx].BoneIndices[3]);
			}


			for( KCL::uint32 i=0; i<Partition._Indices.size(); i++)
			{
				KCL::uint32 idx = Partition._Indices[i];

				new_mesh->m_vertex_indices[0].push_back( idx);
			}

			new_mesh->GenerateLOD( 100.0f);

			result.push_back( new_mesh);
		}
	}
}


void KCL::Mesh3::CreateSphere( std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, KCL::uint32 subdivisionsAxis, KCL::uint32 subdivisionsHeight)
{
	Vector3D vertex;
	Vector2D tc;
	KCL::uint16 index0;
	KCL::uint16 index1;
	KCL::uint16 index2;

	for (KCL::uint32 y = 0; y <= subdivisionsHeight; y++)
	{
		for (KCL::uint32 x = 0; x <= subdivisionsAxis; x++)
		{
			float u = (float)x / (float)subdivisionsAxis;
			float v = (float)y / (float)subdivisionsHeight;
			float theta = 2 * M_PI * u;
			float phi = M_PI * v;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);
			float sinPhi = sinf(phi);
			float cosPhi = cosf(phi);
			float ux = cosTheta * sinPhi;
			float uy = cosPhi;
			float uz = sinTheta * sinPhi;

			vertex.set( ux, uy, uz);
			tc.set( u, v);

			vertices.push_back( vertex);
			tcs.push_back( tc);
		}
	}

	KCL::uint32 numVertsAround = subdivisionsAxis + 1;

	for(KCL::uint32 x=0; x<subdivisionsAxis; x++)
	{
		for( KCL::uint32 y=0; y<subdivisionsHeight; y++) 
		{
			index0 = (y+0) * numVertsAround + x;
			index1 = (y+0) * numVertsAround + x + 1;
			index2 = (y+1) * numVertsAround + x;

			indices.push_back( index0);
			indices.push_back( index1);
			indices.push_back( index2);

			index0 = (y+1) * numVertsAround + x;
			index1 = (y+0) * numVertsAround + x + 1;
			index2 = (y+1) * numVertsAround + x + 1;

			indices.push_back( index0);
			indices.push_back( index1);
			indices.push_back( index2);
		}
	}
}

void KCL::Mesh3::CreateCone(std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, KCL::uint32 subdivisionsAxis, KCL::uint32 subdivisionsHeight)
{
	Vector3D vertex;
	Vector2D tc;
	float theta = 0.0f;
	float thetaStep = 2 * M_PI / subdivisionsAxis;
	//float tcStep = 1.0f / subdivisionsAxis;
	float heightStep = 1.0f / subdivisionsHeight;
	float height = 1.0f - heightStep;

	// Push top vertex at index: subdivisionsAxis
	Vector3D topVertex(0, 0, 1);
	Vector2D topVertexTc(0.5f, 0.5f);
	vertices.push_back(topVertex);
	tcs.push_back(topVertexTc);

	// Push bottom vertex at index: subdivisionsAxis + 1
	Vector3D bottomVertex(0, 0, 0);
	Vector2D bottomVertexTc(0.5f, 0.5f);
	vertices.push_back(bottomVertex);
	tcs.push_back(bottomVertexTc);

	// Generate vertices: subdivisionsHeight * (subdivisionsAxis + 1) will be generated
	for (KCL::uint32 y = 0; y < subdivisionsHeight; y++, height -= heightStep)
	{
		float radius = (1 - height);
		for (KCL::uint32 x = 0; x <= subdivisionsAxis; x++, theta += thetaStep)
		{
			float sin = sinf(theta) * radius;
			float cos = cosf(theta) * radius;

			vertex.set(sin, cos, height);
			tc.set(0.5f + cos * 0.5f, 0.5 + sin * 0.5f);	
			vertices.push_back( vertex);
			tcs.push_back(tc);
		}
	}

	for (KCL::uint32 x = 1; x <= subdivisionsAxis; x++)
	{
		KCL::uint16 topRingOffset = x + 2;

		// Push top triangle
		indices.push_back(topRingOffset);
		indices.push_back(topRingOffset - 1);
		indices.push_back(0);

		// Push quads along vertical axis
		for (KCL::uint32 y = 1; y < subdivisionsHeight; y++, topRingOffset += subdivisionsAxis + 1)
		{
			indices.push_back(topRingOffset);
			indices.push_back(topRingOffset + subdivisionsAxis);
			indices.push_back(topRingOffset - 1);

			indices.push_back(topRingOffset);
			indices.push_back(topRingOffset + subdivisionsAxis + 1);
			indices.push_back(topRingOffset + subdivisionsAxis);
		}

		// Push bottom triangle
		indices.push_back(topRingOffset - 1);
		indices.push_back(topRingOffset);
		indices.push_back(1);
	}
}

void KCL::Mesh3::CreateCube( std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, std::vector<KCL::uint16>* edgeList )
{
	//TODO tcs

	vertices.reserve( 8 );
	vertices.push_back( KCL::Vector3D( -1.0f, -1.0f, -1.0f ) ); //0 left-bottom-near
	vertices.push_back( KCL::Vector3D(  1.0f, -1.0f, -1.0f ) ); //1 right-bottom-near
	vertices.push_back( KCL::Vector3D(  1.0f,  1.0f, -1.0f ) ); //2 right-top-near
	vertices.push_back( KCL::Vector3D( -1.0f,  1.0f, -1.0f ) ); //3 left-top-near
	vertices.push_back( KCL::Vector3D( -1.0f, -1.0f,  1.0f ) ); //4 left-bottom-far
	vertices.push_back( KCL::Vector3D(  1.0f, -1.0f,  1.0f ) ); //5 right-bottom-far
	vertices.push_back( KCL::Vector3D(  1.0f,  1.0f,  1.0f ) ); //6 right-top-far
	vertices.push_back( KCL::Vector3D( -1.0f,  1.0f,  1.0f ) ); //7 left-top-far

	indices.reserve( 3 * 2 * 6 );

	//front face
	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 2 );

	indices.push_back( 2 );
	indices.push_back( 3 );
	indices.push_back( 0 );

	//back face
	indices.push_back( 4 + 0 );
	indices.push_back( 4 + 1 );
	indices.push_back( 4 + 2 );

	indices.push_back( 4 + 2 );
	indices.push_back( 4 + 3 );
	indices.push_back( 4 + 0 );

	//left face
	indices.push_back( 0 );
	indices.push_back( 3 );
	indices.push_back( 7 );

	indices.push_back( 7 );
	indices.push_back( 4 );
	indices.push_back( 0 );

	//right face
	indices.push_back( 1 );
	indices.push_back( 2 );
	indices.push_back( 6 );

	indices.push_back( 6 );
	indices.push_back( 2 );
	indices.push_back( 1 );

	//top face
	indices.push_back( 3 );
	indices.push_back( 2 );
	indices.push_back( 6 );

	indices.push_back( 6 );
	indices.push_back( 7 );
	indices.push_back( 3 );

	//bottom face
	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 5 );

	indices.push_back( 5 );
	indices.push_back( 4 );
	indices.push_back( 0 );

	if( edgeList )
	{
		edgeList->reserve( 12 * 2 );

		//front face
		edgeList->push_back( 0 );
		edgeList->push_back( 1 );

		edgeList->push_back( 1 );
		edgeList->push_back( 2 );

		edgeList->push_back( 2 );
		edgeList->push_back( 3 );

		edgeList->push_back( 3 );
		edgeList->push_back( 0 );

		//right face
		edgeList->push_back( 1 );
		edgeList->push_back( 5 );

		edgeList->push_back( 5 );
		edgeList->push_back( 6 );

		edgeList->push_back( 6 );
		edgeList->push_back( 2 );

		//back face
		edgeList->push_back( 5 );
		edgeList->push_back( 4 );

		edgeList->push_back( 4 );
		edgeList->push_back( 7 );

		edgeList->push_back( 7 );
		edgeList->push_back( 6 );

		//left face
		edgeList->push_back( 4 );
		edgeList->push_back( 0 );

		edgeList->push_back( 3 );
		edgeList->push_back( 7 );
	}
}

void KCL::Mesh3::CreateCylinder(std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, KCL::uint32 subdivisionsAxis, KCL::uint32 subdivisionsHeight, std::vector<KCL::uint16>* edgeList)
{
	subdivisionsHeight = subdivisionsHeight < 2 ? 2 : subdivisionsHeight;

	Vector3D vertex;
	Vector2D tc;
	float theta = 0.0f;
	float thetaStep = 2 * M_PI / subdivisionsAxis;
	//float tcStep = 1.0f / subdivisionsAxis;
	float heightStep = 2.0f / (subdivisionsHeight - 1);
	float height = 1.0f;

	// Push top vertex at index: subdivisionsAxis
	Vector3D topVertex(0, 0, 1);
	Vector2D topVertexTc(0.5f, 0.5f);
	vertices.push_back(topVertex);
	tcs.push_back(topVertexTc);

	// Push bottom vertex at index: subdivisionsAxis + 1
	Vector3D bottomVertex(0, 0, -1);
	Vector2D bottomVertexTc(0.5f, 0.5f);
	vertices.push_back(bottomVertex);
	tcs.push_back(bottomVertexTc);

	// Generate vertices: subdivisionsHeight * (subdivisionsAxis + 1) will be generated
	for (KCL::uint32 y = 0; y < subdivisionsHeight; y++, height -= heightStep)
	{
		//float radius = (1 - height);
		for (KCL::uint32 x = 0; x <= subdivisionsAxis; x++, theta += thetaStep)
		{
			float sin = sinf(theta);// * radius;
			float cos = cosf(theta);// * radius;

			vertex.set(sin, cos, height);
			tc.set(0.5f + cos * 0.5f, 0.5 + sin * 0.5f);
			vertices.push_back(vertex);
			tcs.push_back(tc);
		}
	}

	if( edgeList )
	{
	//hardcoded for
	//subdivaxis=15
	//subdivheight=1
	//only used for volumetric light...
	for( int x = 3; x <= 17; ++x )
	{
		edgeList->push_back( x );
		edgeList->push_back( x+15 );
	}
	}


	for (KCL::uint32 x = 1; x <= subdivisionsAxis; x++)
	{
		KCL::uint16 topRingOffset = x + 2;

		// Push top triangle
		indices.push_back(topRingOffset);
		indices.push_back(0);
		indices.push_back(topRingOffset - 1);

	//if user wants edge list too
	if( edgeList )
	{
		edgeList->push_back( topRingOffset );
		edgeList->push_back( topRingOffset - 1 );
	}

		// Push quads along vertical axis
		for (KCL::uint32 y = 1; y < subdivisionsHeight; y++, topRingOffset += subdivisionsAxis + 1)
		{
			indices.push_back(topRingOffset);
			indices.push_back(topRingOffset - 1);
			indices.push_back(topRingOffset + subdivisionsAxis);

			indices.push_back(topRingOffset);
			indices.push_back(topRingOffset + subdivisionsAxis);
			indices.push_back(topRingOffset + subdivisionsAxis + 1);
		}

		// Push bottom triangle
		indices.push_back(topRingOffset - 1);
		indices.push_back(1);
		indices.push_back(topRingOffset);

	//if user wants edge list too
	if( edgeList )
	{
		edgeList->push_back( topRingOffset - 1 );
		edgeList->push_back( topRingOffset );
	}
	}
}

#define uint32_t KCL::uint32
#define uint16_t KCL::uint16
#define uint8_t KCL::uint8

#define int32_t KCL::int32
#define int16_t KCL::int16
#define int8_t KCL::int8

#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif
#include "forsyth.cpp"
#include <algorithm>


void KCL::Mesh3::reorderVertices(std::vector<uint16>& vertex_remapping)
{
	std::vector<KCL::Vector2D> vertex_attribs2[4];
	std::vector<KCL::Vector3D> vertex_attribs3[4];
	std::vector<KCL::Vector4D> vertex_attribs4[1];
	std::vector<KCL::uint8> vertex_matrix_indices;

	for( uint32 j=0; j<4; j++)
	{
		if( m_vertex_attribs2[j].size())
		{
			vertex_attribs2[j].resize( m_vertex_attribs2[j].size());
		}
		if( m_vertex_attribs3[j].size())
		{
			vertex_attribs3[j].resize( m_vertex_attribs3[j].size());
		}
	}

	if( m_vertex_attribs4[0].size())
	{
		vertex_attribs4[0].resize( m_vertex_attribs4[0].size());
	}

	if( m_vertex_matrix_indices.size())
	{
		vertex_matrix_indices.resize( m_vertex_matrix_indices.size());
	}

	for(uint32 new_index=0; new_index<vertex_remapping.size(); ++new_index)
	{
		uint32 i = vertex_remapping[new_index];

		for( uint32 j=0; j<4; j++)
		{
			if( m_vertex_attribs2[j].size())
			{
				vertex_attribs2[j][new_index] = m_vertex_attribs2[j][i];
			}
			if( m_vertex_attribs3[j].size())
			{
				vertex_attribs3[j][new_index] = m_vertex_attribs3[j][i];
			}
		}
		if( m_vertex_attribs4[0].size())
		{
			vertex_attribs4[0][new_index] = m_vertex_attribs4[0][i];
		}
		if( m_vertex_matrix_indices.size())
		{
			vertex_matrix_indices[new_index*4+0] = m_vertex_matrix_indices[i*4+0];
			vertex_matrix_indices[new_index*4+1] = m_vertex_matrix_indices[i*4+1];
			vertex_matrix_indices[new_index*4+2] = m_vertex_matrix_indices[i*4+2];
			vertex_matrix_indices[new_index*4+3] = m_vertex_matrix_indices[i*4+3];
		}
	}

	for( uint32 j=0; j<4; j++)
	{
		m_vertex_attribs2[j] = vertex_attribs2[j];
		m_vertex_attribs3[j] = vertex_attribs3[j];
	}

	m_vertex_attribs4[0] = vertex_attribs4[0];

	m_vertex_matrix_indices = vertex_matrix_indices;
}


//
// Reorders triangles using Forsyth's algorithm, and then creates a vertex 
// remapping that fetches vertices in the order they are fetched in the new 
// triangle list, to avoid possible bad side-effects from the triangle 
// reordering.
//
void KCL::Mesh3::reorderTriangles(std::vector<uint16>& new_to_old, uint32 lod)
{
	// Reorder triangles with Forsyth's algorithm
	uint16 max_index = *std::max_element(m_vertex_indices[lod].begin(), m_vertex_indices[lod].end());
	size_t num_vertices = max_index+1;
	int32* indices_before_reordering = new int32[m_vertex_indices[lod].size()];
	for(unsigned int i = 0; i < m_vertex_indices[lod].size(); i++)
	{
		indices_before_reordering[i] = m_vertex_indices[lod][i];
	}
	VertexIndexType* indices_after_reordering = reorderForsyth( (const KCL::int32*)indices_before_reordering, static_cast<int>(m_vertex_indices[lod].size() / 3), static_cast<int>(num_vertices));

	// Calculate an index remapping that fetches vertices in order they are used in index buffer
	std::vector<uint16> old_to_new;
	std::vector<bool> index_used;
	old_to_new.resize(num_vertices, 0xFFFF);
	index_used.resize(num_vertices, false);
	uint16 new_index = 0;
	for(unsigned int i = 0; i < m_vertex_indices[lod].size(); i++)
	{
		uint16 old_index = indices_after_reordering[i];
		if(!index_used[old_index])
		{
			new_to_old.push_back(old_index);
			old_to_new[old_index] = new_index;
			index_used[old_index] = true;
			new_index++;
		}
	}

	// Remap indices for new vertex order
	for(unsigned int i = 0; i < m_vertex_indices[lod].size(); i++)
	{
		uint16 old_index = indices_after_reordering[i];
		m_vertex_indices[lod][i] = old_to_new[old_index];
	}

	delete[] indices_before_reordering;
	delete[] indices_after_reordering;
}


void KCL::Mesh3::VCacheOptimize()
{
	std::vector<uint16> vertex_remapping;

	reorderTriangles( vertex_remapping, 0);
	reorderVertices( vertex_remapping);
}


KCL::Mesh *KCL::DefaultMeshNodeFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	return new KCL::Mesh(name, parent, owner);
}
