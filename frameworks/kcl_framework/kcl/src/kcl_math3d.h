/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file math3d.h
	Contains definitions of classes used to calculate spatial data.
	Also inline Math3D functions and some utility function templates are defnied here.
*/
#ifndef KCL_MATH3D_H
#define KCL_MATH3D_H

#include <kcl_base.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace KCL
{
	inline bool IsNan(float v)
	{
		return v != v;
	}

    inline bool IsNan(double v)
    {
        return v != v;
    }

	class Vector3D;
	class Vector4D;

    class dVector3D;
    class dVector4D;

	/// Represents a four by four matrix and its operations.
	class KCL_API Matrix4x4
	{
	public:
		union
		{
			float v[16];
			struct
			{
				float v11, v12, v13, v14,
					v21, v22, v23, v24,
					v31, v32, v33, v34,
					v41, v42, v43, v44;
			};
		};
		//Matrix4x4 ();
		Matrix4x4 ( float m11 = 1, float m12 = 0, float m13 = 0, float m14 = 0, 
					float m21 = 0, float m22 = 1, float m23 = 0, float m24 = 0, 
					float m31 = 0, float m32 = 0, float m33 = 1, float m34 = 0, 
					float m41 = 0, float m42 = 0, float m43 = 0, float m44 = 1);

		Matrix4x4 &operator*= (const Matrix4x4 &M);
		friend Matrix4x4 operator* (const Matrix4x4 &A, const Matrix4x4 &B);
		Matrix4x4 &operator+= (const Matrix4x4 &M);
		friend Matrix4x4 operator+ (const Matrix4x4 &A, const Matrix4x4 &B);
        bool operator== (const Matrix4x4 &M) const;
        bool operator!= (const Matrix4x4 &M) const;
		friend Vector3D mult4x3 (const Matrix4x4 &A, const Vector3D &v);
		friend void mult4x3 (const Matrix4x4 &A, const Vector3D &v, Vector3D &result);
		friend void mult4x4 (const Matrix4x4 &A, const Vector3D &v, Vector3D &result);
		friend void mult4x4 (const Matrix4x4 &A, const Vector3D &v, Vector4D &result);
		friend void mult4x4 (const Matrix4x4 &A, const Vector4D &v, Vector4D &result);
		friend Vector3D mult3x3 (const Matrix4x4 &A, const Vector3D &v);
		friend void mult3x3 (const Matrix4x4 &A, const Vector3D &v, Vector3D &result);
		operator const float *() const;

		void getNormalMatrix (Matrix4x4 &normal) const;
		float determinant3x3 () const;
		void invert4x3 ();
		void transpose ();
		void identity ();
		void zero ();
		void rotate (const float angle, const Vector3D &axis, const bool normalize = false);
		void rotateTo (const float angle, const Vector3D &axis, const bool normalize = false, int axisLimit = 0);
		void translate (const Vector3D &v);
		void scale (const Vector3D &s);
		void lookat (const Vector3D &eye, const Vector3D &center, const Vector3D &up);
		void perspective (const float fovy, const float aspect, const float zNear, const float zFar);
		void perspective (float fov, float aspect, float zNear);
		void reflection( const Vector4D &plane);

		bool IsNan() const;

		Vector3D getTranslation () const;
		Vector3D getScale () const;
		
		
		static void Scale_translate_to_fit (Matrix4x4& output, const Vector3D& vMin, const Vector3D& vMax);
		void adjustNearPlaneForPerspective( const Vector4D &clipPlane);

		static void Zero (Matrix4x4 &M);
		static void Identity (Matrix4x4 &M);
		static void Rotate (Matrix4x4 &M, float angle, const Vector3D &axis, bool normalize = false);
		static void RotateX(Matrix4x4 &M, float angle);
		static void RotateY(Matrix4x4 &M, float angle);
		static void RotateZ(Matrix4x4 &M, float angle);
		static void Translate (Matrix4x4 &M, const Vector3D &v);
		static void Scale (Matrix4x4 &M, const Vector3D &scale);
		static void LookAt (Matrix4x4 &M, const Vector3D &eye, const Vector3D &center, const Vector3D &up);
		static void Perspective (Matrix4x4 &M, float fovy, float aspect, float zNear, float zFar);
		static void PerspectiveGL(Matrix4x4 &M, float fovy, float aspect, float zNear, float zFar);
		static void Frustum (Matrix4x4 &M, float left, float right, float bottom, float top, float zNear, float zFar);
		static void Ortho (Matrix4x4 &M, float left, float right, float bottom, float top, float zNear, float zFar);
		static void RotationMatrixFromDirection(Matrix4x4 &M, KCL::Vector3D direction, const KCL::Vector3D &up);

		static float Determinant3x3 (const Matrix4x4 &M);
		static Matrix4x4 Invert4x3 (const Matrix4x4 &M);
		static void InvertModelView (const Matrix4x4 &modelView , Matrix4x4 &outM);
		static bool Invert4x4(const Matrix4x4 &matrix, Matrix4x4 &resultMatrix);
		//static void Print (const Matrix4x4 &M);
	};

	/// Three-component vectors.
	class KCL_API Vector3D
	{
	public:
		Vector3D ();
		Vector3D (const float nx, const float ny, const float nz);
		explicit Vector3D (const float v[]);
		explicit Vector3D (const Vector4D &v);
		Vector3D (const Vector3D &v);

		Vector3D &operator= (const Vector3D &v);
		Vector3D &operator+= (const Vector3D &v);
		Vector3D &operator-= (const Vector3D &v);
		Vector3D &operator*= (const float &f);
		Vector3D &operator/= (const float &f);
		bool operator== (const Vector3D &v) const;
		bool operator!= (const Vector3D &v) const;
		operator const float *() const;

		friend KCL_API Vector3D operator- (const Vector3D &v);
		friend KCL_API Vector3D operator+ (const Vector3D &a, const Vector3D &b);
		friend KCL_API Vector3D operator- (const Vector3D &a, const Vector3D &b);
		friend KCL_API Vector3D operator* (const Vector3D &v, const float factor);
		friend KCL_API Vector3D operator* (const float factor, const Vector3D &v);
		friend KCL_API Vector3D operator* (const Vector3D &a, const Vector3D &b);
		friend KCL_API Vector3D operator/ (const Vector3D &v, const float factor);

		Vector3D& normalize ();
		float length () const;
		float length2 () const;
		void set (const float nx, const float ny, const float nz);
		void set (const float v[]);
		void PackTo01 ();
		

		static Vector3D &normalize (Vector3D &v);
		static float length (const Vector3D &v);
		static float length2 (const Vector3D &v);
		static float distance (const Vector3D &a, const Vector3D &b);
		static float distance2 (const Vector3D &a, const Vector3D &b);
		static float dot (const Vector3D &a, const Vector3D &b);
		static Vector3D cross (const Vector3D &a, const Vector3D &b);
		static Vector3D interpolate (const Vector3D &a, const Vector3D &b, const float t);


		union
		{
			float v[3];
			struct
			{
				float x, y, z;
			};
		};

	};

	/// Four-component vectors.
	class KCL_API Vector4D
	{
	public:
		Vector4D ();
		explicit Vector4D (const Vector3D &v);
		Vector4D (const Vector3D &v, float nw);
		Vector4D (const float nx, const float ny, const float nz, const float nw);
		Vector4D (const float v[]);

		bool operator== (const Vector4D &v) const;
		bool operator!= (const Vector4D &v) const;
		Vector4D &operator/= (const float &f);
		operator const float *() const;

		friend KCL_API Vector4D operator- (const Vector4D &v);
		friend KCL_API Vector4D operator+ (const Vector4D &a, const Vector4D &b);
		friend KCL_API Vector4D operator- (const Vector4D &a, const Vector4D &b);
		friend KCL_API Vector4D operator* (const Vector4D &v, const float factor);
		friend KCL_API Vector4D operator* (const float factor, const Vector4D &v);
		friend KCL_API Vector4D operator* (const Matrix4x4 &M, const Vector4D &v);
		friend KCL_API Vector4D operator/ (const Vector4D &v, const float factor);

		Vector4D& homogenize ();

		void set (const float nx, const float ny, const float nz, const float nw);
		void set(const Vector3D &v, float w_);
		void set(const float v[]);

		float length3 ();

		static Vector4D interpolate (const Vector4D &a, const Vector4D &b, const float t);
		static float dot (const Vector4D &a, const Vector4D &b);

		union
		{
			float v[4];
			struct
			{
				float x, y, z, w;
			};
		};
	};

	/// Two-component vectors.
	class KCL_API Vector2D
	{
	public:
		union
		{
			float v[2];
			struct
			{
				float x, y;
			};
		};

		Vector2D () : x(0), y(0) {}
		explicit Vector2D (const Vector3D &v) : x(v.x), y(v.x) {}
		explicit Vector2D (const Vector4D &v) : x(v.x), y(v.y) {}
		Vector2D (float nx, float ny) : x(nx), y(ny) {}
		void set (float nx, float ny) { x = nx; y = ny; }
	};


	/// Quaternions for animation.
	class KCL_API Quaternion
	{
	public:
		Quaternion ();
		Quaternion (const float nx, const float ny, const float nz, const float nw);
		Quaternion (const float *v);

		Quaternion &operator*= (const float &f);

		friend KCL_API Quaternion operator- (const Quaternion &a);
		friend KCL_API Quaternion operator+ (const Quaternion &a, const Quaternion &b);
		friend KCL_API Quaternion operator- (const Quaternion &a, const Quaternion &b);
		friend KCL_API Quaternion operator* (const Quaternion &v, const float factor);
		friend KCL_API Quaternion operator* (const float factor, const Quaternion &v);

		void set (float w, float x, float y, float z);
		void fromAngleAxis (const float angle, const Vector3D &axis);
		void toAngleAxis (float &angle, Vector3D &axis) const;
		void getRotationMatrix(Matrix4x4 &m);

		static void Interpolate (Quaternion q1, Quaternion q2, float t, Quaternion &result);
		static float Dot (const Quaternion &q1, const Quaternion &q2);

		float w, x, y, z;
	};

	inline float Quaternion::Dot (const Quaternion &q1, const Quaternion &q2)
	{
		return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
	}

	KCL_API extern const KCL::Matrix4x4 kIdentityMatrix4x4;
	KCL_API extern const KCL::Vector3D kZeroVector3D;
	KCL_API extern const KCL::Vector4D kZeroVector4D;

	/// Namespace for basic math utility functions.
	namespace Math
	{
		const float kPi = 3.14159265f;
		const float k2Pi = kPi * 2.0f;
		const float kPiOver2 = kPi / 2.0f;
		const float k1OverPi = 1.0f / kPi;
		const float k1Over2Pi = 1.0f / k2Pi;
		const float kPiOver180 = kPi / 180.0f;
		const float k180OverPi = 180.0f / kPi;
		const float kEpsilon = 0.0000001f;

		template <typename T>
		void swap (T& a, T& b)
		{
			T tmp = a;
			a = b;
			b = tmp;
		}

        KCL_API double interpolate(const double a, const double b, const double t);
        KCL_API float interpolate(const float a, const float b, const float t);

		KCL_API float Rad (const float deg);
		KCL_API float Deg (const float rad);
		KCL_API bool Odd (const KCL::uint32 x);
		KCL_API bool Even (const KCL::uint32 x);
		template <typename T> const T& Max(const T& a, const T& b); 
		template <typename T> const T& Min(const T& a, const T& b);
		template <typename T> const T& Clamp(const T& x, const T& l, const T& h);

		float randomf (int *seed);
		float randomf_signed (int *seed);
		float randomf_from_range (int *seed, float min, float max);
	}


	/******************************************************
	 * Matrix4x4 implementation
	 ******************************************************/

	//inline Matrix4x4::Matrix4x4 () :
	//	v11(1.0f), v12(0.0f), v13(0.0f), v14(0.0f),
	//	v21(0.0f), v22(1.0f), v23(0.0f), v24(0.0f),
	//	v31(0.0f), v32(0.0f), v33(1.0f), v34(0.0f),
	//	v41(0.0f), v42(0.0f), v43(0.0f), v44(1.0f)
	//{
	//}


	inline Matrix4x4::Matrix4x4 (float m11, float m12, float m13, float m14,
								 float m21, float m22, float m23, float m24,
								 float m31, float m32, float m33, float m34,
								 float m41, float m42, float m43, float m44) :
		v11(m11), v12(m12), v13(m13), v14(m14),
		v21(m21), v22(m22), v23(m23), v24(m24),
		v31(m31), v32(m32), v33(m33), v34(m34),
		v41(m41), v42(m42), v43(m43), v44(m44)
	{
	}


	inline Matrix4x4 operator* (const Matrix4x4 &A, const Matrix4x4 &B)
	{
		return Matrix4x4
		(
			A.v11*B.v11 + A.v12*B.v21 + A.v13*B.v31 + A.v14*B.v41,
			A.v11*B.v12 + A.v12*B.v22 + A.v13*B.v32 + A.v14*B.v42,
			A.v11*B.v13 + A.v12*B.v23 + A.v13*B.v33 + A.v14*B.v43,
			A.v11*B.v14 + A.v12*B.v24 + A.v13*B.v34 + A.v14*B.v44,

			A.v21*B.v11 + A.v22*B.v21 + A.v23*B.v31 + A.v24*B.v41,
			A.v21*B.v12 + A.v22*B.v22 + A.v23*B.v32 + A.v24*B.v42,
			A.v21*B.v13 + A.v22*B.v23 + A.v23*B.v33 + A.v24*B.v43,
			A.v21*B.v14 + A.v22*B.v24 + A.v23*B.v34 + A.v24*B.v44,

			A.v31*B.v11 + A.v32*B.v21 + A.v33*B.v31 + A.v34*B.v41,
			A.v31*B.v12 + A.v32*B.v22 + A.v33*B.v32 + A.v34*B.v42,
			A.v31*B.v13 + A.v32*B.v23 + A.v33*B.v33 + A.v34*B.v43,
			A.v31*B.v14 + A.v32*B.v24 + A.v33*B.v34 + A.v34*B.v44,

			A.v41*B.v11 + A.v42*B.v21 + A.v43*B.v31 + A.v44*B.v41,
			A.v41*B.v12 + A.v42*B.v22 + A.v43*B.v32 + A.v44*B.v42,
			A.v41*B.v13 + A.v42*B.v23 + A.v43*B.v33 + A.v44*B.v43,
			A.v41*B.v14 + A.v42*B.v24 + A.v43*B.v34 + A.v44*B.v44
		);
	}

	/*
	inline Matrix4x4 operator* (const Matrix4x4 &A, const Matrix4x4 &B)
	{
		Matrix4x4 r;

		float nv11 = A.v11*B.v11 + A.v12*B.v21 + A.v13*B.v31 + A.v14*B.v41;
		float nv12 = A.v11*B.v12 + A.v12*B.v22 + A.v13*B.v32 + A.v14*B.v42;
		float nv13 = A.v11*B.v13 + A.v12*B.v23 + A.v13*B.v33 + A.v14*B.v43;
		float nv14 = A.v11*B.v14 + A.v12*B.v24 + A.v13*B.v34 + A.v14*B.v44;

		float nv21 = A.v21*B.v11 + A.v22*B.v21 + A.v23*B.v31 + A.v24*B.v41;
		float nv22 = A.v21*B.v12 + A.v22*B.v22 + A.v23*B.v32 + A.v24*B.v42;
		float nv23 = A.v21*B.v13 + A.v22*B.v23 + A.v23*B.v33 + A.v24*B.v43;
		float nv24 = A.v21*B.v14 + A.v22*B.v24 + A.v23*B.v34 + A.v24*B.v44;

		float nv31 = A.v31*B.v11 + A.v32*B.v21 + A.v33*B.v31 + A.v34*B.v41;
		float nv32 = A.v31*B.v12 + A.v32*B.v22 + A.v33*B.v32 + A.v34*B.v42;
		float nv33 = A.v31*B.v13 + A.v32*B.v23 + A.v33*B.v33 + A.v34*B.v43;
		float nv34 = A.v31*B.v14 + A.v32*B.v24 + A.v33*B.v34 + A.v34*B.v44;

		float nv41 = A.v41*B.v11 + A.v42*B.v21 + A.v43*B.v31 + A.v44*B.v41;
		float nv42 = A.v41*B.v12 + A.v42*B.v22 + A.v43*B.v32 + A.v44*B.v42;
		float nv43 = A.v41*B.v13 + A.v42*B.v23 + A.v43*B.v33 + A.v44*B.v43;
		float nv44 = A.v41*B.v14 + A.v42*B.v24 + A.v43*B.v34 + A.v44*B.v44;

		r.v11 = nv11; r.v12 = nv12; r.v13 = nv13; r.v14 = nv14;
		r.v21 = nv21; r.v22 = nv22; r.v23 = nv23; r.v24 = nv24;
		r.v31 = nv31; r.v32 = nv32; r.v33 = nv33; r.v34 = nv34;
		r.v41 = nv41; r.v42 = nv42; r.v43 = nv43; r.v44 = nv44;

		return r;
	}
	*/

	inline void Matrix4x4::translate (const Vector3D &v)
	{
		v41 = v11*v.x + v21*v.y + v31*v.z + v41;
		v42 = v12*v.x + v22*v.y + v32*v.z + v42;
		v43 = v13*v.x + v23*v.y + v33*v.z + v43;
		v44 = v14*v.x + v24*v.y + v34*v.z + v44;
	}


	inline void Matrix4x4::Translate (Matrix4x4 &M, const Vector3D &v)
	{
		Identity (M);
		M.v41 = v.x; M.v42 = v.y; M.v43 = v.z;
	}

	inline Vector3D &Vector3D::operator*= (const float &f)
	{
		x *= f; y *= f; z *= f;
		return *this;
	}

	inline Vector3D mult3x3 (const Matrix4x4 &A, const Vector3D &v)
	{
		return Vector3D (
			A.v11*v.x + A.v21*v.y + A.v31*v.z,
			A.v12*v.x + A.v22*v.y + A.v32*v.z,
			A.v13*v.x + A.v23*v.y + A.v33*v.z
			);
	}

	inline void mult3x3 (const Matrix4x4 &A, const Vector3D &v, Vector3D &result)
	{
		result.x = A.v11*v.x + A.v21*v.y + A.v31*v.z;
		result.y = A.v12*v.x + A.v22*v.y + A.v32*v.z;
		result.z = A.v13*v.x + A.v23*v.y + A.v33*v.z;
	}


	inline void mult4x3 (const Matrix4x4 &A, const Vector3D &v, Vector3D &result)
	{
		result.x = A.v11*v.x + A.v21*v.y + A.v31*v.z + A.v41;
		result.y = A.v12*v.x + A.v22*v.y + A.v32*v.z + A.v42;
		result.z = A.v13*v.x + A.v23*v.y + A.v33*v.z + A.v43;
	}


	inline void mult4x4 (const Matrix4x4 &A, const Vector3D &v, Vector3D &result)
	{
		const float w = 1.0f / (A.v14*v.x + A.v24*v.y + A.v34*v.z + A.v44);
		result.x = (A.v11*v.x + A.v21*v.y + A.v31*v.z + A.v41)*w;
		result.y = (A.v12*v.x + A.v22*v.y + A.v32*v.z + A.v42)*w;
		result.z = (A.v13*v.x + A.v23*v.y + A.v33*v.z + A.v43)*w;
	}


	inline void mult4x4 (const Matrix4x4 &A, const Vector3D &v, Vector4D &result)
	{
		result.x = A.v11*v.x + A.v21*v.y + A.v31*v.z + A.v41;
		result.y = A.v12*v.x + A.v22*v.y + A.v32*v.z + A.v42;
		result.z = A.v13*v.x + A.v23*v.y + A.v33*v.z + A.v43;
		result.w = A.v14*v.x + A.v24*v.y + A.v34*v.z + A.v44;
	}


	inline void mult4x4 (const Matrix4x4 &A, const Vector4D &v, Vector4D &result)
	{
		result.x = A.v11*v.x + A.v21*v.y + A.v31*v.z + A.v41*v.w;
		result.y = A.v12*v.x + A.v22*v.y + A.v32*v.z + A.v42*v.w;
		result.z = A.v13*v.x + A.v23*v.y + A.v33*v.z + A.v43*v.w;
		result.w = A.v14*v.x + A.v24*v.y + A.v34*v.z + A.v44*v.w;
	}


	inline void Matrix4x4::Rotate (Matrix4x4 &M, float angle, const Vector3D &axis, bool normalize)
	{
		const float rad = Math::Rad (angle);
		const float co = cos (rad);
		const float si = sin (rad);
		Vector3D n (axis);
		if (normalize) {
			n.normalize ();
		}
		const float nx = n.x;
		const float ny = n.y;
		const float nz = n.z;

		Identity (M);

		M.v11 = nx*nx*(1.0f - co) + co;
		M.v12 = (1.0f - co)*nx*ny + si*nz;
		M.v13 = (1.0f - co)*nx*nz - si*ny;

		M.v21 = (1.0f - co)*nx*ny - si*nz;
		M.v22 = ny*ny*(1.0f - co) + co;
		M.v23 = (1.0f - co)*ny*nz + si*nx;

		M.v31 = (1.0f - co)*nx*nz + si*ny;
		M.v32 = (1.0f - co)*ny*nz - si*nx;
		M.v33 = nz*nz*(1.0f - co) + co;
	}


	inline void Matrix4x4::RotateX (Matrix4x4 &M, float angle)
	{
		const float rad = Math::Rad (angle);
		const float ca = cos (rad);
		const float sa = sin (rad);

		Zero (M);
		M.v44 = 1.0f;

		M.v11 = 1.0f;

		M.v22 = ca;
		M.v23 = -sa;

		M.v32 = sa;
		M.v33 = ca;
	}


	inline void Matrix4x4::RotateY (Matrix4x4 &M, float angle)
	{
		const float rad = Math::Rad (angle);
		const float ca = cos (rad);
		const float sa = sin (rad);

		Zero (M);
		M.v44 = 1.0f;

		M.v11 = ca;
		M.v13 = sa;

		M.v22 = 1.0f;

		M.v31 = -sa;
		M.v33 = ca;
	}


	inline void Matrix4x4::RotateZ (Matrix4x4 &M, float angle)
	{
		const float rad = Math::Rad (angle);
		const float ca = cos (rad);
		const float sa = sin (rad);

		Zero (M);
		M.v44 = 1.0f;

		M.v11 = ca;
		M.v12 = -sa;

		M.v21 = sa;
		M.v22 = ca;

		M.v33 = 1.0f;
	}


	inline Vector3D Matrix4x4::getTranslation () const
	{
		return Vector3D (v41, v42, v43);
	}


	inline void Matrix4x4::Identity (Matrix4x4 &M)
	{
		memset(&M.v11,0,sizeof(float)*16);
		M.v11=1.0f;
		M.v22=1.0f;
		M.v33=1.0f;
		M.v44=1.0f;
	}

	inline void Matrix4x4::identity ()
	{
		Identity (*this);
	}


	inline void Matrix4x4::Zero (Matrix4x4 &M)
	{
		memset(&M.v11,0,sizeof(float)*16);
	}


	inline void Matrix4x4::Scale (Matrix4x4 &M, const Vector3D &scale)
	{
		Identity (M);
		M.v11 = scale.x; M.v22 = scale.y; M.v33 = scale.z;
	}


	/******************************************************
	 * Vector3D implementation
	 ******************************************************/

	inline Vector3D::Vector3D() : x(0.0f), y(0.0f), z(0.0f)
	{
	}


	inline Vector3D::Vector3D (const float nx, const float ny, const float nz) : x(nx), y(ny), z(nz)
	{
	}


	inline Vector3D::Vector3D (const Vector3D &v) : x(v.x), y(v.y), z(v.z)
	{
	}


	inline Vector3D &Vector3D::operator= (const Vector3D &v)
	{
		x = v.x; y = v.y; z = v.z;
		return *this;
	}


	inline Vector3D &Vector3D::operator+= (const Vector3D &v)
	{
		x += v.x; y += v.y; z += v.z;
		return *this;
	}


	inline Vector3D &Vector3D::operator-= (const Vector3D &v)
	{
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}


	inline Vector3D &Vector3D::operator/= (const float &f)
	{
		x /= f; y /= f; z /= f;
		return *this;
	}


	inline bool Vector3D::operator== (const Vector3D &v) const
	{
		 return x==v.x && y==v.y && z==v.z;
	}


	inline bool Vector3D::operator!= (const Vector3D &v) const
	{
		 return x!=v.x || y!=v.y || z!=v.z;;
	}



	inline Vector3D operator- (const Vector3D &v)
	{
		return Vector3D (-v.x, -v.y, -v.z);
	}


	inline Vector3D operator+ (const Vector3D &a, const Vector3D &b)
	{
		return Vector3D (a.x + b.x, a.y + b.y, a.z + b.z);
	}


	inline Vector3D operator- (const Vector3D &a, const Vector3D &b)
	{
		return Vector3D (a.x - b.x, a.y - b.y, a.z - b.z);
	}

	inline Vector3D operator/ (const Vector3D &v, const float factor)
	{
		return Vector3D (v.x / factor, v.y / factor, v.z / factor);
	}


	inline Vector3D operator* (const Vector3D &a, const Vector3D &b)
	{
		return Vector3D( a.x * b.x, a.y * b.y, a.z * b.z);
	}

	inline float Vector3D::length () const
	{
		return sqrt (x*x + y*y + z*z);
	}

	inline float Vector3D::length2 () const
	{
		return x*x + y*y + z*z;
	}


	inline void Vector3D::set (const float nx, const float ny, const float nz)
	{
		x = nx; y = ny; z = nz;
	}


	inline void Vector3D::set (const float *v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}


	inline void Vector3D::PackTo01 ()
	{
		for( KCL::uint32 i=0; i<3; i++)
		{
			v[i] = (v[i] + 1.0f) * 0.5f;
		}
	}


	inline Vector3D &Vector3D::normalize (Vector3D &v)
	{
		return v.normalize();
	}


	inline float Vector3D::length (const Vector3D &v)
	{
		return sqrt (v.x*v.x + v.y*v.y + v.z*v.z);
	}

	inline float Vector3D::length2 (const Vector3D &v)
	{
		return v.x*v.x + v.y*v.y + v.z*v.z;
	}

	inline Vector3D Vector3D::cross (const Vector3D &a, const Vector3D &b)
	{
		return Vector3D (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
	}


	inline Vector3D& Vector3D::normalize ()
	{
		float L = length();
		if (L > Math::kEpsilon)
		{
			float d = 1.0f / L;
			x *= d; y *= d; z *= d;
		}
		return *this;
	}


	inline float Vector3D::distance (const Vector3D &a, const Vector3D &b)
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		float dz = a.z - b.z;
		return sqrt (dx*dx + dy*dy + dz*dz);
	}

	inline float Vector3D::distance2 (const Vector3D &a, const Vector3D &b)
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		float dz = a.z - b.z;
		return dx*dx + dy*dy + dz*dz;
	}

	inline Vector4D &Vector4D::operator/= (const float &f)
	{
		x /= f; y /= f; z /= f; w /= f;
		return *this;
	}


	/******************************************************
	 * Math utils implementation
	 ******************************************************/
	inline float sgn(float a)
	{
		if (a > 0.0f)
		{
			return 1.0f;
		}
		if (a < 0.0f)
		{
			return -1.0f;
		}
		return 0.0f;
	}

    inline float Math::interpolate(const float a, const float b, const float t)
    {
        return (t*b + (1.0f - t)*a);
    }

    inline double Math::interpolate(const double a, const double b, const double t)
    {
        return (t*b + (1.0f - t)*a);
    }

	inline float Math::Rad (const float deg)
	{
		return deg * kPiOver180;
	}

	inline float Math::Deg (const float rad)
	{
		return rad * k180OverPi;
	}

	inline bool Math::Odd (const KCL::uint32 x)
	{
		return x%2 == 1;
	}

	inline bool Math::Even (const KCL::uint32 x)
	{
		return x%2 == 0;
	}

	template <typename T>
	const T& Max(const T& a, const T& b)
	{
		return a>=b? a:b;
	}

	template <typename T>
	inline const T& Min(const T& a, const T& b)
	{
		return a<=b? a:b;
	}

	template <typename T>
	const T& Clamp(const T&x, const T& l, const T& h)
	{
		return Max(l,Min(x,h));
	}

	template <typename T>
	void minmax (const T* v, KCL::uint32 n, T& min, T& max, KCL::uint32 &minLoc, KCL::uint32 &maxLoc)
	{
		if (n == 0) return;

		v[0] = min;
		v[0] = max;

		for (KCL::uint32 i = 0; i < n; i++) {
			if (v[i] > max) max = v[i];
			if (v[i] < min) min = v[i];
		}
	}

	int texture_levels(int width, int height);



    class KCL_API dMatrix4x4
    {
    public:
        union
        {
            double v[16];
            struct
            {
                double v11, v12, v13, v14,
                    v21, v22, v23, v24,
                    v31, v32, v33, v34,
                    v41, v42, v43, v44;
            };
        };
        //dMatrix4x4 ();
        dMatrix4x4(double m11 = 1, double m12 = 0, double m13 = 0, double m14 = 0,
            double m21 = 0, double m22 = 1, double m23 = 0, double m24 = 0,
            double m31 = 0, double m32 = 0, double m33 = 1, double m34 = 0,
            double m41 = 0, double m42 = 0, double m43 = 0, double m44 = 1);
        dMatrix4x4(KCL::Matrix4x4& M);

        dMatrix4x4 &operator*= (const dMatrix4x4 &M);
        friend dMatrix4x4 operator* (const dMatrix4x4 &A, const dMatrix4x4 &B);
        dMatrix4x4 &operator+= (const dMatrix4x4 &M);
        friend dMatrix4x4 operator+ (const dMatrix4x4 &A, const dMatrix4x4 &B);
        bool operator== (const dMatrix4x4 &M) const;
        bool operator!= (const dMatrix4x4 &M) const;
        friend dVector3D mult4x3(const dMatrix4x4 &A, const dVector3D &v);
        friend void mult4x3(const dMatrix4x4 &A, const dVector3D &v, dVector3D &result);
        friend void mult4x4(const dMatrix4x4 &A, const dVector3D &v, dVector3D &result);
        friend void mult4x4(const dMatrix4x4 &A, const dVector3D &v, dVector4D &result);
        friend void mult4x4(const dMatrix4x4 &A, const dVector4D &v, dVector4D &result);
        friend dVector3D mult3x3(const dMatrix4x4 &A, const dVector3D &v);
        friend void mult3x3(const dMatrix4x4 &A, const dVector3D &v, dVector3D &result);
        operator const double *() const;

        void getNormalMatrix(dMatrix4x4 &normal) const;
        double determinant3x3() const;
        void invert4x3();
        void transpose();
        void identity();
        void zero();
        void rotate(const double angle, const dVector3D &axis, const bool normalize = false);
        void rotateTo(const double angle, const dVector3D &axis, const bool normalize = false, int axisLimit = 0);
        void translate(const dVector3D &v);
        void scale(const dVector3D &s);
        void lookat(const dVector3D &eye, const dVector3D &center, const dVector3D &up);
        void perspective(const double fovy, const double aspect, const double zNear, const double zFar);
        void perspective(double fov, double aspect, double zNear);
        void reflection(const dVector4D &plane);

        bool IsNan() const;

        dVector3D getTranslation() const;
        dVector3D getScale() const;


        static void Scale_translate_to_fit(dMatrix4x4& output, const dVector3D& vMin, const dVector3D& vMax);
        void adjustNearPlaneForPerspective(const dVector4D &clipPlane);

        static void Zero(dMatrix4x4 &M);
        static void Identity(dMatrix4x4 &M);
        static void Rotate(dMatrix4x4 &M, double angle, const dVector3D &axis, bool normalize = false);
        static void RotateX(dMatrix4x4 &M, double angle);
        static void RotateY(dMatrix4x4 &M, double angle);
        static void RotateZ(dMatrix4x4 &M, double angle);
        static void Translate(dMatrix4x4 &M, const dVector3D &v);
        static void Scale(dMatrix4x4 &M, const dVector3D &scale);
        static void LookAt(dMatrix4x4 &M, const dVector3D &eye, const dVector3D &center, const dVector3D &up);
        static void Perspective(dMatrix4x4 &M, double fovy, double aspect, double zNear, double zFar);
        static void Frustum(dMatrix4x4 &M, double left, double right, double bottom, double top, double zNear, double zFar);
        static void Ortho(dMatrix4x4 &M, double left, double right, double bottom, double top, double zNear, double zFar);
        static void RotationMatrixFromDirection(dMatrix4x4 &M, KCL::dVector3D direction, const KCL::dVector3D &up);

        static double Determinant3x3(const dMatrix4x4 &M);
        static dMatrix4x4 Invert4x3(const dMatrix4x4 &M);
        static void InvertModelView(const dMatrix4x4 &modelView, dMatrix4x4 &outM);
        static bool Invert4x4(const dMatrix4x4 &matrix, dMatrix4x4 &resultMatrix);
        //static void Print (const dMatrix4x4 &M);
    };

    class KCL_API dVector3D
    {
    public:
        dVector3D();
        dVector3D(const double nx, const double ny, const double nz);
        explicit dVector3D(const double v[]);
        explicit dVector3D(const dVector4D &v);
        dVector3D(const dVector3D &v);
        dVector3D(const Vector3D &v);

        dVector3D &operator= (const dVector3D &v);
        dVector3D &operator+= (const dVector3D &v);
        dVector3D &operator-= (const dVector3D &v);
        dVector3D &operator*= (const double &f);
        dVector3D &operator/= (const double &f);
        bool operator== (const dVector3D &v) const;
        bool operator!= (const dVector3D &v) const;
        operator const double *() const;

        friend KCL_API dVector3D operator- (const dVector3D &v);
        friend KCL_API dVector3D operator+ (const dVector3D &a, const dVector3D &b);
        friend KCL_API dVector3D operator- (const dVector3D &a, const dVector3D &b);
        friend KCL_API dVector3D operator* (const dVector3D &v, const double factor);
        friend KCL_API dVector3D operator* (const double factor, const dVector3D &v);
        friend KCL_API dVector3D operator* (const dVector3D &a, const dVector3D &b);
        friend KCL_API dVector3D operator/ (const dVector3D &v, const double factor);

        dVector3D& normalize();
        double length() const;
        double length2() const;
        void set(const double nx, const double ny, const double nz);
        void set(const double v[]);
        void PackTo01();


        static dVector3D &normalize(dVector3D &v);
        static double length(const dVector3D &v);
        static double length2(const dVector3D &v);
        static double distance(const dVector3D &a, const dVector3D &b);
        static double distance2(const dVector3D &a, const dVector3D &b);
        static double dot(const dVector3D &a, const dVector3D &b);
        static dVector3D cross(const dVector3D &a, const dVector3D &b);
        static dVector3D interpolate(const dVector3D &a, const dVector3D &b, const double t);


        union
        {
            double v[3];
            struct
            {
                double x, y, z;
            };
        };

    };

    class KCL_API dVector4D
    {
    public:
        dVector4D();
        explicit dVector4D(const dVector3D &v);
        dVector4D(const dVector3D &v, double nw);
        dVector4D(const double nx, const double ny, const double nz, const double nw);
        dVector4D(const double v[]);

        bool operator== (const dVector4D &v) const;
        bool operator!= (const dVector4D &v) const;
        dVector4D &operator/= (const double &f);
        operator const double *() const;

        friend KCL_API dVector4D operator- (const dVector4D &v);
        friend KCL_API dVector4D operator+ (const dVector4D &a, const dVector4D &b);
        friend KCL_API dVector4D operator- (const dVector4D &a, const dVector4D &b);
        friend KCL_API dVector4D operator* (const dVector4D &v, const double factor);
        friend KCL_API dVector4D operator* (const double factor, const dVector4D &v);
        friend KCL_API dVector4D operator* (const Matrix4x4 &M, const dVector4D &v);
        friend KCL_API dVector4D operator/ (const dVector4D &v, const double factor);

        dVector4D& homogenize();

        void set(const double nx, const double ny, const double nz, const double nw);
        void set(const dVector3D &v, double w_);
        void set(const double v[]);

        double length3();

        static dVector4D interpolate(const dVector4D &a, const dVector4D &b, const double t);
        static double dot(const dVector4D &a, const dVector4D &b);

        union
        {
            double v[4];
            struct
            {
                double x, y, z, w;
            };
        };
    };

    class KCL_API dVector2D
    {
    public:
        union
        {
            double v[2];
            struct
            {
                double x, y;
            };
        };

        dVector2D() : x(0), y(0) {}
        explicit dVector2D(const Vector3D &v) : x(v.x), y(v.x) {}
        explicit dVector2D(const Vector4D &v) : x(v.x), y(v.y) {}
        dVector2D(double nx, double ny) : x(nx), y(ny) {}
        void set(double nx, double ny) { x = nx; y = ny; }

        static dVector2D interpolate(const dVector2D &a, const dVector2D &b, const double t);
        static double dot(const dVector2D &a, const dVector2D &b);

        bool operator== (const dVector2D &v) const;
        bool operator!= (const dVector2D &v) const;
        dVector2D &operator/= (const double &f);
        operator const double *() const;

        friend KCL_API dVector2D operator- (const dVector2D &v);
        friend KCL_API dVector2D operator+ (const dVector2D &a, const dVector2D &b);
        friend KCL_API dVector2D operator- (const dVector2D &a, const dVector2D &b);
        friend KCL_API dVector2D operator* (const dVector2D &v, const double factor);
        friend KCL_API dVector2D operator* (const double factor, const dVector2D &v);
        friend KCL_API dVector2D operator* (const Matrix4x4 &M, const dVector2D &v);
        friend KCL_API dVector2D operator* (const dVector2D &v1, const dVector2D &v2);
        friend KCL_API dVector2D operator/ (const dVector2D &v, const double factor);
    };

    inline dMatrix4x4::dMatrix4x4(KCL::Matrix4x4& M)
    {
        for (int i = 0; i < 16; ++i)
        {
            v[i] = M.v[i];
        }
    }

    inline dVector3D &dVector3D::operator*= (const double &d)
    {
        x *= d; y *= d; z *= d;
        return *this;
    }

    inline void mult4x4(const dMatrix4x4 &A, const dVector4D &v, dVector4D &result)
    {
        result = dVector4D(
            A.v11*v.x + A.v21*v.y + A.v31*v.z + A.v41,
            A.v12*v.x + A.v22*v.y + A.v32*v.z + A.v42,
            A.v13*v.x + A.v23*v.y + A.v33*v.z + A.v43,
            A.v14*v.x + A.v24*v.y + A.v34*v.z + A.v44
            );
    }

	/******************************************************
	 * dVector3D implementation
	 ******************************************************/

    inline dVector3D::dVector3D() : x(0.0f), y(0.0f), z(0.0f)
    {
    }


    inline dVector3D::dVector3D(const double nx, const double ny, const double nz) : x(nx), y(ny), z(nz)
    {
    }


    inline dVector3D::dVector3D(const dVector3D &v) : x(v.x), y(v.y), z(v.z)
    {
    }

    inline dVector3D::dVector3D(const Vector3D &v) : x(v.x), y(v.y), z(v.z)
    {
    }


    inline dVector3D &dVector3D::operator= (const dVector3D &v)
    {
        x = v.x; y = v.y; z = v.z;
        return *this;
    }


    inline dVector3D &dVector3D::operator+= (const dVector3D &v)
    {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }


    inline dVector3D &dVector3D::operator-= (const dVector3D &v)
    {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }


    inline dVector3D &dVector3D::operator/= (const double &f)
    {
        x /= f; y /= f; z /= f;
        return *this;
    }


    inline bool dVector3D::operator== (const dVector3D &v) const
    {
        return x == v.x && y == v.y && z == v.z;
    }


    inline bool dVector3D::operator!= (const dVector3D &v) const
    {
        return x != v.x || y != v.y || z != v.z;;
    }



    inline dVector3D operator- (const dVector3D &v)
    {
        return dVector3D(-v.x, -v.y, -v.z);
    }


    inline dVector3D operator+ (const dVector3D &a, const dVector3D &b)
    {
        return dVector3D(a.x + b.x, a.y + b.y, a.z + b.z);
    }


    inline dVector3D operator- (const dVector3D &a, const dVector3D &b)
    {
        return dVector3D(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    inline dVector3D operator/ (const dVector3D &v, const double factor)
    {
        return dVector3D(v.x / factor, v.y / factor, v.z / factor);
    }


    inline dVector3D operator* (const dVector3D &a, const dVector3D &b)
    {
        return dVector3D(a.x * b.x, a.y * b.y, a.z * b.z);
    }

    inline double dVector3D::length() const
    {
        return sqrt(x*x + y*y + z*z);
    }

    inline double dVector3D::length2() const
    {
        return x*x + y*y + z*z;
    }


    inline void dVector3D::set(const double nx, const double ny, const double nz)
    {
        x = nx; y = ny; z = nz;
    }


    inline void dVector3D::set(const double *v)
    {
        x = v[0]; y = v[1]; z = v[2];
    }


    inline void dVector3D::PackTo01()
    {
        for (KCL::uint32 i = 0; i<3; i++)
        {
            v[i] = (v[i] + 1.0f) * 0.5f;
        }
    }


    inline dVector3D &dVector3D::normalize(dVector3D &v)
    {
        return v.normalize();
    }


    inline double dVector3D::length(const dVector3D &v)
    {
        return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    }

    inline double dVector3D::length2(const dVector3D &v)
    {
        return v.x*v.x + v.y*v.y + v.z*v.z;
    }

    inline dVector3D dVector3D::cross(const dVector3D &a, const dVector3D &b)
    {
        return dVector3D(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
    }


    inline dVector3D& dVector3D::normalize()
    {
        double L = length();
        if (L > Math::kEpsilon)
        {
            double d = 1.0f / L;
            x *= d; y *= d; z *= d;
        }
        return *this;
    }


    inline double dVector3D::distance(const dVector3D &a, const dVector3D &b)
    {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        double dz = a.z - b.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }

    inline double dVector3D::distance2(const dVector3D &a, const dVector3D &b)
    {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        double dz = a.z - b.z;
        return dx*dx + dy*dy + dz*dz;
    }

    inline dVector4D &dVector4D::operator/= (const double &f)
    {
        x /= f; y /= f; z /= f; w /= f;
        return *this;
    }
}

#endif
