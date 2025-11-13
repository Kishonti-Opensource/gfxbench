/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SPACEPARTITION_H
#define SPACEPARTITION_H

#include <kcl_aabb.h>
#include <vector>

class BVH;
class BVHNode;
struct MeshList;


const size_t MAXCHILDRENCOUNT = 4;


///Node for BVH tree
///Use only in BVH constructor
class BVHNode
{
public:
	~BVHNode()
	{
		for(size_t i=0; i < MAXCHILDRENCOUNT; ++i)
		{
			delete m_children[i];
		}
	}

	const KCL::AABB& GetAABB() const { return m_AABB; }
	const KCL::Vector3D& GetMinVertex() const { return GetAABB().GetMinVertex(); }
	const KCL::Vector3D& GetMaxVertex() const { return GetAABB().GetMaxVertex(); }
	MeshList* GetMeshList() { return m_mesh_list; }

private:
	friend class BVH;
	BVHNode(const KCL::Vector3D& minV, const KCL::Vector3D& maxV, MeshList* ml = 0) : m_AABB(minV, maxV), m_mesh_list(ml), m_parent(0)
	{
		for(size_t i=0; i < MAXCHILDRENCOUNT+1; ++i) m_children[i] = 0;
	}

	BVHNode(const KCL::AABB& aabb, MeshList* ml = 0) : m_AABB(aabb), m_mesh_list(ml), m_parent(0)
	{
		for(size_t i=0; i < MAXCHILDRENCOUNT+1; ++i) m_children[i] = 0;
	}

	KCL::AABB m_AABB; //the Node's own geometry
	MeshList* m_mesh_list;
	BVHNode* m_parent;
	BVHNode* m_children[MAXCHILDRENCOUNT+1]; //m_children[MAXCHILDRENCOUNT] has to be NULL! May store maximum MAXCHILDRENCOUNT count child node!

	BVHNode(const BVHNode&);
	BVHNode& operator=(const BVHNode&);
};


///Bounding volume hierarchy for static meshes
///Contains a tree built from Nodes
///WARNING: inpAABBs.size() has to be eq. to inpMeshes.size(),
///and inpAABBs[j] has to be the AABB of inpMeshes[j]!!!
class BVH
{
public:
	BVH(const std::vector<KCL::AABB> &inpAABBs, const std::vector<MeshList*> &inpMeshes);
	~BVH()
	{
		if(m_root)
			delete m_root;
	}

	void FrustumCull(const KCL::Camera2 *const cam, std::vector<BVHNode*> &out) const;
	void AABBCull(const KCL::AABB *const aabb, std::vector<BVHNode*> &out) const;


private:
	void process(std::vector<BVHNode*>& in, const float scale);
	void frustumCull(BVHNode* node, const KCL::Camera2 *const cam, std::vector<BVHNode*> &out) const;
	void AABBCull(BVHNode* node, const KCL::AABB *const cam, std::vector<BVHNode*> &out) const;

	BVH(const BVH&);
	BVH& operator=(const BVH&);

	BVHNode* m_root;
};

#endif //SPACEPARTITION_H__
