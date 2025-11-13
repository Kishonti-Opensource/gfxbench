/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__CAMERA_H__
#define __XX__CAMERA_H__

#include <kcl_math3d.h>
#include <kcl_aabb.h>

#include <cassert>

namespace KCL
{
	const size_t CULLPLANE_LEFT = 0;
	const size_t CULLPLANE_RIGHT = 1;
	const size_t CULLPLANE_BOTTOM = 2;
	const size_t CULLPLANE_TOP = 3;
	const size_t CULLPLANE_FAR = 4;
	const size_t CULLPLANE_NEAR = 5;

	class Camera2
	{
	public:
		Camera2();

		void LookAt(const Vector3D& eye, const Vector3D& center, const Vector3D& up);
		void LookAt(const Matrix4x4& view);
		void LookAtOmni(const Vector3D& eye, KCL::uint32 dir);
		void Perspective(float fov, KCL::uint32 w, KCL::uint32 h, float near, float far);
		void Ortho(float left, float right, float bottom, float top, float zNear, float zFar);
		void SetNearFar(float near, float far);
		void OrthoFocus(const AABB *const aabb, const Vector3D& eye, const Vector3D& center, const Vector3D& up);
		void OrthoFocusLispsm(const AABB *const aabb, const Vector3D& eye, const Vector3D& viewdir, const Vector3D& lightdir, float znear);
		void OrthoCrop(float left, float right, float bottom, float top, float zNear, float zFar, const KCL::Matrix4x4 & crop);

		///Calculates ViewProjection and cull planes
		void Update( bool mirror = false, const Vector4D *plane = 0);

		const Vector3D& GetEye() const { return m_eye; }
		const Vector3D& GetCenter() const { return m_center; }
		const Vector3D& GetUp() const { return m_up; }
		const Matrix4x4& GetView() const { return m_view; }
		const Matrix4x4& GetProjection() const { return m_projection; }
		const Matrix4x4& GetViewProjection() const { return m_viewProjection; }
		const Matrix4x4& GetViewProjectionOrigo() const { return m_viewProjectionOrigo; }
		float GetFov() const { return m_fov; }
		float GetWidth() const { return m_width; }
		float GetHeight() const { return m_height; }
		float GetAspectRatio() const { return m_aspectRatio; }
		float GetNear() const { return m_near; }
		float GetFar() const { return m_far; }
		float GetLeft() const { return m_left; }
		float GetRight() const { return m_right; }
		float GetBottom() const { return m_bottom; }
		float GetTop() const { return m_top; }

		const Vector4D& GetCullPlane(size_t idx) const
		{
			assert(idx < 6);
			return m_cull_planes[idx];
		}

		bool IsVisible(const AABB *const aabb) const;

		void CalculateFullscreenBillboard( KCL::Vector3D vertices[4]);

		void CalculateFullscreenBillboard( int tile_x, int tile_y, KCL::Vector3D vertices[4]);

		void CalculateRaysToFullscreenBillboard( KCL::Vector3D vertices[4], bool inViewSpace = false);

		void CalculateVSRaysToFullscreenBillboard(KCL::Vector4D vertices[4]);

		void Perspective( int tile_x, int tile_y, float fov, KCL::uint32 w, KCL::uint32 h, float near, float far);

		void GetBillboardTranform(Matrix4x4& matrix);

		void GetBillboardTranform(int tile_x, int tile_y, Matrix4x4& matrix);

		static void GetOmniVectors(KCL::uint32 dir, KCL::Vector3D &forward, KCL::Vector3D &up);

		KCL::Vector4D m_depth_linearize_factors;

		static bool enable_orrientation_rotation;

	private:
		Vector3D m_eye;
		Vector3D m_center;
		Vector3D m_up;

		Matrix4x4 m_view;
		Matrix4x4 m_projection;
		Matrix4x4 m_viewProjection;
		Matrix4x4 m_viewProjectionOrigo;

		float m_fov;
		float m_width;
		float m_height;
		float m_aspectRatio;
		float m_near;
		float m_far;

		float m_left;
		float m_right;
		float m_bottom;
		float m_top;

		Vector4D m_cull_planes[6];

		bool m_is_landscape;
	};
}

#endif //__XX__CAMERA_H__

