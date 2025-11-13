/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file math3d.cpp
	Math functions supporting 3D rendering and animation calculations.
*/

#include <kcl_math3d.h>
#include "jsonserializer.h"
using namespace KCL;

const Matrix4x4 KCL::kIdentityMatrix4x4;


void Serialize(KCL::Vector2D& vec, JsonSerializer& s)
{
	s.Serialize(0, vec.x);
	s.Serialize(1, vec.y);
}


void Serialize(KCL::Vector3D& vec, JsonSerializer& s)
{
	s.Serialize(0, vec.x);
	s.Serialize(1, vec.y);
	s.Serialize(2, vec.z);
}


void Serialize(KCL::Vector4D& vec, JsonSerializer& s)
{
	s.Serialize(0, vec.x);
	s.Serialize(1, vec.y);
	s.Serialize(2, vec.z);
	s.Serialize(3, vec.w);
}


void Serialize(KCL::Matrix4x4& m, JsonSerializer& s)
{
	for (int i = 0; i < 16; ++i)
	{
		s.Serialize(i, m.v[i]);
	}
}


Matrix4x4 &Matrix4x4::operator+= (const Matrix4x4 &M)
{
	v11 += M.v11; v12 += M.v12;	v13 += M.v13; v14 += M.v14;
	v21 += M.v21; v22 += M.v22; v23 += M.v23; v24 += M.v24;
	v31 += M.v31; v32 += M.v32; v33 += M.v33; v34 += M.v34;
	v41 += M.v41; v42 += M.v42; v43 += M.v43; v44 += M.v44;
	return *this;
}

Matrix4x4 KCL::operator+ (const Matrix4x4 &L, const Matrix4x4 &R)
{
	Matrix4x4 m;
	m.v11 = L.v11 + R.v11; m.v12 = L.v12 + R.v12; m.v13 = L.v13 + R.v13; m.v14 = L.v14 + R.v14;
	m.v21 = L.v21 + R.v21; m.v22 = L.v22 + R.v22; m.v23 = L.v23 + R.v23; m.v24 = L.v24 + R.v24;
	m.v31 = L.v31 + R.v31; m.v32 = L.v32 + R.v32; m.v33 = L.v33 + R.v33; m.v34 = L.v34 + R.v34;
	m.v41 = L.v41 + R.v41; m.v42 = L.v42 + R.v42; m.v43 = L.v43 + R.v43; m.v44 = L.v44 + R.v44;
	return m;
}

bool Matrix4x4::operator== (const Matrix4x4 &M) const
{
    for (unsigned int i = 0; i < 16; i++)
    {
        if (v[i] != M.v[i])
        {
            return false;
        }
    }
    return true;
}

bool Matrix4x4::operator!= (const Matrix4x4 &M) const
{
    return !((*this) == M);
}

Matrix4x4 &Matrix4x4::operator*= (const Matrix4x4 &M)
{
	float nv11 = v11*M.v11 + v12*M.v21 + v13*M.v31 + v14*M.v41;
	float nv12 = v11*M.v12 + v12*M.v22 + v13*M.v32 + v14*M.v42;
	float nv13 = v11*M.v13 + v12*M.v23 + v13*M.v33 + v14*M.v43;
	float nv14 = v11*M.v14 + v12*M.v24 + v13*M.v34 + v14*M.v44;

	float nv21 = v21*M.v11 + v22*M.v21 + v23*M.v31 + v24*M.v41;
	float nv22 = v21*M.v12 + v22*M.v22 + v23*M.v32 + v24*M.v42;
	float nv23 = v21*M.v13 + v22*M.v23 + v23*M.v33 + v24*M.v43;
	float nv24 = v21*M.v14 + v22*M.v24 + v23*M.v34 + v24*M.v44;

	float nv31 = v31*M.v11 + v32*M.v21 + v33*M.v31 + v34*M.v41;
	float nv32 = v31*M.v12 + v32*M.v22 + v33*M.v32 + v34*M.v42;
	float nv33 = v31*M.v13 + v32*M.v23 + v33*M.v33 + v34*M.v43;
	float nv34 = v31*M.v14 + v32*M.v24 + v33*M.v34 + v34*M.v44;

	float nv41 = v41*M.v11 + v42*M.v21 + v43*M.v31 + v44*M.v41;
	float nv42 = v41*M.v12 + v42*M.v22 + v43*M.v32 + v44*M.v42;
	float nv43 = v41*M.v13 + v42*M.v23 + v43*M.v33 + v44*M.v43;
	float nv44 = v41*M.v14 + v42*M.v24 + v43*M.v34 + v44*M.v44;

	v11 = nv11; v12 = nv12; v13 = nv13; v14 = nv14;
	v21 = nv21; v22 = nv22; v23 = nv23; v24 = nv24;
	v31 = nv31; v32 = nv32; v33 = nv33; v34 = nv34;
	v41 = nv41; v42 = nv42; v43 = nv43; v44 = nv44;

	return *this;
}




Vector3D KCL::mult4x3 (const Matrix4x4 &A, const Vector3D &v)
{
	return Vector3D (
		A.v11*v.x + A.v21*v.y + A.v31*v.z + A.v41,
		A.v12*v.x + A.v22*v.y + A.v32*v.z + A.v42,
		A.v13*v.x + A.v23*v.y + A.v33*v.z + A.v43
		);
}


Matrix4x4::operator const float *() const
{
	return (float *)&v11;
}


float Matrix4x4::determinant3x3 () const
{
	return Determinant3x3 (*this);
}


void Matrix4x4::getNormalMatrix (Matrix4x4 &normal) const
{
	normal = *this;
	normal.v41 = 0.0f;
	normal.v42 = 0.0f;
	normal.v43 = 0.0f;
	normal = Invert4x3 (normal);
	normal.transpose ();
}

void Matrix4x4::transpose ()
{
	Math::swap (v12, v21);
	Math::swap (v13, v31);
	Math::swap (v14, v41);
	Math::swap (v24, v42);
	Math::swap (v34, v43);
	Math::swap (v23, v32);
}


void Matrix4x4::zero ()
{
	Zero (*this);
}


void Matrix4x4::rotate (const float angle, const Vector3D &axis, const bool normalize)
{
	if (fabs (angle) < Math::kEpsilon)
	{
		return;
	}
	float rad = Math::Rad (angle);
	float co = cos (rad);
	float si = sin (rad);
	Vector3D n (axis);
	if (normalize) {
		n.normalize ();
	}
	float nx = n.x;
	float ny = n.y;
	float nz = n.z;

	float m11 = nx*nx*(1.0f - co) + co;
	float m12 = (1.0f - co)*nx*ny + si*nz;
	float m13 = (1.0f - co)*nx*nz - si*ny;

	float m21 = (1.0f - co)*nx*ny - si*nz;
	float m22 = ny*ny*(1.0f - co) + co;
	float m23 = (1.0f - co)*ny*nz + si*nx;

	float m31 = (1.0f - co)*nx*nz + si*ny;
	float m32 = (1.0f - co)*ny*nz - si*nx;
	float m33 = nz*nz*(1.0f - co) + co;

	Matrix4x4 m(*this);

	v11 = m11*m.v11 + m12*m.v21 + m13*m.v31;
	v12 = m11*m.v12 + m12*m.v22 + m13*m.v32;
	v13 = m11*m.v13 + m12*m.v23 + m13*m.v33;

	v21 = m21*m.v11 + m22*m.v21 + m23*m.v31;
	v22 = m21*m.v12 + m22*m.v22 + m23*m.v32;
	v23 = m21*m.v13 + m22*m.v23 + m23*m.v33;

	v31 = m31*m.v11 + m32*m.v21 + m33*m.v31;
	v32 = m31*m.v12 + m32*m.v22 + m33*m.v32;
	v33 = m31*m.v13 + m32*m.v23 + m33*m.v33;
}

void Matrix4x4::rotateTo (const float angle, const Vector3D &axis, const bool normalize, int axisLimit)
{
	if (fabs (angle) < Math::kEpsilon)
	{
		return;
	}
	float rad = Math::Rad (angle);
	float co = cos (rad);
	float si = sin (rad);
	Vector3D n (axis);
	if (normalize) {
		n.normalize ();
	}
	float nx = n.x;
	float ny = n.y;
	float nz = n.z;

	float m11 = nx*nx*(1.0f - co) + co;
	float m12 = (1.0f - co)*nx*ny + si*nz;
	float m13 = (1.0f - co)*nx*nz - si*ny;

	float m21 = (1.0f - co)*nx*ny - si*nz;
	float m22 = ny*ny*(1.0f - co) + co;
	float m23 = (1.0f - co)*ny*nz + si*nx;

	float m31 = (1.0f - co)*nx*nz + si*ny;
	float m32 = (1.0f - co)*ny*nz - si*nx;
	float m33 = nz*nz*(1.0f - co) + co;

	Matrix4x4 m(*this);

	if( (axisLimit == 0) || (axisLimit != 1) )
	{
		v11 = m11*m.v11 + m12*m.v21 + m13*m.v31;
		v12 = m11*m.v12 + m12*m.v22 + m13*m.v32;
		v13 = m11*m.v13 + m12*m.v23 + m13*m.v33;
	}

	if( (axisLimit == 0) || (axisLimit != 2) )
	{
		v21 = m21*m.v11 + m22*m.v21 + m23*m.v31;
		v22 = m21*m.v12 + m22*m.v22 + m23*m.v32;
		v23 = m21*m.v13 + m22*m.v23 + m23*m.v33;
	}

	if( (axisLimit == 0) || (axisLimit != 3) )
	{
		v31 = m31*m.v11 + m32*m.v21 + m33*m.v31;
		v32 = m31*m.v12 + m32*m.v22 + m33*m.v32;
		v33 = m31*m.v13 + m32*m.v23 + m33*m.v33;
	}
}


void Matrix4x4::scale (const Vector3D &s)
{
	v11 *= s.x; v12 *= s.x; v13 *= s.x; v14 *= s.x;
	v21 *= s.y; v22 *= s.y; v23 *= s.y; v24 *= s.y;
	v31 *= s.z; v32 *= s.z; v33 *= s.z; v34 *= s.z;
}


void Matrix4x4::lookat (const Vector3D &eye, const Vector3D &center, const Vector3D &up)
{
	Matrix4x4 L;
	LookAt (L, eye, center, up);
	*this = L * *this;
}

void Matrix4x4::Scale_translate_to_fit (Matrix4x4& output, const Vector3D& vMin, const Vector3D& vMax)
{
	float *o = output.v;
	
	o[ 0] = 2/(vMax[0]-vMin[0]);
	o[ 4] = 0;
	o[ 8] = 0;
	o[12] = -(vMax[0]+vMin[0])/(vMax[0]-vMin[0]);

	o[ 1] = 0;
	o[ 5] = 2/(vMax[1]-vMin[1]);
	o[ 9] = 0;
	o[13] = -(vMax[1]+vMin[1])/(vMax[1]-vMin[1]);

	o[ 2] = 0;
	o[ 6] = 0;

	if (zRangeZeroToOneGlobalOpt)
	{
		o[10] = 1/(vMax[2]-vMin[2]);
		o[14] = -(vMax[2]+vMin[2])/( 2*( vMax[2]-vMin[2]))+0.5;
	}
	else {
		o[10] = 2 / (vMax[2] - vMin[2]);
		o[14] = -(vMax[2] + vMin[2]) / (vMax[2] - vMin[2]);
	}

	o[ 3] = 0;
	o[ 7] = 0;
	o[11] = 0;
	o[15] = 1;

}


void Matrix4x4::perspective (const float fovy, const float aspect, const float zNear, const float zFar)
{
	Matrix4x4 P;
	Perspective (P, fovy, aspect, zNear, zFar);
	*this = P * *this;
}


void Matrix4x4::LookAt (Matrix4x4 &M, const Vector3D &eye, const Vector3D &center, const Vector3D &up)
{
	Identity (M);

	Vector3D forward (center - eye);
	Vector3D u (up);
	Vector3D side;

	forward.normalize ();
	u.normalize ();

	side = Vector3D::cross (forward, up);
	side.normalize ();
	u = Vector3D::cross (side, forward);
	u.normalize ();

	M.v11 = side.x; M.v21 = side.y; M.v31 = side.z; M.v41 = 0.0f;
	M.v12 = u.x; M.v22 = u.y; M.v32 = u.z; M.v42 = 0.0f;
	M.v13 = -forward.x; M.v23 = -forward.y; M.v33 = -forward.z; M.v43 = 0.0f;

	M.translate (-eye);
}


void Matrix4x4::Perspective(Matrix4x4 &M, float fovy, float aspect, float zNear, float zFar)
{
	Zero(M);
	float slopey = (float)tan(Math::Rad(fovy / 2.0f));
	M.v11 = 1.0f / slopey / aspect;
	M.v22 = 1.0f / slopey;
	M.v34 = -1.0f;
	if (zRangeZeroToOneGlobalOpt)
	{
		//see DirectXMath.h+DirectXMathMatrix.inl / XMMatrixPerspectiveFovRH
		M.v33 = -(zFar) / (zFar - zNear);
		M.v43 = -zNear * zFar / (zFar - zNear);
	} else {
		M.v33 = -(zNear + zFar) / (zFar - zNear);
		M.v43 = -2.0f * zNear * zFar / (zFar - zNear);
	}
    
}


void Matrix4x4::PerspectiveGL(Matrix4x4 &M, float fovy, float aspect, float zNear, float zFar)
{
	Zero(M);
	float slopey = (float)tan(Math::Rad(fovy / 2.0f));
	M.v11 = 1.0f / slopey / aspect;
	M.v22 = 1.0f / slopey;
	M.v34 = -1.0f;

	M.v33 = -(zNear + zFar) / (zFar - zNear);
	M.v43 = -2.0f * zNear * zFar / (zFar - zNear);
}


void Matrix4x4::Frustum (Matrix4x4 &M, float left, float right, float bottom, float top, float zNear, float zFar)
{
	Zero (M);
	M.v11 = 2.0f*zNear / (right - left);
	M.v22 = 2.0f*zNear / (top - bottom);

    M.v31 = (right + left) / (right - left);
	M.v32 = (top + bottom) / (top - bottom);

    M.v34 = -1.0;
	if (zRangeZeroToOneGlobalOpt)
	{
		//see DirectXMath.h+DirectXMathMatrix.inl / XMMatrixPerspectiveOffCenterRH
		M.v33 = zFar / (zNear - zFar);	
		M.v43 = zNear * zFar / (zNear - zFar);;
	} else {
		M.v33 = -(zFar + zNear) / (zFar - zNear);
		M.v43 = -2.0f*zFar*zNear / (zFar - zNear);
	}
}


void Matrix4x4::Ortho (Matrix4x4 &M, float left, float right, float bottom, float top, float zNear, float zFar)
{
	Zero (M);
	M.v11 = 2.0f / (right - left);
	M.v22 = 2.0f / (top - bottom);

    M.v41 = - (left + right) / (right - left);
    M.v42 = - (top + bottom) / (top - bottom);
	if (zRangeZeroToOneGlobalOpt)
	{
		//see DirectXMath.h+DirectXMathMatrix.inl / XMMatrixOrthographicOffCenterRH
		M.v33 = 1.0f / (zNear - zFar);
		M.v43 = zNear / (zNear - zFar);
	} else {
		M.v33 = -2.0f / (zFar - zNear);
		M.v43 = -(zFar + zNear) / (zFar - zNear);
	}
	M.v44 = 1.0f;
}


void Matrix4x4::RotationMatrixFromDirection(Matrix4x4 &M, KCL::Vector3D direction, const KCL::Vector3D& up)
{
    direction.normalize();
    direction *= -1.0f;
	KCL::Vector3D xaxis = KCL::Vector3D::cross(up, direction);
	xaxis.normalize();

	KCL::Vector3D yaxis = KCL::Vector3D::cross(direction, xaxis);
	yaxis.normalize();

	KCL::Matrix4x4 result;

	result.v11 = xaxis.x;
	result.v21 = yaxis.x;
	result.v31 = direction.x;
	result.v41 = 0.0f;

	result.v12 = xaxis.y;
	result.v22 = yaxis.y;
	result.v32 = direction.y;
	result.v42 = 0.0f;

	result.v13 = xaxis.z;
	result.v23 = yaxis.z;
	result.v33 = direction.z;
	result.v43 = 0.0f;

	result.v14 = 0.0f;
	result.v24 = 0.0f;
	result.v34 = 0.0f;
	result.v44 = 1.0f;

	M = result;
};


float Matrix4x4::Determinant3x3 (const Matrix4x4 &M)
{
	return M.v11 * (M.v22*M.v33 - M.v23*M.v32)
		 + M.v12 * (M.v23*M.v31 - M.v21*M.v33)
		 + M.v13 * (M.v21*M.v32 - M.v22*M.v31);
}


Matrix4x4 Matrix4x4::Invert4x3 (const Matrix4x4 &m)
{
	float det = Determinant3x3 (m);

	const float k1OverDet = 1.0f / det;

	Matrix4x4	r;

	r.v11 = (m.v22*m.v33 - m.v23*m.v32) * k1OverDet;
	r.v12 = (m.v13*m.v32 - m.v12*m.v33) * k1OverDet;
	r.v13 = (m.v12*m.v23 - m.v13*m.v22) * k1OverDet;

	r.v21 = (m.v23*m.v31 - m.v21*m.v33) * k1OverDet;
	r.v22 = (m.v11*m.v33 - m.v13*m.v31) * k1OverDet;
	r.v23 = (m.v13*m.v21 - m.v11*m.v23) * k1OverDet;

	r.v31 = (m.v21*m.v32 - m.v22*m.v31) * k1OverDet;
	r.v32 = (m.v12*m.v31 - m.v11*m.v32) * k1OverDet;
	r.v33 = (m.v11*m.v22 - m.v12*m.v21) * k1OverDet;

	r.v41 = -(m.v41*r.v11 + m.v42*r.v21 + m.v43*r.v31);
	r.v42 = -(m.v41*r.v12 + m.v42*r.v22 + m.v43*r.v32);
	r.v43 = -(m.v41*r.v13 + m.v42*r.v23 + m.v43*r.v33);
	r.v44 = 1.0f;

	return r;
}


void Matrix4x4::InvertModelView (const Matrix4x4 &modelView , Matrix4x4 &outM)
{
	outM.v11 = modelView.v11;
	outM.v21 = modelView.v12;
	outM.v31 = modelView.v13;
	outM.v41 = 0;
	
	outM.v12 = modelView.v21;
	outM.v22 = modelView.v22;
	outM.v32 = modelView.v23;
	outM.v42 = 0;
	
	outM.v13 = modelView.v31;
	outM.v23 = modelView.v32;
	outM.v33 = modelView.v33;
	outM.v43 = 0;
	
	outM.v14 = -modelView.v14;
	outM.v24 = -modelView.v24;
	outM.v34 = -modelView.v34;
	outM.v44 = 1;
}

void Matrix4x4::perspective (float fov, float aspect, float zNear)
{
	double slopey = (float)tan(fov * 3.14159268f / 360.0f);
	double m00 = 1 / slopey / aspect;
	double m11 = 1 / slopey;
	double m22 = -1;
	double m32 = -2.0 * zNear;

	identity();

	v11 = m00;
	v22 = m11;
	v33 = m22;
	v34 = -1.0f;
	v43 = m32;
	v44 = 0.0f;
}



void Matrix4x4::reflection( const Vector4D &plane)
{
	float _2ab = -2.0f * plane.x * plane.y;
	float _2ac = -2.0f * plane.x * plane.z;
	float _2bc = -2.0f * plane.y * plane.z;

	v[0] = 1.0f - 2.0f * plane.x * plane.x;
	v[1] = _2ab;
	v[2] = _2ac;
	v[3] = 0.0f;

	v[4] = _2ab;
	v[5] = 1.0f - 2.0f * plane.y * plane.y;
	v[6] = _2bc;
	v[7] = 0.0f;

	v[8] = _2ac;
	v[9] = _2bc;
	v[10] = 1.0f - 2.0f * plane.z * plane.z;
	v[11] = 0.0f;

	v[12] =    - 2 * plane.x * plane.w; 
	v[13] =    - 2 * plane.y * plane.w; 
	v[14] =    - 2 * plane.z * plane.w; 
	v[15] = 1.0f;
}

 
bool Matrix4x4::IsNan() const
{
	for(size_t idx = 0; idx<16; ++idx)
		if (::IsNan(v[idx]))
			return true;

	return false;
}


void Matrix4x4::adjustNearPlaneForPerspective( const Vector4D &clipPlane)
{
	Vector4D    q;

	// Calculate the clip-space corner point opposite the clipping plane
	// as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
	// transform it into camera space by multiplying it
	// by the inverse of the projection matrix

	q.x = (sgn(clipPlane.x) + v[8]) / v[0];
	q.y = (sgn(clipPlane.y) + v[9]) / v[5];
	q.z = -1.0f;
	q.w = (1.0f + v[10]) / v[14];

	// Calculate the scaled plane vector
	Vector4D c;
	
	if (zRangeZeroToOneGlobalOpt)
	{
		c = clipPlane * (1 / Vector4D::dot(clipPlane, q));
	}
	else
	{
		c = clipPlane * (2 / Vector4D::dot(clipPlane, q));
	}

	// Replace the third row of the projection v
	v[2] = c.x;
	v[6] = c.y;
	if (zRangeZeroToOneGlobalOpt)
	{
		v[10] = c.z;
	}
	else
	{
		v[10] = c.z + 1;
	}
	v[14] = c.w;
}


const Vector3D KCL::kZeroVector3D;


Vector3D::Vector3D (const float v[]) : x(v[0]), y(v[1]), z(v[2])
{
}


Vector3D::Vector3D (const Vector4D &v)
{
	x = v.x/v.w; y = v.y/v.w; z = v.z/v.w;
}


Vector3D::operator const float *() const
{
	return (float *) &x;
}



Vector3D KCL::operator* (const Vector3D &v, const float factor)
{
	return Vector3D (v.x * factor, v.y * factor, v.z * factor);
}


Vector3D KCL::operator* (const float factor, const Vector3D &v)
{
	return Vector3D (v.x * factor, v.y * factor, v.z * factor);
}


float Vector3D::dot (const Vector3D &a, const Vector3D &b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}


Vector3D Vector3D::interpolate (const Vector3D &a, const Vector3D &b, const float t)
{
	return Vector3D (t*b + (1.0f-t)*a);
}





const Vector4D KCL::kZeroVector4D;


Vector4D::Vector4D() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}


Vector4D::Vector4D (const Vector3D &v)
{
	x = v.x; y = v.y; z = v.z; w = 1;
}

Vector4D::Vector4D (const Vector3D &v, float nw) : x(v.x), y(v.y), z(v.z), w(nw)
{
}

Vector4D::Vector4D (const float nx, const float ny, const float nz, const float nw) : x(nx), y(ny), z(nz), w(nw)
{
}


Vector4D::Vector4D (const float v[]) : x(v[0]), y(v[1]), z(v[2]), w(v[3])
{
}


bool Vector4D::operator== (const Vector4D &v) const
{
	 return x==v.x && y==v.y && z==v.z && w==v.w;
}


bool Vector4D::operator!= (const Vector4D &v) const
{
	 return x!=v.x || y!=v.y || z!=v.z || w!=v.w;
}

Vector4D::operator const float *() const
{
	return (float *) &x;
}


Vector4D KCL::operator- (const Vector4D &v)
{
	return Vector4D (-v.x, -v.y, -v.z, -v.w);
}


Vector4D KCL::operator+ (const Vector4D &a, const Vector4D &b)
{
	return Vector4D (a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}


Vector4D KCL::operator- (const Vector4D &a, const Vector4D &b)
{
	return Vector4D (a.x/a.w - b.x/b.w, a.y/a.w - b.y/b.w, a.z/a.w - b.z/b.w, 1);
}


Vector4D KCL::operator* (const Vector4D &v, const float factor)
{
	return Vector4D (v.x * factor, v.y * factor, v.z * factor, v.w * factor);
}


Vector4D KCL::operator* (const float factor, const Vector4D &v)
{
	return Vector4D (v.x * factor, v.y * factor, v.z * factor, v.w * factor);
}


Vector4D KCL::operator* (const Matrix4x4 &M, const Vector4D &v)
{
	return Vector4D (
		M.v11*v.x + M.v21*v.y + M.v31*v.z + M.v41*v.w,
		M.v12*v.x + M.v22*v.y + M.v32*v.z + M.v42*v.w,
		M.v13*v.x + M.v23*v.y + M.v33*v.z + M.v43*v.w,
		M.v14*v.x + M.v24*v.y + M.v34*v.z + M.v44*v.w);
}


Vector4D KCL::operator/ (const Vector4D &v, const float factor)
{
	return Vector4D (v.x / factor, v.y / factor, v.z / factor, v.w / factor);
}


Vector4D& Vector4D::homogenize ()
{
	x /= w; y /= w; z /= w; w = 1.0f;
	return *this;
}


void Vector4D::set (const float nx, const float ny, const float nz, const float nw)
{
	x = nx; y = ny; z = nz; w = nw;
}


void Vector4D::set(const Vector3D &v, float w_)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = w_;
}


void Vector4D::set(const float *v)
{
	x = v[0]; y = v[1]; z = v[2]; w = v[3];
}


float Vector4D::length3 ()
{
	return sqrt( x * x + y * y + z * z);
}


Vector4D Vector4D::interpolate (const Vector4D &a, const Vector4D &b, const float t)
{
	return Vector4D (t*b + (1.0f-t)*a);
}


float Vector4D::dot (const Vector4D &a, const Vector4D &b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}




Quaternion::Quaternion ()
{
	w = 1.0; x = y = z = 0.0f;
}

Quaternion::Quaternion (const float nw, const float nx, const float ny, const float nz)
{
	x = nx; y = ny; z = nz; w = nw;
}


Quaternion::Quaternion (const float *v)
{
	x = v[0]; y = v[1]; z = v[2]; w = v[3];
}

Quaternion &Quaternion::operator *=(const float &f)
{
	w *= f; x *= f; y *= f; z *= f;
	return *this;
}


Quaternion KCL::operator+ (const Quaternion &a, const Quaternion &b)
{
	return Quaternion (a.w+b.w, a.x+b.x, a.y+b.y, a.z+b.z);
}


Quaternion KCL::operator- (const Quaternion &a)
{
	return Quaternion (-a.w, -a.x, -a.y, -a.z);
}


Quaternion KCL::operator- (const Quaternion &a, const Quaternion &b)
{
	return Quaternion (a.w-b.w, a.x-b.x, a.y-b.y, a.z-b.z);
}

Quaternion KCL::operator* (const Quaternion &v, const float factor)
{
	return Quaternion (v.w * factor, v.x * factor, v.y * factor, v.z * factor);
}


Quaternion KCL::operator* (const float factor, const Quaternion &v)
{
	return Quaternion (v.w * factor, v.x * factor, v.y * factor, v.z * factor);
}


void Quaternion::set (float nw, float nx, float ny, float nz)
{
	w = nw; x = nx; y = ny; z = nz;
}


void Quaternion::fromAngleAxis (const float angle, const Vector3D &axis)
{
	float halfangle = Math::Rad (angle) * 0.5f;
	float si = sin (halfangle);
	w = cos (halfangle);
	x = axis.x * si;
	y = axis.y * si;
	z = axis.z * si;
}


void Quaternion::getRotationMatrix(Matrix4x4 &m)
{
	const float X = this->x;
	const float Y = this->y;
	const float Z = this->z;
	const float W = this->w;

	const float xx = X * X;
	const float xy = X * Y;
	const float xz = X * Z;
	const float xw = X * W;
	
	const float yy = Y * Y;
	const float yz = Y * Z;
	const float yw = Y * W;
	
	const float zz = Z * Z;
	const float zw = Z * W;
	
	m.v11 = 1 - 2 * (yy + zz);
	m.v12 = 2 * (xy - zw);
	m.v13 = 2 * (xz + yw);

	m.v21 = 2 * (xy + zw);
	m.v22 = 1 - 2 * (xx + zz);
	m.v23 = 2 * (yz - xw);

	m.v31 = 2 * (xz - yw);
	m.v32 = 2 * (yz + xw);
	m.v33 = 1 - 2 * (xx + yy);
}


void Quaternion::toAngleAxis (float &angle, Vector3D &axis) const
{
	float scale = sqrt (x*x + y*y + z*z);
	if (fabs (scale) < Math::kEpsilon || w > 1.0f) {
		angle = 0.0f;
		axis.set (0, 1, 0);
	} else {
		angle = Math::Deg (2.0f * acos (w));
		axis.set (x/scale, y/scale, z/scale);
	}
}

void Quaternion::Interpolate (Quaternion q1, Quaternion q2, float t, Quaternion &result)
{
	float dot, s1, s2, om, sinom;
	dot = Dot (q1, q2);

	if (dot < 0) {
		q1 = -q1;
		dot  = - dot;
	}
	if ((1.0f - dot) > 0.0015) { // spherical interpolation
		om    = acos (dot);
		sinom = sin (om);
		s1    = sin ((1.0f - t)*om) / sinom;
		s2    = sin (t*om)/ sinom;
	} else { // linear interpolation
		s1 = 1.0f - t;
		s2 = t;
	}
	result = s1*q1 + s2*q2;
}


static const KCL::uint16 randTab[] =
{
	0x6897, 0x1507, 0x7AA3, 0x58F0, 0x36BF, 0xC277, 0xAFFD, 0xEFDD, 0x4A48, 0xCC6F, 0x21B2, 0xA819,
	0x5046, 0x1006, 0xE810, 0xB7B0, 0xA86F, 0x488D, 0xA814, 0x76F, 0xD5F2, 0x8FA4, 0x56B2, 0x3CB8,
	0xB96, 0x2D21, 0xAF70, 0xB122, 0x6D3A, 0xE84, 0xDB75, 0xEDE5, 0xE693, 0x3B35, 0x69D6, 0x4499,
	0x5911, 0x497F, 0xFE73, 0x452B, 0xE533, 0x7807, 0x3060, 0x495C, 0x3CF9, 0x2450, 0x75A0, 0x2917,
	0x9D91, 0x72DE, 0x9046, 0x2CEB, 0xB93D, 0xE4EF, 0xFA5, 0x64FA, 0xB545, 0x835A, 0xFDFB, 0xF980,
	0x1EF1, 0x4DEB, 0x1728, 0xAE81, 0x15B6, 0x4045, 0x775, 0x8165, 0x6AEE, 0xF0D9, 0xC84D, 0x36A4,
	0x7BE5, 0x87FD, 0xDC8F, 0xB75D, 0x572D, 0x66C7, 0xF19F, 0x9BD2, 0x3326, 0x18BF, 0xA5CB, 0x108C,
	0x5228, 0xAC9D, 0x2390, 0x5B8D, 0xACC5, 0x3EB7, 0xA2C, 0x6920, 0xDAB5, 0x6434, 0x8E0, 0x7D74,
	0xA549, 0x765F, 0x5202, 0x1328, 0x6BC4, 0x467C, 0x24C1, 0xB61D, 0xB0DA, 0xAC23, 0x8588, 0x203B,
	0x6CD8, 0x9DE6, 0x3FF5, 0x5539, 0x1ECA, 0xD342, 0x7963, 0x7590, 0xD437, 0x6A09, 0x551B, 0x3FB8,
	0xEA87, 0x97E8, 0xC79F, 0x5D12, 0xA00A, 0x8527, 0xFE5A, 0x3411, 0xD23C, 0x6D05, 0x923, 0x72E8,
	0xC19D, 0xFAC8, 0xB171, 0x6A1B, 0xA9F1, 0x4049, 0x233F, 0x745C, 0xD663, 0x1DA9, 0x68BF, 0xA933,
	0x4995, 0x22D0, 0xB0B5, 0x90BC, 0xDCE4, 0x3FA1, 0x9A5B, 0xAF51, 0x11A5, 0xCDC4, 0x16A1, 0xAA6D,
	0xDFD4, 0x8FB5, 0xF06B, 0xCEFC, 0x697A, 0xA079, 0xE240, 0xE726, 0x3937, 0x52D4, 0x199E, 0xB7AD,
	0x2BA1, 0xC2EB, 0xE9BA, 0xBC79, 0x539C, 0x7E2C, 0x98FD, 0x55CF, 0x25E3, 0x8AA, 0xFD37, 0xD588,
	0x379, 0x9D78, 0xB7CF, 0x46B1, 0x82C9, 0x2161, 0xEF90, 0x5DBD, 0x8582, 0x61EF, 0xA868, 0x3D22,
	0xAFCA, 0x772D, 0xCDB2, 0x515D, 0xB425, 0x5F57, 0x9584, 0xFBAC, 0xBCCE, 0x85A, 0x9A1A, 0x9EAB,
	0x6A21, 0x79A0, 0x2F43, 0x7B1A, 0x218, 0xBCAE, 0x5F1B, 0xC820, 0x6FEF, 0x9DF2, 0x3158, 0xAEED,
	0xD972, 0xA787, 0x8410, 0x1577, 0xF3C7, 0x3C77, 0x6D23, 0x31AB, 0x1CEF, 0x1E27, 0xAFD, 0x67D7,
	0xECF8, 0x2BDF, 0x82F2, 0x27AC, 0x9DFE, 0xD95, 0x9676, 0x974E, 0xD8A5, 0x6102, 0x4907, 0x95E0,
	0x493C, 0x9F70, 0x273C, 0x7670, 0xFD58, 0xFBBD, 0x7FC6, 0xF50D, 0x2FFA, 0x6823, 0x87E1, 0x4A5,
	0xEA1A, 0x7CA2, 0x8069, 0x208F, 0x40A9, 0x9B4C, 0xB5B, 0x9B31, 0x60A8, 0x96F2, 0xC7CB, 0x97F0,
	0xFCE8, 0x1444, 0x558, 0x6D81, 0xD9BC, 0xF28E, 0x3B04, 0x603B, 0x5F66, 0x2631, 0x9D33, 0xBA69,
	0xA516, 0xDD0B, 0x633A, 0xFD03, 0x6DAC, 0xDDE6, 0x6BDF, 0x1DE8, 0xD7AD, 0xA979, 0x6BAA, 0xFDFC,
	0x28EF, 0x456C, 0x79D7, 0xE6A8, 0x2FED, 0x98F0, 0x96AC, 0xBB15, 0x7FA7, 0xD4D6, 0x4A25, 0xED46,
	0x1561, 0xF500, 0x8D6A, 0xA89C, 0xF62B, 0x4907, 0xC7F4, 0x39C2, 0x378E, 0x63E4, 0x1CCB, 0xE93,
	0xBF85, 0x81FF, 0xD05, 0xA045, 0x84B8, 0xC2D9, 0x1C6C, 0x972E, 0xD3BC, 0x880A, 0x7E48, 0xC186,
	0x32F6, 0x3A65, 0x3FBE, 0xF4ED, 0x70D4, 0xA754, 0xDA43, 0xACAA, 0x4557, 0x4382, 0x31B9, 0x8E69,
	0xF667, 0x893A, 0xA27B, 0x6DB6, 0xD950, 0xA903, 0x8A7D, 0x2B3C, 0x90FC, 0x5837, 0x73DF, 0xB87,
	0x71F9, 0xF827, 0xFF25, 0x209D, 0xB4CD, 0x8A4F, 0xA049, 0xCCC9, 0xA799, 0xC35F, 0x6425, 0x4BC6,
	0xCAB3, 0x90BC, 0x8DEA, 0xFC7A, 0x97BF, 0xFD2D, 0xE925, 0xD273, 0x4F77, 0xB212, 0x7727, 0x7433,
	0xD4D4, 0x58DA, 0x5646, 0x2825, 0xE8B4, 0x5A7F, 0x90DF, 0x72E2, 0x67A2, 0xC17, 0xBC2, 0x3709,
	0x9CE, 0xFF57, 0x2D39, 0x69F0, 0x12FB, 0x32DD, 0x55E6, 0xBD04, 0x4519, 0x3F08, 0xF8C5, 0xA574,
	0x9F6E, 0x1C43, 0xB9A9, 0x4DA0, 0x5E9F, 0xE7, 0x505F, 0xA3B0, 0x1B91, 0xADE1, 0x7856, 0x11CE,
	0x6BB, 0x4B3B, 0xC114, 0xAB90, 0x21C6, 0x86D6, 0xFA03, 0xA290, 0xE771, 0x7C04, 0x5118, 0xBA31,
	0x96A, 0xAEE7, 0x75A5, 0x25F0, 0xA658, 0xA6D7, 0xCA35, 0x94E3, 0x3198, 0x4563, 0xBBE7, 0xEBBC,
	0x4C5, 0x89, 0x4350, 0x614C, 0xBFA9, 0x8984, 0x9047, 0x9366, 0x9F90, 0x8B71, 0xCB26, 0x1C70,
	0x5BD4, 0x70E7, 0xA03A, 0x8D76, 0x934D, 0xE83C, 0xB703, 0xE34E, 0xCA8E, 0x8663, 0xFD45, 0x8DC6,
	0x10C3, 0x196C, 0x1982, 0xD850, 0x4ED7, 0x4BDE, 0x69B6, 0xE019, 0xF6CD, 0x2255, 0xFBDA, 0xD84,
	0x2C86, 0x558C, 0xAC58, 0x61D6, 0xCF7A, 0x589D, 0x36FD, 0x4552, 0xA9A2, 0x56D1, 0xA96E, 0xEE09,
	0x17AA, 0x9DDA, 0xA5BC, 0x9C86, 0xACFB, 0x7A4B, 0x770, 0xEF54, 0xED4A, 0x1797, 0x58D3, 0x48B6,
	0x6EEE, 0xBCB9, 0x9237, 0x2F85, 0xD1, 0xD653, 0xFEE0, 0x3982, 0x348A, 0xA241, 0xFBF3, 0xEEF2,
	0x50C, 0xA096, 0xB991, 0x9261, 0x9F72, 0xC24E, 0xA13C, 0xD418, 0x1A9B, 0x81EB, 0x7CA6, 0xE10C,
	0x39F9, 0x67A8, 0x636B, 0x8B61, 0xD06B, 0x4BF1, 0x4DD, 0x95FC, 0x8DB4, 0xED7F, 0x3E2B, 0x804D,
	0xE8BA, 0x28F7, 0xBA6E, 0x9275, 0x4B8D, 0xA297, 0x498E, 0x4D6E, 0x58D0, 0xC4D3, 0x13A9, 0x9C0,
	0x8993, 0x7D13, 0x47D7, 0xBD23, 0x93B2, 0x3C14, 0xD08E, 0xB485, 0xF55C, 0xCCE1, 0xAB17, 0x5F32,
	0x396E, 0xB558, 0x8D57, 0xED0F, 0x6B03, 0xA1BC, 0xF5FC, 0xA26D, 0x6108, 0x15AB, 0x5509, 0xD350,
	0xEB79, 0xD466, 0x46D2, 0xB449, 0x784, 0x960C, 0x465B, 0xCEF2, 0xA650, 0xB13A, 0x5BE5, 0xD7E9,
	0x3D5C, 0x81A0, 0xBF8C, 0x142B, 0xDB1C, 0x4EDB, 0xB76A, 0x896C, 0x6326, 0x1DFF, 0x1EDF, 0x5709,
	0x884E, 0x1C50, 0x4D72, 0x93AF, 0x3F01, 0xB0F3, 0xE6BF, 0xBC0A, 0xCE51, 0xF508, 0xE2D0, 0x2966,
	0xB6C4, 0x5564, 0x1302, 0x64A1, 0xD2B0, 0xCDD2, 0xBCFA, 0x42A, 0x6AF0, 0x1192, 0xB6F6, 0x9D2F,
	0x31F3, 0x38E1, 0x6CEA, 0xE54D, 0xE16D, 0xB047, 0xD828, 0xFDED, 0x3129, 0xE91F, 0x485B, 0xFF6F,
	0xF445, 0xF8C4, 0x6D4C, 0x29EB, 0x9C5C, 0xCA6F, 0x3542, 0xB0CA, 0x793D, 0x14F9, 0x3F8C, 0x5B40,
	0xF7EA, 0x7B2D, 0x9F7, 0xE5, 0x1DE, 0xACEC, 0xB498, 0x190F, 0xDC15, 0x9F8F, 0xEAFF, 0xF35C,
	0x5512, 0xCBFC, 0xB7C9, 0x1127, 0x467B, 0xB61B, 0xCD44, 0xA2FF, 0x9772, 0xA4C1, 0x5691, 0xB30B,
	0xB10C, 0x72F4, 0x8C1E, 0xFCC5, 0x2FA9, 0xB730, 0x5C0B, 0x7E6B, 0x1689, 0xA6B9, 0x3101, 0x4630,
	0x4C69, 0x280F, 0x6EA2, 0xEABD, 0xB4BC, 0x3D2D, 0xD873, 0xA263, 0xA3FE, 0x46AD, 0x8C12, 0x3DBC,
	0x9316, 0xD15E, 0x2735, 0x4C30, 0xA170, 0xE108, 0xAAC2, 0xFA08, 0x4B9E, 0xCEF5, 0xA57, 0xA3E9,
	0x83E, 0x3B16, 0x587, 0x293D, 0x9D5F, 0xC5A4, 0xB6ED, 0xA966, 0x545C, 0x2DCB, 0x92D8, 0x3BF5,
	0xFDD4, 0x5392, 0x28CF, 0x69DB, 0xD2CD, 0xAC51, 0xFBE4, 0x8160, 0x707F, 0x5F25, 0x3A21, 0xC658,
	0xEEA3, 0xD950, 0x7128, 0x25CF, 0x6F2C, 0x6A27, 0x15D8, 0xCD12, 0xDE9F, 0xE2CF, 0x9BCC, 0xF337,
	0xFCB2, 0x9D70, 0xFA62, 0x26EB, 0x3803, 0xB6DF, 0x657A, 0xDE53, 0xCED, 0xAD18, 0x49AE, 0xD381,
	0xF51B, 0x8C8D, 0xA5D0, 0x5AA7, 0x18A9, 0x6D32, 0x27F9, 0x8A91, 0xDD63, 0x4034, 0x57B, 0xD9F1,
	0x3F35, 0x305, 0x1218, 0xF3AC, 0xA59A, 0xDA9A, 0xBA2A, 0x5ACC, 0xE4C7, 0x402A, 0x75C6, 0x150,
	0x55CF, 0xA9E1, 0x615D, 0x3AEB, 0x618B, 0x272F, 0x6CA6, 0xC671, 0xB616, 0x2C3B, 0x41F8, 0x9C69,
	0xED7, 0x6DCC, 0x2E45, 0x2BDA, 0x43E0, 0x75EA, 0xFC2B, 0x438D, 0x17F, 0x3E02, 0xE8AE, 0xF0AC,
	0xD6CE, 0x97B1, 0xCD1D, 0x5978, 0xD1BE, 0x4262, 0x704A, 0x603B, 0xDD41, 0x37E, 0x3398, 0x8D0E,
	0xA804, 0x9164, 0x62CA, 0x60E7, 0x49FC, 0xBA05, 0xF296, 0x1E11, 0x5816, 0x98D0, 0x7205, 0xCCC9,
	0x84EE, 0xCF7C, 0x9270, 0xFEFD, 0x29CD, 0x123F, 0xA7F4, 0x5B9A, 0xCACD, 0xED33, 0xFEF8, 0x5525,
	0x8410, 0x205C, 0xEA4F, 0xC860, 0x62F3, 0x64AF, 0xA0CF, 0x3222, 0x45A3, 0xCE7A, 0xB8B0, 0xCD03,
	0x966D, 0x1E61, 0x9A05, 0xE2A1, 0xB3C3, 0xDD1F, 0xB9CA, 0x6356, 0x1EA6, 0x2787, 0x95B0, 0x56C1,
	0x4E85, 0x8B68, 0xD085, 0x66E2, 0xEF0B, 0xB69D, 0x1B35, 0xA5C2, 0xA5FA, 0x5FFB, 0x85A7, 0xF5C1,
	0x242E, 0x1CBD, 0x1331, 0xA512, 0x5434, 0xC0EC, 0x7D3E, 0xE373, 0x5D5A, 0x1253, 0xF392, 0x434C,
	0x1DD4, 0x4526, 0xB707, 0x7777, 0xCB01, 0x2B, 0x457D, 0xB364, 0x13AD, 0xA65, 0x694E, 0xCD9B,
	0xEF99, 0xB5BE, 0x14F, 0x96D3, 0x7AA5, 0x1DFC, 0xA09E, 0x1A85, 0xFE4, 0xC48C, 0x78AB, 0x90F,
	0xD560, 0xB880, 0x2DB8, 0xEC4F, 0x19AA, 0x516B, 0xD104, 0xECF8, 0xD88C, 0x2D24, 0x1400, 0x25B4,
	0x50FD, 0x755E, 0x78D9, 0xE556, 0xFB47, 0xE878, 0x1F49, 0x307F, 0xD76E, 0x7964, 0x3410, 0x8297,
	0x4379, 0x7AC7, 0x5A07, 0x36CC, 0x6C45, 0x424F, 0x2902, 0x9ADF, 0xCCE9, 0x27F0, 0x1996, 0x40A9,
	0xA40E, 0x81D0, 0x9EA9, 0xF085, 0xB4C9, 0x3206, 0x6D23, 0x493F, 0xEA99, 0xBE42, 0x6C25, 0x280E,
	0x67A1, 0x7ADB, 0xBF16, 0x587F, 0x5F04, 0x8E7D, 0xB111, 0x573C, 0xFD18, 0xFE6D, 0x1226, 0x3426,
	0xB163, 0xE0C5, 0xEFF2, 0xF878, 0x5F00, 0x1C79, 0x48FC, 0xD5A0, 0x4C1E, 0x94F9, 0x502, 0xE145,
	0x71A8, 0x57F9, 0x386D, 0x35E, 0xEB59, 0x9510, 0x303A, 0xC1FF, 0x2025, 0x13D3, 0x9D10, 0x4067,
	0x7F26, 0x5D09, 0xCA77, 0xAEC5, 0x770A, 0x9754, 0xACB1, 0x1988, 0x3E2B, 0x2DD1, 0xF7E4, 0xDEC4,
	0xF0DB, 0xC335, 0x4147, 0x4D21
};


float Math::randomf (int *seed)
{
	(*seed)++;
	return randTab[(*seed)%1000]/65536.0f;
}


float Math::randomf_signed (int *seed)
{
	float f = randomf( seed);
	return (f - 0.5f) * 2.0f;
}


float Math::randomf_from_range (int *seed, float min, float max)
{
	return min + randomf( seed) * (max - min);
}


bool Matrix4x4::Invert4x4(const Matrix4x4 &matrix, Matrix4x4 &resultMatrix)
{
	const float *m  = matrix.v;
	
	float inv[16], det;
	int i;

	inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] =   m[4]*m[9] *m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12] = -m[4]*m[9] *m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] =  -m[0]*m[9] *m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] =  m[0]*m[9] *m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] =   m[1]*m[6] *m[15] - m[1]*m[7] *m[14] - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7]  - m[13]*m[3]*m[6];
	inv[6] =  -m[0]*m[6] *m[15] + m[0]*m[7] *m[14] + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7]  + m[12]*m[3]*m[6];
	inv[10] =  m[0]*m[5] *m[15] - m[0]*m[7] *m[13] - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7]  - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5] *m[14] + m[0]*m[6] *m[13] + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6]  + m[12]*m[2]*m[5];
	inv[3] =  -m[1]*m[6] *m[11] + m[1]*m[7] *m[10] + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9] *m[2]*m[7]  + m[9] *m[3]*m[6];
	inv[7] =   m[0]*m[6] *m[11] - m[0]*m[7] *m[10] - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8] *m[2]*m[7]  - m[8] *m[3]*m[6];
	inv[11] = -m[0]*m[5] *m[11] + m[0]*m[7] *m[9]  + m[4]*m[1]*m[11] - m[4]*m[3]*m[9]  - m[8] *m[1]*m[7]  + m[8] *m[3]*m[5];
	inv[15] =  m[0]*m[5] *m[10] - m[0]*m[6] *m[9]  - m[4]*m[1]*m[10] + m[4]*m[2]*m[9]  + m[8] *m[1]*m[6]  - m[8] *m[2]*m[5];

	det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
	if (det == 0.0f)
		return false;

	det = 1.0f / det;

	for (i = 0; i < 16; i++)
		resultMatrix.v[i] = inv[i] * det;

	return true;
}

int KCL::texture_levels(int width, int height)
{
	int levels = 1;

	if (width == 0 || height == 0)
		return 0;

	for (;;)
	{
		if (width == 1 && height == 1)
			break;
		width = (width > 1) ? (width / 2) : 1;
		height = (height > 1) ? (height / 2) : 1;
		++levels;
	}
	return levels;
}

void dMatrix4x4::transpose()
{
    Math::swap(v12, v21);
    Math::swap(v13, v31);
    Math::swap(v14, v41);
    Math::swap(v24, v42);
    Math::swap(v34, v43);
    Math::swap(v23, v32);
}

/******************************************************************************************************/
//
//							dVector2D
//
/******************************************************************************************************/

dVector2D KCL::operator- (const dVector2D &a, const dVector2D &b)
{
    return dVector2D(a.x - b.x, a.y - b.y);
}


dVector2D KCL::operator+ (const dVector2D &a, const dVector2D &b)
{
    return dVector2D(a.x + b.x, a.y + b.y);
}

dVector2D::operator const double *() const
{
    return (double *)&x;
}

dVector2D KCL::operator* (const dVector2D &v1, const dVector2D &v2)
{
    return dVector2D(v1.x * v2.x, v1.y * v2.y);
}

dVector2D KCL::operator* (const dVector2D &v, const double factor)
{
    return dVector2D(v.x * factor, v.y * factor);
}


dVector2D KCL::operator* (const double factor, const dVector2D &v)
{
    return dVector2D(v.x * factor, v.y * factor);
}


double dVector2D::dot(const dVector2D &a, const dVector2D &b)
{
    return a.x*b.x + a.y*b.y;
}


dVector2D dVector2D::interpolate(const dVector2D &a, const dVector2D &b, const double t)
{
    return dVector2D(t*b + (1.0f - t)*a);
}

/******************************************************************************************************/
//
//							dVector3D
//
/******************************************************************************************************/


dVector3D::dVector3D(const double v[]) : x(v[0]), y(v[1]), z(v[2])
{
}


dVector3D::dVector3D(const dVector4D &v)
{
    x = v.x / v.w; y = v.y / v.w; z = v.z / v.w;
}


dVector3D::operator const double *() const
{
    return (double *)&x;
}



dVector3D KCL::operator* (const dVector3D &v, const double factor)
{
    return dVector3D(v.x * factor, v.y * factor, v.z * factor);
}


dVector3D KCL::operator* (const double factor, const dVector3D &v)
{
    return dVector3D(v.x * factor, v.y * factor, v.z * factor);
}


double dVector3D::dot(const dVector3D &a, const dVector3D &b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


dVector3D dVector3D::interpolate(const dVector3D &a, const dVector3D &b, const double t)
{
    return dVector3D(t*b + (1.0f - t)*a);
}

/******************************************************************************************************/
//
//							dVector4D
//
/******************************************************************************************************/



dVector4D::dVector4D() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}


dVector4D::dVector4D(const dVector3D &v)
{
    x = v.x; y = v.y; z = v.z; w = 1;
}

dVector4D::dVector4D(const dVector3D &v, double nw) : x(v.x), y(v.y), z(v.z), w(nw)
{
}

dVector4D::dVector4D(const double nx, const double ny, const double nz, const double nw) : x(nx), y(ny), z(nz), w(nw)
{
}


dVector4D::dVector4D(const double v[]) : x(v[0]), y(v[1]), z(v[2]), w(v[3])
{
}


bool dVector4D::operator== (const dVector4D &v) const
{
    return x == v.x && y == v.y && z == v.z && w == v.w;
}


bool dVector4D::operator!= (const dVector4D &v) const
{
    return x != v.x || y != v.y || z != v.z || w != v.w;
}

dVector4D::operator const double *() const
{
    return (double *)&x;
}


dVector4D KCL::operator- (const dVector4D &v)
{
    return dVector4D(-v.x, -v.y, -v.z, -v.w);
}


dVector4D KCL::operator+ (const dVector4D &a, const dVector4D &b)
{
    return dVector4D(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}


dVector4D KCL::operator- (const dVector4D &a, const dVector4D &b)
{
    return dVector4D(a.x / a.w - b.x / b.w, a.y / a.w - b.y / b.w, a.z / a.w - b.z / b.w, 1);
}


dVector4D KCL::operator* (const dVector4D &v, const double factor)
{
    return dVector4D(v.x * factor, v.y * factor, v.z * factor, v.w * factor);
}


dVector4D KCL::operator* (const double factor, const dVector4D &v)
{
    return dVector4D(v.x * factor, v.y * factor, v.z * factor, v.w * factor);
}


dVector4D KCL::operator* (const Matrix4x4 &M, const dVector4D &v)
{
    return dVector4D(
        M.v11*v.x + M.v21*v.y + M.v31*v.z + M.v41*v.w,
        M.v12*v.x + M.v22*v.y + M.v32*v.z + M.v42*v.w,
        M.v13*v.x + M.v23*v.y + M.v33*v.z + M.v43*v.w,
        M.v14*v.x + M.v24*v.y + M.v34*v.z + M.v44*v.w);
}


dVector4D KCL::operator/ (const dVector4D &v, const double factor)
{
    return dVector4D(v.x / factor, v.y / factor, v.z / factor, v.w / factor);
}


dVector4D& dVector4D::homogenize()
{
    x /= w; y /= w; z /= w; w = 1.0f;
    return *this;
}


void dVector4D::set(const double nx, const double ny, const double nz, const double nw)
{
    x = nx; y = ny; z = nz; w = nw;
}


void dVector4D::set(const dVector3D &v, double w_)
{
    x = v.x;
    y = v.y;
    z = v.z;
    w = w_;
}


void dVector4D::set(const double *v)
{
    x = v[0]; y = v[1]; z = v[2]; w = v[3];
}


double dVector4D::length3()
{
    return sqrt(x * x + y * y + z * z);
}


dVector4D dVector4D::interpolate(const dVector4D &a, const dVector4D &b, const double t)
{
    return dVector4D(t*b + (1.0f - t)*a);
}


double dVector4D::dot(const dVector4D &a, const dVector4D &b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}
