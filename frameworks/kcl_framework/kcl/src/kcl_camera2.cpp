/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_camera2.h>
#include <kcl_aabb.h>
#include <kcl_os.h>

using namespace KCL;

bool Camera2::enable_orrientation_rotation = true;

Camera2::Camera2()
{
	LookAt(Vector3D(0.0f,0.0f,0.0f),Vector3D(0.0f,0.0f,1.0f),Vector3D(0.0f,1.0f,0.0f)) ;
	Perspective(60.0f,512,512,0.001f,100.0f) ;
	Update() ;
}

void Camera2::LookAt(const Vector3D& eye, const Vector3D& center, const Vector3D& up)
{
	m_eye = eye;
	m_center = center;
	m_up = up;

	KCL::Matrix4x4::LookAt(m_view, eye, center, up);
}

void Camera2::LookAt(const Matrix4x4& view)
{
	KCL::Matrix4x4 inv_view;
	KCL::Matrix4x4::InvertModelView(view, inv_view);

	KCL::Vector3D tmp(view.v[12], view.v[13], view.v[14]);

	KCL::Vector4D trans_pos = inv_view * KCL::Vector4D(tmp, 1.0f);

	m_eye =  KCL::Vector3D(trans_pos) * -1.0f;
	m_up = KCL::Vector3D(view.v[4], view.v[5], view.v[6]);
	KCL::Vector3D forward = KCL::Vector3D(view.v[2], view.v[6], view.v[10]) * -1.0f;
	m_center = m_eye + forward;

	m_view = view;
}

void Camera2::Perspective(float fov, KCL::uint32 w, KCL::uint32 h, float near, float far)
{
	if (enable_orrientation_rotation)
	{
		if( w >= h)
		{
			m_is_landscape = false;
			m_aspectRatio = float(w) / float(h);
		}
		else
		{
			m_is_landscape = true;
			m_aspectRatio = float(h) / float(w);
		}
	}
	else
	{
		m_is_landscape = false;
		m_aspectRatio = float(w) / float(h);
	}

	m_fov = fov;

	m_width = float(w);
	m_height = float(h);

	m_left = 0.0f;
	m_right = 0.0f;
	m_bottom = 0.0f;
	m_top = 0.0f;

	SetNearFar(near, far);
}



void Camera2::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	m_is_landscape = false;
	m_fov = 0.0f;
	m_width = right - left;
	m_height = top - bottom;
	m_aspectRatio = m_width / m_height;

	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;
	m_near = zNear;
	m_far = zFar;

	m_near = zNear;
	m_far = zFar;
	KCL::Matrix4x4::Ortho(m_projection, left, right, bottom, top, zNear, zFar);
}


void Camera2::SetNearFar(float near, float far)
{
	m_near = near;
	m_far = far;

	m_depth_linearize_factors.x = m_far / (m_far- m_near);
	m_depth_linearize_factors.y = (m_far * m_near) / (m_near- m_far);
	m_depth_linearize_factors.z = m_near;
	m_depth_linearize_factors.w = m_far;

	KCL::Matrix4x4::Perspective(m_projection, m_fov, m_aspectRatio, near, far);
}


void positiveVertex(Vector3D& p, const Vector3D& minVertex, const Vector3D& maxVertex, const Vector3D& normal)
{
	p.x = normal.x >= 0 ? maxVertex.x : minVertex.x;
	p.y = normal.y >= 0 ? maxVertex.y : minVertex.y;
	p.z = normal.z >= 0 ? maxVertex.z : minVertex.z;
}


bool Camera2::IsVisible(const AABB *const aabb) const
{
	Vector3D posVertex, normal;
	for(int i=0; i < 6; i++)
	{
		normal.set(m_cull_planes[i].x, m_cull_planes[i].y, m_cull_planes[i].z);
		positiveVertex(posVertex, aabb->GetMinVertex(), aabb->GetMaxVertex(), normal);
		if(0 > Vector3D::dot(normal, posVertex) + m_cull_planes[i].w)
			return false;
	}
	return true;
}


void Camera2::Update( bool mirror, const Vector4D *plane)
{
	Matrix4x4 tmp1;

	if( mirror)
	{
		Matrix4x4 inv_view;
		Matrix4x4 tmp;
		Vector4D p1;
		Vector4D p2;

		{
			float d;

			d = 2.0f * Vector4D::dot( *plane, Vector4D( m_eye));
			for( KCL::uint32 i=0; i<3; i++)
			{
				m_eye.v[i] = m_eye.v[i] - plane->v[i] * d;
			}

			d = 2.0f * Vector4D::dot( *plane, Vector4D( m_center));
			for( KCL::uint32 i=0; i<3; i++)
			{
				m_center.v[i] = m_center.v[i] - plane->v[i] * d;
			}

			d = 2.0f * Vector3D::dot( Vector3D(plane->v), m_up);
			for( KCL::uint32 i=0; i<3; i++)
			{
				m_up.v[i] = m_up.v[i] - plane->v[i] * d;
			}

			LookAt( m_eye, m_center, m_up);

			tmp.scale( Vector3D( -1, 1, 1));

			m_view = m_view * tmp;
		}


		inv_view = Matrix4x4::Invert4x3( m_view);
		inv_view.transpose();

		float d = Vector4D::dot( *plane, Vector4D( m_eye));

		if( d < 0.0f)
		{
			p1 = *plane;
		}
		else
		{
			p1 = -*plane;
		}

		mult4x4( inv_view, p1, p2);

		m_projection.adjustNearPlaneForPerspective( p2);
	}

	m_viewProjection =  m_view * m_projection;

	tmp1 = m_view;
	tmp1.v[12] = 0.0f;
	tmp1.v[13] = 0.0f;
	tmp1.v[14] = 0.0f;

	m_viewProjectionOrigo =  tmp1 * m_projection;


	m_cull_planes[CULLPLANE_LEFT].x = m_viewProjection.v[ 3] + m_viewProjection.v[ 0];
	m_cull_planes[CULLPLANE_LEFT].y = m_viewProjection.v[ 7] + m_viewProjection.v[ 4];
	m_cull_planes[CULLPLANE_LEFT].z = m_viewProjection.v[11] + m_viewProjection.v[ 8];
	m_cull_planes[CULLPLANE_LEFT].w = m_viewProjection.v[15] + m_viewProjection.v[12];
	m_cull_planes[CULLPLANE_RIGHT].x = m_viewProjection.v[ 3] - m_viewProjection.v[ 0];
	m_cull_planes[CULLPLANE_RIGHT].y = m_viewProjection.v[ 7] - m_viewProjection.v[ 4];
	m_cull_planes[CULLPLANE_RIGHT].z = m_viewProjection.v[11] - m_viewProjection.v[ 8];
	m_cull_planes[CULLPLANE_RIGHT].w = m_viewProjection.v[15] - m_viewProjection.v[12];
	if (zRangeZeroToOneGlobalOpt)
	{
		//Note: DX has Z: 0..1 in clip space, GL has Z in -1..1
		m_cull_planes[CULLPLANE_NEAR].x = m_viewProjection.v[ 2];
		m_cull_planes[CULLPLANE_NEAR].y = m_viewProjection.v[ 6];
		m_cull_planes[CULLPLANE_NEAR].z = m_viewProjection.v[10];
		m_cull_planes[CULLPLANE_NEAR].w = m_viewProjection.v[14];
	} else {
		m_cull_planes[CULLPLANE_NEAR].x = m_viewProjection.v[3] + m_viewProjection.v[2];
		m_cull_planes[CULLPLANE_NEAR].y = m_viewProjection.v[7] + m_viewProjection.v[6];
		m_cull_planes[CULLPLANE_NEAR].z = m_viewProjection.v[11] + m_viewProjection.v[10];
		m_cull_planes[CULLPLANE_NEAR].w = m_viewProjection.v[15] + m_viewProjection.v[14];
	}
	m_cull_planes[CULLPLANE_FAR].x = m_viewProjection.v[ 3] - m_viewProjection.v[ 2];
	m_cull_planes[CULLPLANE_FAR].y = m_viewProjection.v[ 7] - m_viewProjection.v[ 6];
	m_cull_planes[CULLPLANE_FAR].z = m_viewProjection.v[11] - m_viewProjection.v[10];
	m_cull_planes[CULLPLANE_FAR].w = m_viewProjection.v[15] - m_viewProjection.v[14];
	m_cull_planes[CULLPLANE_BOTTOM].x = m_viewProjection.v[ 3] + m_viewProjection.v[ 1];
	m_cull_planes[CULLPLANE_BOTTOM].y = m_viewProjection.v[ 7] + m_viewProjection.v[ 5];
	m_cull_planes[CULLPLANE_BOTTOM].z = m_viewProjection.v[11] + m_viewProjection.v[ 9];
	m_cull_planes[CULLPLANE_BOTTOM].w = m_viewProjection.v[15] + m_viewProjection.v[13];
	m_cull_planes[CULLPLANE_TOP].x = m_viewProjection.v[ 3] - m_viewProjection.v[ 1];
	m_cull_planes[CULLPLANE_TOP].y = m_viewProjection.v[ 7] - m_viewProjection.v[ 5];
	m_cull_planes[CULLPLANE_TOP].z = m_viewProjection.v[11] - m_viewProjection.v[ 9];
	m_cull_planes[CULLPLANE_TOP].w = m_viewProjection.v[15] - m_viewProjection.v[13];

	for( KCL::uint32 i=0; i<=CULLPLANE_NEAR; i++)
	{
		float d = m_cull_planes[i].length3();

		m_cull_planes[i] /= d;
	}


	if( m_is_landscape)
	{
		Matrix4x4 rotate;

		rotate.rotate(90,Vector3D(0,0,1));

		m_viewProjection = m_viewProjection * rotate;
		m_viewProjectionOrigo = m_viewProjectionOrigo * rotate;
	}
}


void Camera2::OrthoFocus(const AABB *const aabb, const Vector3D& eye, const Vector3D& center, const Vector3D& up)
{
	LookAt(eye, center, up);
	Vector4D vertices[8];
	aabb->CalculateVertices4D(vertices);

	AABB tmp;

	for(size_t i=0; i<8; ++i)
	{
		vertices[i] = m_view *vertices[i];

		tmp.Merge(Vector3D(vertices[i].x, vertices[i].y, vertices[i].z));
	}

	m_left   = tmp.GetMinVertex().x;
	m_right  = tmp.GetMaxVertex().x;
	m_bottom = tmp.GetMinVertex().y;
	m_top    = tmp.GetMaxVertex().y;
	m_near   = tmp.GetMinVertex().z;
	m_far    = tmp.GetMaxVertex().z;

	m_is_landscape = false;
	m_fov = 0.0f;
	m_aspectRatio = (m_right-m_left)/(m_top-m_bottom);
	KCL::Matrix4x4::Scale_translate_to_fit(m_projection, tmp.GetMinVertex(), tmp.GetMaxVertex());
}


void look(Matrix4x4 &output, const Vector3D &pos, const Vector3D &dir, const Vector3D &up);
void mult(Matrix4x4 &output, const Matrix4x4 &a, const Matrix4x4 &b);
void mulHomogenPoint( Vector4D &output, const Matrix4x4 &m, const Vector4D &v);
void cross( Vector3D &result, const Vector3D &a, const Vector3D &b);
void linCombVector3(Vector3D& result, const Vector3D &pos, const Vector3D &dir, const double t);


void Camera2::OrthoFocusLispsm(const AABB *const aabb, const Vector3D& eye, const Vector3D& viewdir, const Vector3D& lightdir, float znear)
{
	OrthoFocus( aabb, eye, eye - lightdir, Vector3D( 0, 1, 0));
	return;

	Vector3D a, up;
	Vector4D vertices[8];
	Vector3D new_eye;
	AABB tmp;
	Matrix4x4 lispMtx;
	Matrix4x4 tmpm;


	cross( a, lightdir, viewdir);
	cross( up, a, lightdir);

	look( m_view, eye, lightdir, up);

	aabb->CalculateVertices4D( vertices);

	for(size_t i=0; i<8; ++i)
	{
		mulHomogenPoint( vertices[i], m_view, vertices[i]);

		tmp.Merge(Vector3D(vertices[i].x, vertices[i].y, vertices[i].z));
	}

	const Vector3D &max = tmp.GetMaxVertex();
	const Vector3D &min = tmp.GetMinVertex();

	float dotProd = Vector3D::dot( viewdir, lightdir);
	float sinGamma = sqrt(1.0f-dotProd*dotProd);

	const double factor = 1.0/sinGamma;
	const double d = fabs(max.y - min.y);
	const double z_n = znear * factor;
	const double z_f = z_n + d*sinGamma;
	double n = (z_n + sqrt(z_n*z_f) )/sinGamma + fabs( min.y);
	//if( n <= znear)
	//{
	//	n = znear + 0.001f;
	//	printf(".");
	//}
	const double f = n+d;

	linCombVector3( new_eye, eye, up, -(n - znear));

	look( m_view, new_eye, lightdir, up);

	lispMtx.v[5] = (f+n)/(f-n);
	lispMtx.v[13] = -2*f*n/(f-n);
	lispMtx.v[7] = 1.0f;
	lispMtx.v[15] = 0.0f;

	mult( tmpm, lispMtx, m_view);

	tmp.Reset();
	aabb->CalculateVertices4D(vertices);

	for(size_t i=0; i<8; ++i)
	{
		mulHomogenPoint( vertices[i], tmpm, vertices[i]);

		tmp.Merge(Vector3D(vertices[i].x, vertices[i].y, vertices[i].z));
	}

	KCL::Matrix4x4::Scale_translate_to_fit(m_projection, tmp.GetMinVertex(), tmp.GetMaxVertex());
	mult( m_projection, m_projection, lispMtx);

	Matrix4x4 rh2lf;
	rh2lf.scale( Vector3D( 1, 1, -1));
	mult(m_projection,rh2lf,m_projection);

}

void Camera2::OrthoCrop(float left, float right, float bottom, float top, float zNear, float zFar, const KCL::Matrix4x4 & crop)
{
	// Set orthographic projection and apply the crop matrix
	Ortho(left, right, bottom, top, zNear, zFar);
	m_projection = m_projection * crop;
}

double det2x2(const double a1, const double a2, const double b1, const double b2)
{
	return a1*b2 - b1*a2;
}


void cross( Vector3D &result, const Vector3D &a, const Vector3D &b)
{
	result.v[0] =  det2x2(a.v[1],b.v[1],a.v[2],b.v[2]);
	result.v[1] = -det2x2(a.v[0],b.v[0],a.v[2],b.v[2]);
	result.v[2] =  det2x2(a.v[0],b.v[0],a.v[1],b.v[1]);
}


void look(Matrix4x4 &output, const Vector3D &pos, const Vector3D &dir, const Vector3D &up)
{
	Vector3D dirN;
	Vector3D upN;
	Vector3D lftN;

	cross(lftN,dir,up);
	Vector3D::normalize(lftN);

	cross(upN,lftN,dir);
	Vector3D::normalize(upN);


	dirN = dir;
	dirN.normalize();

	output.v[ 0] = lftN[0];
	output.v[ 1] = upN[0];
	output.v[ 2] = -dirN[0];
	output.v[ 3] = 0.0;

	output.v[ 4] = lftN[1];
	output.v[ 5] = upN[1];
	output.v[ 6] = -dirN[1];
	output.v[ 7] = 0.0;

	output.v[ 8] = lftN[2];
	output.v[ 9] = upN[2];
	output.v[10] = -dirN[2];
	output.v[11] = 0.0;

	output.v[12] = -Vector3D::dot(lftN,pos);
	output.v[13] = -Vector3D::dot(upN,pos);
	output.v[14] = Vector3D::dot(dirN,pos);
	output.v[15] = 1.0;
}


void multUnSave(Matrix4x4 &output, const Matrix4x4 &a, const Matrix4x4 &b)
{
	const int SIZE = 4;
	int iCol;
	for(iCol = 0; iCol < SIZE; iCol++) {
		const int cID = iCol*SIZE;
		int iRow;
		for(iRow = 0; iRow < SIZE; iRow++) {
			const int id = iRow+cID;
			int k;
			output.v[id] = a[iRow]*b[cID];
			for(k = 1; k < SIZE; k++) {
				output.v[id] += a[iRow+k*SIZE]*b[k+cID];
			}
		}
	}
}

void mult(Matrix4x4 &output, const Matrix4x4 &a, const Matrix4x4 &b)
{
	if(a == output) {
		Matrix4x4 tmpA;
		tmpA = a;
		if(b == output) {
			multUnSave(output,tmpA,tmpA);
		}
		else {
			multUnSave(output,tmpA,b);
		}
	}
	else {
		if(b == output) {
			Matrix4x4 tmpB;
			tmpB = b;
			multUnSave(output,a,tmpB);
		}
		else {
			multUnSave(output,a,b);
		}
	}
}


void mulHomogenPoint( Vector4D &output, const Matrix4x4 &m, const Vector4D &v)
{
	//if v == output -> overwriting problems -> so store in temp
	double x = m.v[0]*v.v[0] + m.v[4]*v.v[1] + m.v[ 8]*v.v[2] + m.v[12];
	double y = m.v[1]*v.v[0] + m.v[5]*v.v[1] + m.v[ 9]*v.v[2] + m.v[13];
	double z = m.v[2]*v.v[0] + m.v[6]*v.v[1] + m.v[10]*v.v[2] + m.v[14];
	double w = m.v[3]*v.v[0] + m.v[7]*v.v[1] + m.v[11]*v.v[2] + m.v[15];

	output.v[0] = x/w;
	output.v[1] = y/w;
	output.v[2] = z/w;
}


void linCombVector3(Vector3D& result, const Vector3D &pos, const Vector3D &dir, const double t)
{
	int i;
	for(i = 0; i < 3; i++)
	{
		result.v[i] = pos.v[i] + t * dir.v[i];
	}
}


void Camera2::Perspective( int tile_x, int tile_y, float fov, KCL::uint32 w, KCL::uint32 h, float near, float far)
{
	if (enable_orrientation_rotation)
	{
		if( w >= h)
		{
			m_is_landscape = false;
			m_aspectRatio = float(w) / float(h);
		}
		else
		{
			m_is_landscape = true;
			m_aspectRatio = float(h) / float(w);
		}
	}
	else
	{
		m_is_landscape = false;
		m_aspectRatio = float(w) / float(h);
	}

	m_fov = fov;
	m_near = near;
	m_far = far;

	m_left = 0.0f;
	m_right = 0.0f;
	m_bottom = 0.0f;
	m_top = 0.0f;


	float aperture = (float)tanf( KCL::Math::Rad( m_fov / 2.0f)) * m_near;
	float left0 = -aperture * m_aspectRatio;
	float right0 = aperture * m_aspectRatio;
	float top0 = aperture;
	float bottom0 = -aperture;

	float left_i = tile_x / 15.0;
	float right_i = (tile_x + 1) / 15.0;
	float bottom_i = tile_y / 15.0;
	float top_i = (tile_y + 1) / 15.0;

	float left = left0 * (1.0f - left_i) + right0 * left_i;
	float right = left0 * (1.0f - right_i) + right0 * right_i;
	float bottom = bottom0 * (1.0f - bottom_i) + top0 * bottom_i;
	float top = bottom0 * (1.0f - top_i) + top0 * top_i;

	KCL::Matrix4x4::Frustum( m_projection, left, right, bottom, top, near, far);
}


void Camera2::CalculateFullscreenBillboard( int tile_x, int tile_y, KCL::Vector3D vertices[4])
{
	KCL::Vector3D eye_forward;
	KCL::Vector3D eye_left;
	KCL::Vector3D eye_up;
	KCL::Vector3D eye_position = m_eye;
	KCL::Vector3D tmp0;
	KCL::Vector3D tmp_left;
	KCL::Vector3D tmp_right;
	KCL::Vector3D tmp_top;
	KCL::Vector3D tmp_bottom;


	float aperture = (float)tanf( KCL::Math::Rad( m_fov / 2.0f)) * m_near;
	float left0 = -aperture * m_aspectRatio;
	float right0 = aperture * m_aspectRatio;
	float top0 = aperture;
	float bottom0 = -aperture;

	float left_i = tile_x / 15.0;
	float right_i = (tile_x + 1) / 15.0;
	float bottom_i = tile_y / 15.0;
	float top_i = (tile_y + 1) / 15.0;

	float left = left0 * (1.0f - left_i) + right0 * left_i;
	float right = left0 * (1.0f - right_i) + right0 * right_i;
	float bottom = bottom0 * (1.0f - bottom_i) + top0 * bottom_i;
	float top = bottom0 * (1.0f - top_i) + top0 * top_i;


	m_depth_linearize_factors.x = m_far / (m_far- m_near);
	m_depth_linearize_factors.y = (m_far * m_near) / (m_near- m_far);
	m_depth_linearize_factors.z = m_near;
	m_depth_linearize_factors.w = m_far;

	eye_forward.set( -m_view.v13, -m_view.v23, -m_view.v33);
	eye_up.set( m_view.v12, m_view.v22, m_view.v32);
	eye_left.set( m_view.v11, m_view.v21, m_view.v31);

	tmp0 = eye_forward * m_depth_linearize_factors.z;
	tmp_left = eye_left * left;
	tmp_right = eye_left * right;

	tmp_top = eye_up * top;
	tmp_bottom = eye_up * bottom;

	for( KCL::uint32 i=0; i<3; i++)
	{
		vertices[0].v[i] = eye_position.v[i] + tmp0.v[i] + tmp_left.v[i] + tmp_bottom.v[i];
		vertices[1].v[i] = eye_position.v[i] + tmp0.v[i] + tmp_right.v[i] + tmp_bottom.v[i];
		vertices[2].v[i] = eye_position.v[i] + tmp0.v[i] + tmp_left.v[i] + tmp_top.v[i];
		vertices[3].v[i] = eye_position.v[i] + tmp0.v[i] + tmp_right.v[i] + tmp_top.v[i];
	}
}

void Camera2::CalculateFullscreenBillboard( KCL::Vector3D vertices[4])
{
	KCL::Vector3D eye_forward;
	KCL::Vector3D eye_left;
	KCL::Vector3D eye_up;
	KCL::Vector3D eye_position = m_eye;
	KCL::Vector3D tmp0;
	KCL::Vector3D tmp1;
	KCL::Vector3D tmp2;
	KCL::Vector3D fullscreen_billboard_factors;


	float aperture = (float)tanf( KCL::Math::Rad( m_fov / 2.0f)) * m_near;
	//float left = -aperture * m_aspectRatio;//unused variable
	float right = aperture * m_aspectRatio;
	//float bottom = -aperture ;//unused variable
	float top = aperture;

	fullscreen_billboard_factors.x = right;
	fullscreen_billboard_factors.y = top;

	m_depth_linearize_factors.x = m_far / (m_far- m_near);
	m_depth_linearize_factors.y = (m_far * m_near) / (m_near- m_far);
	m_depth_linearize_factors.z = m_near;
	m_depth_linearize_factors.w = m_far;

	eye_forward.set( -m_view.v13, -m_view.v23, -m_view.v33);
	eye_up.set( m_view.v12, m_view.v22, m_view.v32);
	eye_left.set( m_view.v11, m_view.v21, m_view.v31);

	tmp0 = eye_forward * m_depth_linearize_factors.z;
	tmp1 = eye_left * fullscreen_billboard_factors.x;
	tmp2 = eye_up * fullscreen_billboard_factors.y;

	for( KCL::uint32 i=0; i<3; i++)
	{
		vertices[0].v[i] = eye_position.v[i] + tmp0.v[i] - tmp1.v[i] - tmp2.v[i];
		vertices[1].v[i] = eye_position.v[i] + tmp0.v[i] + tmp1.v[i] - tmp2.v[i];
		vertices[2].v[i] = eye_position.v[i] + tmp0.v[i] - tmp1.v[i] + tmp2.v[i];
		vertices[3].v[i] = eye_position.v[i] + tmp0.v[i] + tmp1.v[i] + tmp2.v[i];
	}
}

void Camera2::GetBillboardTranform(Matrix4x4& matrix)
{
	KCL::Matrix4x4::Zero(matrix);

	KCL::Vector3D eye_forward(-m_view.v13, -m_view.v23, -m_view.v33);
	KCL::Vector3D eye_left(m_view.v11, m_view.v21, m_view.v31);
	KCL::Vector3D eye_up(m_view.v12, m_view.v22, m_view.v32);

	float distance = m_near + (m_far - m_near) * 0.999f;
	float tanFovY = distance * (float)tanf( KCL::Math::Rad( m_fov / 2.0f));
	float tanFovX = tanFovY * m_aspectRatio;

	eye_left *= tanFovX;
	matrix.v11 = eye_left.x;
	matrix.v12 = eye_left.y;
	matrix.v13 = eye_left.z;

	eye_up *= tanFovY;
	matrix.v21 = eye_up.x;
	matrix.v22 = eye_up.y;
	matrix.v23 = eye_up.z;

	KCL::Vector3D center = m_eye + distance * eye_forward;
	matrix.v41 = center.x;
	matrix.v42 = center.y;
	matrix.v43 = center.z;
	matrix.v44 = 1;
}

void Camera2::GetBillboardTranform(int tile_x, int tile_y, Matrix4x4& matrix)
{
	const int tile_x_count = 15;
	const int tile_y_count = 15;

	KCL::Matrix4x4::Zero(matrix);

	KCL::Vector3D eye_forward(-m_view.v13, -m_view.v23, -m_view.v33);
	KCL::Vector3D eye_left(m_view.v11, m_view.v21, m_view.v31);
	KCL::Vector3D eye_up(m_view.v12, m_view.v22, m_view.v32);

	float distance = m_near + (m_far - m_near) * 0.999f;
	float tanFovY = distance * (float)tanf( KCL::Math::Rad( m_fov / 2.0f));
	float tanFovX = tanFovY * m_aspectRatio;

	float tileWidth = 1.0f / tile_x_count;
	float tileHeight = 1.0f / tile_y_count;

	eye_left *= tanFovX * tileWidth;
	matrix.v11 = eye_left.x;
	matrix.v12 = eye_left.y;
	matrix.v13 = eye_left.z;

	eye_up *= tanFovY * tileHeight;
	matrix.v21 = eye_up.x;
	matrix.v22 = eye_up.y;
	matrix.v23 = eye_up.z;

	KCL::Vector3D center = m_eye + distance * eye_forward;
	center += eye_left * (tileWidth * tile_x - 0.5f);
	center += eye_up * (tileHeight * tile_y - 0.5f);

	matrix.v41 = center.x;
	matrix.v42 = center.y;
	matrix.v43 = center.z;
	matrix.v44 = 1;
}


void KCL::Camera2::CalculateRaysToFullscreenBillboard( KCL::Vector3D vertices[4], bool inViewSpace)
{
	KCL::Vector3D eye_forward;
	KCL::Vector3D eye_left;
	KCL::Vector3D eye_up;
	//KCL::Vector3D eye_position = m_eye;
	KCL::Vector3D tmp0;
	KCL::Vector3D tmp1;
	KCL::Vector3D tmp2;
	KCL::Vector3D fullscreen_billboard_factors;


	float aperture = (float)tanf( KCL::Math::Rad( m_fov / 2.0f)) * m_near;
	//float left = -aperture * m_aspectRatio;//unused variable
	float right = aperture * m_aspectRatio;
	//float bottom = -aperture ;//unused variable
	float top = aperture;

	fullscreen_billboard_factors.x = right;
	fullscreen_billboard_factors.y = top;

	m_depth_linearize_factors.x = m_far / (m_far- m_near);
	m_depth_linearize_factors.y = (m_far * m_near) / (m_near- m_far);
	m_depth_linearize_factors.z = m_near;
	m_depth_linearize_factors.w = m_far;

	if(inViewSpace)
	{
		eye_forward.set(0, 0, -1); //change at 714 if not z is forward
		eye_up.set(-1, 0, 0);
		eye_left.set(0, 1, 0);

		//eye_forward.set(0, 0, 1); //change at 714 if not z is forward
		//eye_up.set(0, 1, 0);
		//eye_left.set(1, 0, 0);
	}
	else
	{
		eye_forward.set( -m_view.v13, -m_view.v23, -m_view.v33);
		eye_up.set( m_view.v12, m_view.v22, m_view.v32);
		eye_left.set( m_view.v11, m_view.v21, m_view.v31);
	}

	tmp0 = eye_forward * m_depth_linearize_factors.z;
	tmp1 = eye_left * fullscreen_billboard_factors.x;
	tmp2 = eye_up * fullscreen_billboard_factors.y;

	for( KCL::uint32 i=0; i<3; i++)
	{
		vertices[0].v[i] = tmp0.v[i] - tmp1.v[i] - tmp2.v[i];
		vertices[1].v[i] = tmp0.v[i] + tmp1.v[i] - tmp2.v[i];
		vertices[2].v[i] = tmp0.v[i] - tmp1.v[i] + tmp2.v[i];
		vertices[3].v[i] = tmp0.v[i] + tmp1.v[i] + tmp2.v[i];
	}

	for( uint32 i=0; i<4; i++)
	{
		float l;

		if(inViewSpace)
		{
			l = fabs( 1.0f / vertices[i].z);
		}
		else
		{
			l = KCL::Vector3D::dot( eye_forward, vertices[i]);
			l = 1.0f / l;
		}

		vertices[i] *= l;
	}
}


void KCL::Camera2::CalculateVSRaysToFullscreenBillboard(KCL::Vector4D vertices[4])
{
	KCL::Vector4D vs_corners[4];
	KCL::Matrix4x4 inv_p;

	KCL::Matrix4x4::Invert4x4(m_projection, inv_p);

	vs_corners[0].set(-1.0f, -1.0f, -1.0f, 1.0f);
	vs_corners[1].set(+1.0f, -1.0f, -1.0f, 1.0f);
	vs_corners[2].set(-1.0f, +1.0f, -1.0f, 1.0f);
	vs_corners[3].set(+1.0f, +1.0f, -1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		vs_corners[i] = inv_p * vs_corners[i];
		vs_corners[i] /= vs_corners[i].w;
		vs_corners[i] /= -vs_corners[i].z;
		vertices[i] = vs_corners[i];
	}
}


void KCL::Camera2::LookAtOmni(const Vector3D& eye, KCL::uint32 dir)
{
	KCL::Vector3D forward;
	KCL::Vector3D up;

	GetOmniVectors(dir, forward, up);

	m_eye = eye;
	m_center = eye + forward;
	m_up = up;

	KCL::Matrix4x4::LookAt(m_view, m_eye, m_center, m_up);
}


void KCL::Camera2::GetOmniVectors(KCL::uint32 dir, KCL::Vector3D &forward, KCL::Vector3D &up)
{
	switch (dir)
	{
	case 0:
	{
		forward = KCL::Vector3D(1.0f, 0.0f, 0.0f);
		up = KCL::Vector3D(0.0f, -1.0f, 0.0f);
		break;
	}
	case 1:
	{
		forward = KCL::Vector3D(-1.0f, 0.0f, 0.0f);
		up = KCL::Vector3D(0.0f, -1.0f, 0.0f);
		break;
	}
	case 2:
	{
		forward = KCL::Vector3D(0.0f, 1.0f, 0.0f);
		up = KCL::Vector3D(0.0f, 0.0f, 1.0f);
		break;
	}
	case 3:
	{
		forward = KCL::Vector3D(0.0f, -1.0f, 0.0f);
		up = KCL::Vector3D(0.0f, 0.0f, -1.0f);
		break;
	}
	case 4:
	{
		forward = KCL::Vector3D(0.0f, 0.0f, 1.0f);
		up = KCL::Vector3D(0.0f, -1.0f, 0.0f);
		break;
	}
	case 5:
	default:
	{
		forward = KCL::Vector3D(0.0f, 0.0f, -1.0f);
		up = KCL::Vector3D(0.0f, -1.0f, 0.0f);
		break;
	}
	}
}
