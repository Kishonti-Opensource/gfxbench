/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_MESH_H
#define KCL_MESH_H

#define USE_VBO

#include <kcl_base.h>
#include <kcl_material.h>
#include <kcl_node.h>
#include <kcl_aabb.h>
#include <kcl_room.h>


#include <string>
#include <vector>
#include <set>


namespace KCL
{
	struct VertexAttribArray
	{
		KCL::uint32 m_size;
		KCL::uint32 m_type;
		KCL::uint32 m_stride;
		bool m_normalized;
		const void *m_data;

		VertexAttribArray() : m_size(0), m_type(0), m_stride(0), m_normalized( false), m_data(0){}
	};

	class Mesh3Factory;
	struct MeshInstanceOwner2;

	class Mesh3
	{
	public:
		static KCL::uint32 MAX_BONES;

		int m_instanceCount; //number of instances present in the scene after loading


		std::string m_name;
		VertexAttribArray m_vertex_attribs[16];

		//texcoord
		std::vector<KCL::Vector2D> m_vertex_attribs2[4];
		//position, normal, tangent, color
		std::vector<KCL::Vector3D> m_vertex_attribs3[4];
		//weight
		std::vector<KCL::Vector4D> m_vertex_attribs4[1];
		//matrix indices
		std::vector<KCL::uint8> m_vertex_matrix_indices;

		std::vector<KCL::uint16> m_vertex_indices[2];

		unsigned int m_vbo;
		struct
		{
			unsigned int m_buffer;
			const void* m_offset;
		} m_ebo[2];

		std::vector<Node*> m_nodes;
		std::vector<float> m_node_matrices;
		std::vector<float> m_prev_node_matrices;
		std::vector<Matrix4x4> m_node_matrices2; // TODO: osszevonni a m_node_matrices
		KCL::Vector3D m_uv0_scale;
		KCL::uint32 m_num_patch_vertices;

		virtual ~Mesh3();
		Mesh3( const char *name);

		virtual void InitVertexAttribs(bool reinit = false) { }
		void DeleteVertexAttribs();
		void CalculateNormals();
		void CalculateTangents();
		void ConvertToBillboard( float left, float right, float bottom, float top, float z_offset);
		void ConvertToParticle( KCL::uint32 num);
		void ConvertToSpark();
		void ConvertToLensFlare();
		void UpdateNodeMatrices();
		void GenerateLOD( float threshold);
		void VCacheOptimize();
		size_t getIndexCount(int lod) const	{ return m_index_counts[lod] ? m_index_counts[lod] : m_vertex_indices[lod].size(); }
		void InitBoneData();

		void AnimateMeshPositionsInWorld( std::vector<Vector3D> &result );
		void AnimateMeshNormalsInWorld( std::vector<Vector3D> &result );

		void GetAnimationDataStatic( unsigned int offset, std::vector<float>& bone_weights, std::vector<unsigned int>& bone_indices );
		void GetAnimationDataDynamic( std::vector<float>& bone_matrices );

		void AdjustMeshToJointNum( KCL::Mesh3Factory &factory, std::vector<Mesh3*> &result, uint32 max_joint_num);

		static void CreateSphere( std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, KCL::uint32 subdivisionsAxis, KCL::uint32 subdivisionsHeight);
		static void CreateCone(std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, KCL::uint32 subdivisionsAxis, KCL::uint32 subdivisionsHeight = 1);
		static void CreateCylinder(std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, KCL::uint32 subdivisionsAxis, KCL::uint32 subdivisionsHeight = 1, std::vector<KCL::uint16>* edgeList = 0);
		static void CreateCube( std::vector<Vector3D> &vertices, std::vector<Vector2D> &tcs, std::vector<KCL::uint16> &indices, std::vector<KCL::uint16>* edgeList = 0 );

	protected:
		void reorderTriangles(std::vector<uint16>& new_to_old, uint32 lod);
		void reorderVertices(std::vector<uint16>& new_to_old);

		KCL::uint32 m_index_counts[2];
	};

	class Mesh3Factory
	{
	public:
		virtual ~Mesh3Factory() {}
		virtual Mesh3 *Create(const char* name) = 0;
	};


	class Mesh : public Node
	{
	public:
		friend class DefaultMeshNodeFactory;
		enum ObjectFlags
		{
			OF_SHADOW_CASTER = 1,
			OF_REFLECTION_CASTER = 2,
			OF_SHADOW_ONLY = OF_SHADOW_CASTER | 4,
			OF_LAST = 128
		};
	protected:

		Mesh( const std::string &name, Node *parent, Object *owner);
	public:
		Mesh();

		~Mesh();
		int m_lod_idx;
		AABB m_aabb;

		KCL::uint32 m_flags; //to restrict rendering for some passes, etc.

		KCL::Vector3D m_vertexCenter;
		Mesh3 *m_mesh;
		Mesh3 *m_mesh_variants[3];
	//protected:
		KCL::Material *m_material;
		std::string m_geometry_name; //for serialization

		KCL::Image2D *m_lightmap;
		KCL::Vector3D m_color;
		void *m_user_data;
		uint32 m_offset0[2];
		uint32 m_offset1;
		bool m_is_motion_blurred;
		KCL::uint32 m_primitive_count;
		KCL::uint32 m_envmap_id;

		KCL::int32 m_ubo_handle;
		KCL::int32 m_indirect_draw_id;
		KCL::uint32 m_num_visible_instances;
	public:
		virtual void SetGuid(const std::string& guid);

		void SetMaterials( KCL::Material *p, KCL::Material *s);
		void SetActiveMaterial( int idx);
		KCL::Material *m_materials[2];
		XRoom *m_room;

		void CalculateStaticAABB();
		void DeleteUnusedAttribs();

		void GetPositionsInWorld( std::vector<Vector3D> &result);
		void GetNormalsInWorld( std::vector<Vector3D> &result);
		void GetTangentsInWorld( std::vector<Vector3D> &result);
		void GetMatrixF16( std::vector<float> &result);

		KCL::uint32 m_frame_when_rendered;
		MeshInstanceOwner2 *m_mio2;
	private:
		void InitMembers();
	};

	class MeshFactory : public FactoryBase
	{
	public:
		virtual ~MeshFactory() {}
		virtual Mesh *Create(const std::string& name, KCL::Node *parent, KCL::Object *owner) = 0;
	};

	class DefaultMeshNodeFactory : public KCL::MeshFactory
	{
	public:
		virtual KCL::Mesh *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner);
	};


	class MeshInstanceOwner
	{
	public:
		std::vector<KCL::uint16> m_original_vertex_indices[2];
		std::vector<KCL::uint16> m_current_vertex_indices[2];
		KCL::uint32 m_vertex_indices_length[2];

		std::vector<KCL::Mesh*> m_visible_instances;
		std::vector<KCL::Mesh*> m_instances;
		KCL::Mesh *m_mesh;

		std::vector<KCL::uint8> m_prev_visibility_mask;
		std::vector<KCL::uint8> m_visibility_mask;

		void Instance();
		void GetLightMapSizeMultiplier(int &widthmul, int &heightmul);
		void ConvertLightmapUV(int k, float w, float h, float &u, float &v);
		void Update();
		bool IsNeedUpdate();

		~MeshInstanceOwner();
	private:
		bool m_is_need_update;
	};




	struct MeshInstanceOwner2
	{
		MeshInstanceOwner2();

		Mesh3 *m_mesh;
		Material* m_material;

		std::vector<KCL::Mesh*> m_visible_instances;
		std::vector<KCL::Mesh*> m_instances;
		KCL::uint32 m_frame_when_rendered;

		KCL::int32 m_indirect_draw_id;
	};

	class CullingAlgorithm
	{
	public:

		enum eType
		{
			CA_REFL = 1,
			CA_DEFAULT
		} m_type;

		bool m_force_cast;

		CullingAlgorithm(eType type = CA_DEFAULT) : m_type(type), m_force_cast(false) {}
		virtual ~CullingAlgorithm() {}
		virtual bool CullMesh(KCL::Camera2 *camera, KCL::Mesh *mesh) = 0;
	};

}

#endif
