/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file lod.cpp
Implementation of mesh optimization funtionality.
*/
#include <kcl_mesh.h>
#include <kcl_os.h>

#if defined WIN32
#include "../platforms/windows/src/winapifamilynull.h"
#endif

#include <vector>
#include <string>
#include <cstring>

using namespace KCL;

/// Namespace for Level of Detail implementation.
namespace LOD2 
{
#define MR_EPSILON 5e-2f

	struct Edge;
	struct Triangle;
	class MeshRep2;
	class Heap;

	/// Structure that represents a triangle edge at LOD generation.
	struct Edge 
	{
		bool deleted;
		int i1, i2;		///< Offset into m_idxBuf, Inv: i1<i2
		int cnt;			///< How many triangles include this edge.
		int target, source;
		float weight;
		int heapPos;
		Edge()
		{
			deleted = false;
			i1 = 0;
			i2 = 0;
			cnt = 0;
			target = 0;
			source = 0;
			weight = 0.0f;
			heapPos = 0;
		}
	};

	/// Structure that represents a triangle and associated data at LOD generation.
	struct Triangle 
	{
		bool deleted;
		int i1, i2, i3;	///< Offset into m_idxBuf, Inv: i1<i2<i3 
		Matrix4x4 q;
		Triangle()
		{
			deleted = false;
			i1 = 0;
			i2 = 0;
			i3 = 0;
			q.identity();
		}
	};

	/// Structure that represents vertices and associated data at LOD generation.
	struct Vertex 
	{
		bool deleted;
		bool margin;
		Matrix4x4 q;
		std::vector<Edge*> edges;
		Vertex()
		{
			deleted = false;
			margin = false;
		}
	};

	/// A modified heap class.
	/// Added operations:
	/// -  remove(): removes an a specified element
	/// -  update(): updates the value (and the position in the heap) of an element
	/// -  operator[]: retrieves the n'th element of the representing std::vector
	class Heap
	{
	public:
		void make(std::vector<Edge*> &e);
		Edge *top() const;
		void pop();
		void push(Edge *e);
		size_t size() const;
		bool empty() const;
		void remove(Edge *e);
		void update(Edge *e);
		Edge *operator[] (int i) const;
	private:
		std::vector<Edge*> m_edge;
		void update(int i);
		int find(Edge *e);
		void up(const KCL::uint32 idx);
		void down(const KCL::uint32 idx);
	};

	void Heap::make(std::vector<Edge*> &e)
	{
		for (KCL::uint32 i=0; i<e.size(); i++)
			if (!e[i]->deleted)
			{
				m_edge.push_back( e[i]);
				e[i]->heapPos = (int)m_edge.size()-1;
			}
			for (KCL::uint32 i=0; i<m_edge.size(); i++)
				down(i);
	}

	inline Edge *Heap::top() const
	{
		return m_edge[0];
	}

	void Heap::pop()
	{
		if (size() > 1)
		{
			m_edge[0]->heapPos = -1;
			m_edge[0] = m_edge.back();
			m_edge[0]->heapPos = 0;
		}
		m_edge.pop_back();
		if (size() == 0)
			return;
		up(0);
	}

	void Heap::push(Edge *e)
	{
		if (e->deleted)
			return;
		m_edge.push_back(e);
		e->heapPos = (int)size()-1;
		down((KCL::uint32)size()-1);
	}

	inline size_t Heap::size() const
	{
		return m_edge.size();
	}

	inline bool Heap::empty() const
	{
		return m_edge.size() == 0;
	}

	void Heap::remove(Edge *e)
	{
		KCL::uint32 i = find(e);
		if (i == size())
			return;
		e->heapPos = -1;
		if (i != size()-1)
		{
			m_edge[i] = m_edge.back();
			m_edge[i]->heapPos = i;
		}
		m_edge.pop_back();
		if (size()==i || size()<=1)
			return;
		update(i);
	}

	void Heap::update(Edge *e)
	{
		KCL::uint32 i = find(e);
		if (i == size())
			return;
		update(i);
	}

	inline Edge *Heap::operator[] (int i) const
	{
		return m_edge[i];
	}

	void Heap::update(int i)
	{
		if (i>0 && m_edge[i]->weight < m_edge[(i-1)>>1]->weight)
			down(i);
		else
			up(i);
	}

	int Heap::find(Edge *e)
	{
		return e->heapPos!=-1 ? e->heapPos : (int)size();
	}

	template<typename T> inline void swap(T &e1, T&e2)
	{
		T tmp = e1;
		e1 = e2;
		e2 = tmp;
	}

	void Heap::up(const KCL::uint32 idx)
	{
		int i = (int)idx;
		int s = (int)size();
		bool loop = true;
		while (loop && i*2+1 < s)
		{
			int m = (int)i*2+1;
			if (i*2+2 < s && m_edge[i*2+1]->weight > m_edge[i*2+2]->weight)
				m = i*2+2;
			if (m_edge[m]->weight < m_edge[i]->weight)
			{
				swap(m_edge[m], m_edge[i]);
				swap(m_edge[m]->heapPos, m_edge[i]->heapPos);
			}
			else
				loop = false;
			i = m;
		}
	}

	void Heap::down(const KCL::uint32 idx)
	{
		int i = idx;
		while (i>0 && m_edge[i]->weight < m_edge[(i-1)>>1]->weight)
		{
			swap(m_edge[i], m_edge[(i-1)>>1]);
			swap(m_edge[i]->heapPos, m_edge[(i-1)>>1]->heapPos);
			i = (i-1)>>1;
		}
	}

	/// Class that represents a Mesh the levels are generated for.
	class MeshRep2
	{
	public:
		MeshRep2(const int nidx, const KCL::uint16 idx[], const int nvert, const float *vert);
		~MeshRep2();
		void getIdxBuf( std::vector<KCL::uint16> &indices);
		void decimateStep(float threshold);
		void decimate(float threshold);
		void calcTrianglePlane(Triangle &t);
	private:
		void decimateStep();
		void updateAllTriangles();
		Vector3D &vtxInd(int idx);
		void updateEdge(Edge &e, int target, int source);
		bool degeneratedTriangle(int t) const;
		Edge *findEdge(int i1, int i2);
		void calcEdgeWeight(Edge &e);
		void addEdge(int i1, int i2);
		void normalize(float size);
		Heap h;
		int m_nidx;
		KCL::uint16 *m_idxBuf;
		int m_nvert;
		Vector3D *m_vertBuf;
		std::vector<Vertex> m_vertex;
		std::vector<Edge*> m_edge;
		std::vector<Triangle> m_triangle;
	};

	MeshRep2::MeshRep2(const int nidx, const KCL::uint16 idx[], const int nvert, const float *vert):
		m_nidx(nidx),	m_idxBuf(new KCL::uint16[nidx]), m_nvert(nvert), m_vertBuf(new Vector3D[nvert])
	{
		memcpy(m_idxBuf, idx, sizeof(KCL::uint16)*nidx);
		memcpy(m_vertBuf, vert, sizeof(Vector3D)*nvert);
		normalize(100.f);
		for (KCL::int32 i=0; i<nvert; i++)
		{
			Vertex v;
			memset(&v.q.v11, 0, sizeof(float)*16);
			m_vertex.push_back(v);
		}
		for (KCL::int32 i=0; i<nidx/3; i++)
		{
			Triangle trg;
			trg.i1 = i*3 + 0;
			trg.i2 = i*3 + 1;
			trg.i3 = i*3 + 2;

			calcTrianglePlane(trg);
			m_triangle.push_back(trg);
			m_vertex[idx[i*3]].q += trg.q;
			m_vertex[idx[i*3+1]].q += trg.q;
			m_vertex[idx[i*3+2]].q += trg.q;
		}
		for (KCL::int32 i=0; i<nidx/3; i++)
		{
			addEdge(i*3, i*3+1);
			addEdge(i*3+1, i*3+2);
			addEdge(i*3, i*3+2);
		}
		for (KCL::uint32 i=0; i<m_edge.size(); i++)
			if (m_edge[i]->cnt==1)
			{
				Vertex &v1 = m_vertex[m_idxBuf[m_edge[i]->i1]];
				v1.margin = true;
				Vertex &v2 = m_vertex[m_idxBuf[m_edge[i]->i2]];
				v2.margin = true;
			}
			for (KCL::uint32 i=0; i<m_edge.size(); i++)
				calcEdgeWeight( *m_edge[i]);
			h.make( m_edge);
	}

	MeshRep2::~MeshRep2()
	{
		for( KCL::uint32 i=0; i<m_edge.size(); i++)
		{
			delete m_edge[i];
		}
		delete [] m_idxBuf;
		delete [] m_vertBuf;
	}

	void MeshRep2::getIdxBuf( std::vector<KCL::uint16> &indices)
	{
		KCL::uint32 tcnt = 0;
		for (KCL::uint32 i=0; i<m_triangle.size(); i++)
			if (!m_triangle[i].deleted)
				tcnt++;
		indices.resize( tcnt*3);
		for (KCL::uint32 i=0, j=0; j<tcnt; i++)
		{
			if (m_triangle[i].deleted)
				continue;

			indices[j*3] =   m_idxBuf[m_triangle[i].i1];
			indices[j*3+1] = m_idxBuf[m_triangle[i].i2];
			indices[j*3+2] = m_idxBuf[m_triangle[i].i3];
			j++;
		}
	}

	void MeshRep2::decimateStep(float threshold)
	{
		if (!h.empty() && h.top()->weight < threshold)
		{
			decimateStep();
			updateAllTriangles();
		}
	}

	void MeshRep2::decimateStep()
	{
		Edge &e = *h.top();
		assert(e.i1 != e.i2);
		h.top()->deleted = true;
		h.pop();
		int vTarget = m_idxBuf[e.target];
		int vSource = m_idxBuf[e.source];
		if (vSource == vTarget || (m_vertex[vSource].margin && m_vertex[vTarget].margin))
			return;

		m_vertex[vSource].q += m_vertex[vTarget].q;
		assert(!m_vertex[vTarget].margin);
		m_vertex[vTarget].deleted = true;
		for (int i=0; i<m_nidx; i++)
		{
			if (m_idxBuf[i] == vTarget)
			{
				m_idxBuf[i] = vSource;
			}
		}
		std::vector<Edge*> &eT = m_vertex[vTarget].edges;
		std::vector<Edge*> &eS = m_vertex[vSource].edges;
		size_t sizeS = eS.size();
		for (KCL::uint32 i=0; i<eT.size(); i++)
		{
			if (!eT[i]->deleted)
			{
				KCL::uint32 p = 0;
				while (p<sizeS && eS[p]!=eT[i])
					p++;
				if (p==sizeS)
					eS.push_back(eT[i]);
			}
		}
		std::vector<Edge*> &e2 = m_vertex[vSource].edges;
		for (KCL::uint32 i=0; i<e2.size(); i++)
			updateEdge(*e2[i], vTarget, vSource);
	}

	void MeshRep2::updateAllTriangles()
	{
		for (KCL::uint32 i=0; i<m_triangle.size(); i++)
			if (degeneratedTriangle(i))
				m_triangle[i].deleted = true;
	}

	void MeshRep2::decimate(float threshold)
	{
		while (!h.empty() && h.top()->weight < threshold)
		{
			decimateStep();
		}
		updateAllTriangles();
	}

	void MeshRep2::calcTrianglePlane(Triangle &t)
	{
		Vector3D norm = Vector3D::cross(vtxInd(t.i1)-vtxInd(t.i3), vtxInd(t.i2)-vtxInd(t.i3)).normalize();
		float a = norm.x, b = norm.y, c = norm.z, d = -Vector3D::dot(vtxInd(t.i3), norm);
		t.q.v11 = a*a; t.q.v12 = a*b; t.q.v13 = a*c; t.q.v14 = a*d;
		t.q.v21 = b*a; t.q.v22 = b*b; t.q.v23 = b*c; t.q.v24 = b*d;
		t.q.v31 = c*a; t.q.v32 = c*b; t.q.v33 = c*c; t.q.v34 = c*d;
		t.q.v41 = d*a; t.q.v42 = d*b; t.q.v43 = d*c; t.q.v44 = d*d;
	}

	Vector3D &MeshRep2::vtxInd(int idx)
	{
		return m_vertBuf[m_idxBuf[idx]];
	}

	void MeshRep2::updateEdge(Edge &edge, int target, int source)
	{ // Pre: i1<i2
		if (edge.deleted)
			return;
		if (m_idxBuf[edge.i1] == source || m_idxBuf[edge.i2] == source)
		{	// calculate new edge weight
			calcEdgeWeight(edge);
			h.update(&edge);
		}
	}

	bool MeshRep2::degeneratedTriangle(int t) const
	{
		const Triangle &trg = m_triangle[t];
		return
			trg.deleted ||
			m_idxBuf[trg.i1] == m_idxBuf[trg.i2] ||
			m_idxBuf[trg.i2] == m_idxBuf[trg.i3] ||
			m_idxBuf[trg.i3] == m_idxBuf[trg.i1];
	}

	Edge *MeshRep2::findEdge(int i1, int i2)
	{		// Pre: i1<i2
		KCL::uint32 i=0;
		KCL::uint16 v1 = m_idxBuf[i1], v2 = m_idxBuf[i2];
		std::vector<Edge*> &next1 = m_vertex[v1].edges;
		while (i<next1.size() && (next1[i]->deleted ||(
			(m_idxBuf[next1[i]->i1]!=v1 || m_idxBuf[next1[i]->i2]!=v2) &&
			(m_idxBuf[next1[i]->i1]!=v2 || m_idxBuf[next1[i]->i2]!=v1)
			)))
			i++;
		if (i<next1.size())
			return next1[i];
		i = 0;
		std::vector<Edge*> &next2 = m_vertex[v2].edges;
		while (i<next2.size() && (next2[i]->deleted ||(
			(m_idxBuf[next2[i]->i1]!=v1 || m_idxBuf[next2[i]->i2]!=v2) &&
			(m_idxBuf[next2[i]->i1]!=v2 || m_idxBuf[next2[i]->i2]!=v1)
			)))
			i++;
		return i==next2.size() ? NULL:next2[i];
	}

	void MeshRep2::calcEdgeWeight(Edge &e)
	{
		assert(!m_vertex[m_idxBuf[e.i1]].deleted);
		assert(!m_vertex[m_idxBuf[e.i2]].deleted);
		if (m_vertex[m_idxBuf[e.i1]].margin && m_vertex[m_idxBuf[e.i2]].margin)
		{
			e.deleted = true;
			return;
		}
		if (m_vertex[m_idxBuf[e.i1]].margin)
		{
			Matrix4x4 q = m_vertex[m_idxBuf[e.i2]].q;
			Vector4D v1(m_vertBuf[m_idxBuf[e.i1]]);
			float w1 = Vector4D::dot(q * v1, v1);
			if (fabs(w1)<MR_EPSILON)
				w1 = 0;
			e.weight = w1;
			e.target = e.i2;
			e.source = e.i1;
			return;
		}
		if (m_vertex[m_idxBuf[e.i2]].margin)
		{
			Matrix4x4 q = m_vertex[m_idxBuf[e.i1]].q;
			Vector4D v2(m_vertBuf[m_idxBuf[e.i2]]);
			float w2 = Vector4D::dot(q * v2, v2);
			if (fabs(w2)<MR_EPSILON)
				w2 = 0;
			e.weight = w2;
			e.target = e.i1;
			e.source = e.i2;
			return;
		}
		Matrix4x4 q = m_vertex[m_idxBuf[e.i1]].q + m_vertex[m_idxBuf[e.i2]].q;
		Vector4D v1(m_vertBuf[m_idxBuf[e.i1]]);
		Vector4D v2(m_vertBuf[m_idxBuf[e.i2]]);
		float w1 = Vector4D::dot(q * v1, v1);
		float w2 = Vector4D::dot(q * v2, v2);
		if (fabs(w1)<MR_EPSILON)
			w1 = 0;
		if (fabs(w2)<MR_EPSILON)
			w2 = 0;
		if (w1 < w2)
		{
			e.weight = w1;
			e.target = e.i2;
			e.source = e.i1;
		} else {
			e.weight = w2;
			e.target = e.i1;
			e.source = e.i2;
		}
	}

	void MeshRep2::addEdge(int i1, int i2)
	{
		Edge *e = new Edge;

		e->i1 = i1;
		e->i2 = i2;
		e->cnt = 1;

		Edge *fe = findEdge(i1, i2);
		if (fe)
		{
			fe->cnt++;
			return;
		}
		m_edge.push_back( e);
		m_vertex[m_idxBuf[i1]].edges.push_back( e);
		m_vertex[m_idxBuf[i2]].edges.push_back( e);
	}

	/// Structure that represents the bounding box of a Mesh.
	/// It is used only in LOD generation. It holds the axis aligned bounding box of the Mesh.
	/// We use this only once when renormalizing the vertex coordinates to increase accuracy in numeric calculations.
	struct BBox 
	{
		BBox(int nvertices, Vector3D *vertices)
		{
			max.x = min.x = vertices[0].x;
			max.y = min.y = vertices[0].y;
			max.z = min.z = vertices[0].z;
			for (int i=1; i<nvertices; i++)
			{
				if (vertices[i].x < min.x)
					min.x = vertices[i].x;
				else if (vertices[i].x > max.x)
					max.x = vertices[i].x;
				if (vertices[i].y < min.y)
					min.y = vertices[i].y;
				else if (vertices[i].y > max.y)
					max.y = vertices[i].y;
				if (vertices[i].z < min.z)
					min.z = vertices[i].z;
				else if (vertices[i].z > max.z)
					max.z = vertices[i].z;
			}
			size = max-min;
			center = (max+min)/2.f;
		}
		mutable Vector3D min, max, center, size;
	};

	void MeshRep2::normalize(float size)
	{
		Vector3D *vertices = m_vertBuf;
		int nvertices = m_nvert;
		BBox bb(nvertices, vertices);
		float scale = size/Max(Max(bb.size.x, bb.size.y), bb.size.z);
		for (int i=0; i<nvertices; i++)
		{
			vertices[i] = vertices[i]*scale+bb.center*(1-scale);
		}
	}

}


void Mesh3::GenerateLOD( float threshold)
{
	std::string filename;

	filename = m_name;
	filename += ".lod";

	if(!KCL::File::Exists(filename))
	{
#if defined WIN32
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		LOD2::MeshRep2 *mr = new LOD2::MeshRep2( (int)m_vertex_indices[0].size(), &m_vertex_indices[0][0], (int)m_vertex_attribs3[0].size(), m_vertex_attribs3[0][0].v);
		mr->decimate( threshold);
		mr->getIdxBuf( m_vertex_indices[1]);
		delete mr;

		KCL::File newlodfile(KCL::File::GetDataRWPath() + filename, Write, RDir);

		size_t num = m_vertex_indices[1].size();

		newlodfile.Write(&num, sizeof(unsigned int), 1);
		newlodfile.Write(&m_vertex_indices[1][0], sizeof( KCL::uint16), num);
		newlodfile.Close();
#else
		m_vertex_indices[1] = m_vertex_indices[0];
		INFO("warning: no lod info available!");
#endif
#else
		m_vertex_indices[1] = m_vertex_indices[0];
		INFO("warning: no lod info available!");
#endif
	}
	else
	{
		KCL::uint32 num;
		AssetFile file(filename);
		if(!file.GetLastError())
		{
			file.Read(&num, sizeof( KCL::uint32), 1);
			m_vertex_indices[1].resize( num);
			file.Read(&m_vertex_indices[1][0], sizeof( KCL::uint16), num);
			file.Close();
		}
	}

}
