/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "spacepartition.h"

#include <kcl_camera2.h>
#include "glb_kcl_adapter.h"


using namespace GLB;


///simple pair datastructure for BVH::process
struct pbvh_distance{
	pbvh_distance(BVHNode *p =0, float d =0.0f) : pbvhNode(p), distance(d){}
	BVHNode *pbvhNode;
	float distance;

	static int cmp(const void* lhs, const void* rhs)
	{
		float ld = ((pbvh_distance*)lhs)->distance;
		float rd = ((pbvh_distance*)rhs)->distance;
		if(ld > rd) return -1;
		else if (ld == rd) return 0;
		return 1;
	}
};


void BVH::process(std::vector<BVHNode*>& in, const float scale)
{
	std::vector<BVHNode*> work;
	for(size_t i=0; i < in.size(); ++i)
	{
		work.push_back(in[i]);
	}

	in.clear();

	BVHNode* first = work[0];
	AABB aabbFat(first->GetAABB(), scale);

	std::vector<pbvh_distance> foundOnes;
	std::vector<BVHNode*> notFoundOnes;

	for(size_t i=1; i< work.size(); ++i)
	{
		if(AABB::Intersect(aabbFat, work[i]->GetAABB()))
			foundOnes.push_back(pbvh_distance(work[i], AABB::Distance(aabbFat, work[i]->GetAABB())));
		else
			notFoundOnes.push_back(work[i]);
	}

	if(foundOnes.size() > 0)
	{
		pbvh_distance* tmp_array = new pbvh_distance[foundOnes.size()];
		for(size_t i=0; i < foundOnes.size(); ++i)
			tmp_array[i] = foundOnes[i];
		qsort(tmp_array, foundOnes.size(), sizeof(pbvh_distance), &pbvh_distance::cmp);

		for(size_t i=0; i < notFoundOnes.size(); ++i)
			in.push_back(notFoundOnes[i]);

		BVHNode *newBvhNode = new BVHNode(first->GetAABB());
		newBvhNode->m_children[0] = first;
		first->m_parent = newBvhNode;

		size_t loop = 0;
		for(; loop < foundOnes.size() && loop < MAXCHILDRENCOUNT-1; ++loop)
		{
			newBvhNode->m_children[1+loop] = tmp_array[loop].pbvhNode;
			tmp_array[loop].pbvhNode->m_parent = newBvhNode;
			newBvhNode->m_AABB.Merge(tmp_array[loop].pbvhNode->GetAABB());
		}
		for(; loop < foundOnes.size(); ++loop)
			in.push_back(tmp_array[loop].pbvhNode);
		
		in.push_back(newBvhNode);
		
		delete[] tmp_array;
	}
	else
	{
		for(size_t i=0; i < work.size(); ++i)
			in.push_back(work[i]);
	}
}


BVH::BVH(const std::vector<AABB> &inpAABBs, const std::vector<MeshList*> &inpMeshes) : m_root( 0)
{
	float scale = 2.0f;
	std::vector<BVHNode*> work;

	if( inpAABBs.size() != inpMeshes.size())
	{
		return;
	}

	for(size_t i=0; i< inpAABBs.size(); ++i)
	{
		MeshList* ml = inpMeshes[i];
		if( ml)
		{
			work.push_back(new BVHNode(inpAABBs[i], ml));
		}
	}

	while(work.size() > 1)
	{
		process(work, scale);
		scale *= 2.0f;
	}

	m_root = work[0];
}


void BVH::FrustumCull(const Camera2 *const cam, std::vector<BVHNode*> &out) const
{
	if( !m_root)
	{
		return;
	}
	frustumCull(m_root, cam, out);
}


void BVH::frustumCull(BVHNode* node, const Camera2 *const cam, std::vector<BVHNode*> &out) const
{
	if(!cam->IsVisible(&node->GetAABB())) return;
	
	if(node->m_mesh_list)
	{
		out.push_back(node);
	}
	int loop=0;
	while(node->m_children[loop])
	{
		frustumCull(node->m_children[loop], cam, out);
		++loop;
	}
}


void BVH::AABBCull(const AABB *const aabb, std::vector<BVHNode*> &out) const
{
	if( !m_root)
	{
		return;
	}
	AABBCull(m_root, aabb, out);
}


void BVH::AABBCull(BVHNode* node, const AABB *const aabb, std::vector<BVHNode*> &out) const
{
	if( !AABB::Intersect(node->GetAABB(), *aabb)) 
	{
		return;
	}
	
	if(node->m_mesh_list)
	{
		out.push_back(node);
	}
	int loop=0;
	while(node->m_children[loop])
	{
		AABBCull(node->m_children[loop], aabb, out);
		++loop;
	}
}
