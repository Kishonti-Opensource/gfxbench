/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__AABB_H__
#define __XX__AABB_H__

#include <kcl_math3d.h>
#include <cfloat>

/*
AABB representation:

     ------
     |\   |\
     | ------ max
 min --|--- |
      \|    |
       ------


Axis aligned minimal bounding box
WARNING: constructors do NOT validate minV and maxV !!!
*/

namespace KCL
{
	class AABB
	{
	public:
		AABB() : m_minVertex( FLT_MAX, FLT_MAX, FLT_MAX), m_maxVertex( -FLT_MAX, -FLT_MAX, -FLT_MAX)
		{}

		AABB(const Vector3D& minV, const Vector3D& maxV) : m_minVertex(minV), m_maxVertex(maxV)
		{}
	
		AABB(const AABB& aabb, float scale = 1.0f) :  m_minVertex(aabb.m_minVertex), m_maxVertex(aabb.m_maxVertex)
		{
			if(scale > 0 && scale != 1)
			{
				Vector3D halfExtent, center;
				CalculateHalfExtentCenter(halfExtent, center);
				halfExtent = halfExtent * scale;
				SetWithHalfExtentCenter(halfExtent, center);
			}
		}

		void SetWithHalfExtentCenter(const Vector3D &halfExtent, const Vector3D &center)
		{
			m_minVertex = center - halfExtent;
			m_maxVertex = center + halfExtent;
		}

		void Set(const Vector3D& minV, const Vector3D& maxV)
		{
			m_minVertex = minV;
			m_maxVertex = maxV;
		}

		void Reset()
		{
			m_minVertex = Vector3D( FLT_MAX, FLT_MAX, FLT_MAX);
			m_maxVertex = Vector3D( -FLT_MAX, -FLT_MAX, -FLT_MAX);
		}

		bool IsEmpty()
		{
			return m_minVertex.x == FLT_MAX;
		}

		void BiasAndScale( const Vector3D& bias, const Vector3D& scale)
		{
			Vector3D halfExtent, center;

			CalculateHalfExtentCenter( halfExtent, center);
			halfExtent = halfExtent + bias;
			halfExtent = halfExtent * scale;

			SetWithHalfExtentCenter(halfExtent, center);
		}

		void SetMin(const Vector3D& minV) { m_minVertex = minV; }
		void SetMax(const Vector3D& maxV) { m_maxVertex = maxV; }

		const Vector3D& GetMinVertex() const { return m_minVertex; }
		const Vector3D& GetMaxVertex() const { return m_maxVertex; }

		void Merge(const Vector3D& vertex)
		{
			if(m_minVertex.x >= vertex.x) m_minVertex.x = vertex.x;
			if(m_maxVertex.x <= vertex.x) m_maxVertex.x = vertex.x;

			if(m_minVertex.y >= vertex.y) m_minVertex.y = vertex.y;
			if(m_maxVertex.y <= vertex.y) m_maxVertex.y = vertex.y;
		
			if(m_minVertex.z >= vertex.z) m_minVertex.z = vertex.z;
			if(m_maxVertex.z <= vertex.z) m_maxVertex.z = vertex.z;
		}

		void Merge(const AABB& aabb)
		{
			if(m_minVertex.x >= aabb.m_minVertex.x) m_minVertex.x = aabb.m_minVertex.x;
			if(m_minVertex.y >= aabb.m_minVertex.y) m_minVertex.y = aabb.m_minVertex.y;
			if(m_minVertex.z >= aabb.m_minVertex.z) m_minVertex.z = aabb.m_minVertex.z;

			if(m_maxVertex.x <= aabb.m_maxVertex.x) m_maxVertex.x = aabb.m_maxVertex.x;
			if(m_maxVertex.y <= aabb.m_maxVertex.y) m_maxVertex.y = aabb.m_maxVertex.y;
			if(m_maxVertex.z <= aabb.m_maxVertex.z) m_maxVertex.z = aabb.m_maxVertex.z;
		}


		bool Inside(const Vector3D& vertex)
		{
			if( 
				vertex.x > m_minVertex.x &&
				vertex.y > m_minVertex.y &&
				vertex.z > m_minVertex.z &&
				vertex.x < m_maxVertex.x &&
				vertex.y < m_maxVertex.y &&
				vertex.z < m_maxVertex.z)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		void CalculateHalfExtentCenter(Vector3D& halfExtent, Vector3D& center) const
		{
			halfExtent.set((m_maxVertex.x - m_minVertex.x)/2.0f, (m_maxVertex.y - m_minVertex.y)/2.0f, (m_maxVertex.z - m_minVertex.z)/2.0f);
			center.set(m_minVertex.x + halfExtent.x, m_minVertex.y + halfExtent.y, m_minVertex.z + halfExtent.z);
		}

		float CalculateRadius() const
		{
			float result;
			Vector3D he, c;

			CalculateHalfExtentCenter( he, c);

			result = Max( he.x, he.y);
			result = Max( result, he.z);

			return result;
		}


		inline Vector2D DistanceFromPlane( const Vector4D &plane) const
		{
			float d;
			Vector2D result( FLT_MAX, -FLT_MAX);
			Vector4D p[8];
			CalculateVertices4D(p);

			for( KCL::uint32 i=0; i<8; i++)
			{
				d = Vector4D::dot( p[i], plane);
				if( d < result.x)
				{
					result.x = d;
				}
				if( d > result.y)
				{
					result.y = d;
				}
			}
			return result;
		}

		static bool Intersect(const AABB& lhs, const AABB& rhs)
		{
			return (lhs.GetMaxVertex().x >= rhs.GetMinVertex().x && rhs.GetMaxVertex().x >= lhs.GetMinVertex().x)
				&& (lhs.GetMaxVertex().y >= rhs.GetMinVertex().y && rhs.GetMaxVertex().y >= lhs.GetMinVertex().y)
				&& (lhs.GetMaxVertex().z >= rhs.GetMinVertex().z && rhs.GetMaxVertex().z >= lhs.GetMinVertex().z);
			//bool overlap = true;
			//overlap = (lhs.GetMinVertex().v[0] > rhs.GetMaxVertex().v[0] || lhs.GetMaxVertex().v[0] < rhs.GetMinVertex().v[0]) ? false : overlap;
			//overlap = (lhs.GetMinVertex().v[1] > rhs.GetMaxVertex().v[1] || lhs.GetMaxVertex().v[1] < rhs.GetMinVertex().v[1]) ? false : overlap;
			//overlap = (lhs.GetMinVertex().v[2] > rhs.GetMaxVertex().v[2] || lhs.GetMaxVertex().v[2] < rhs.GetMinVertex().v[2]) ? false : overlap;
			//return overlap;
		}

		bool Intersect(const KCL::Vector3D& centre, float radius)
		{
			float s,d=0;

			for(uint32 i=0;i<3;i++)
			{
				if( centre.v[i] < m_minVertex.v[i] )
				{
					s=centre.v[i]-m_minVertex.v[i];
					d+=s*s;
				}
				else if( centre.v[i] > m_maxVertex.v[i] )
				{
					s=centre.v[i]-m_maxVertex.v[i];
					d+=s*s;
				}
			}
			return d <= radius*radius;
		}


		static float Distance(const AABB& lhs, const AABB& rhs)
		{
			Vector3D lhsC((lhs.GetMinVertex().x + lhs.GetMaxVertex().x)/2.0f,
				(lhs.GetMinVertex().y + lhs.GetMaxVertex().y)/2.0f,
				(lhs.GetMinVertex().z + lhs.GetMaxVertex().z)/2.0f);
			Vector3D rhsC((rhs.GetMinVertex().x + rhs.GetMaxVertex().x)/2.0f,
				(rhs.GetMinVertex().y + rhs.GetMaxVertex().y)/2.0f,
				(rhs.GetMinVertex().z + rhs.GetMaxVertex().z)/2.0f);

			return Vector3D::distance(lhsC, rhsC);
		}

		void CalculateVertices4D(Vector4D* vertices_8) const
		{
			vertices_8[0] = Vector4D( m_minVertex.x, m_minVertex.y, m_minVertex.z, 1.0f);
			vertices_8[1] = Vector4D( m_maxVertex.x, m_minVertex.y, m_minVertex.z, 1.0f);
			vertices_8[2] = Vector4D( m_minVertex.x, m_minVertex.y, m_maxVertex.z, 1.0f);
			vertices_8[3] = Vector4D( m_maxVertex.x, m_minVertex.y, m_maxVertex.z, 1.0f);

			vertices_8[4] = Vector4D( m_minVertex.x, m_maxVertex.y, m_minVertex.z, 1.0f);
			vertices_8[5] = Vector4D( m_maxVertex.x, m_maxVertex.y, m_minVertex.z, 1.0f);
			vertices_8[6] = Vector4D( m_minVertex.x, m_maxVertex.y, m_maxVertex.z, 1.0f);
			vertices_8[7] = Vector4D( m_maxVertex.x, m_maxVertex.y, m_maxVertex.z, 1.0f);
		}

		void ConvertToBox( KCL::Vector3D vertices[8], uint8 *triangle_indices_36=0, uint8 *line_indices_24=0)
		{
			vertices[0].set( m_minVertex.x,m_minVertex.y,m_minVertex.z);
			vertices[1].set( m_maxVertex.x,m_minVertex.y,m_minVertex.z);
			vertices[2].set( m_minVertex.x,m_maxVertex.y,m_minVertex.z);
			vertices[3].set( m_maxVertex.x,m_maxVertex.y,m_minVertex.z);

			vertices[4].set( m_minVertex.x,m_minVertex.y,m_maxVertex.z);
			vertices[5].set( m_maxVertex.x,m_minVertex.y,m_maxVertex.z);
			vertices[6].set( m_minVertex.x,m_maxVertex.y,m_maxVertex.z);
			vertices[7].set( m_maxVertex.x,m_maxVertex.y,m_maxVertex.z);

			if( triangle_indices_36)
			{
				uint8 triangle_indices_[36] =
				{
					0,2,1, 1,2,3,
					4,5,6, 5,7,6,
					0,1,4, 1,5,4,
					2,6,3, 3,6,7,
					1,3,5, 5,3,7,
					0,4,2, 2,4,6
				};
				memcpy( triangle_indices_36, triangle_indices_, 36);
			}
			if( line_indices_24)
			{
				uint8 line_indices_[24] =
				{
					0,1,
					1,3,
					2,3,
					2,0,

					4,5,
					5,7,
					6,7,
					6,4,

					0,4,
					1,5,
					2,6,
					3,7
				};
				memcpy( line_indices_24, line_indices_, 24);
			}
		}

	private:
		Vector3D m_minVertex;
		Vector3D m_maxVertex;
	};
}

#endif //__XX__AABB_H__
