/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_DEBUG_RENDERER_H
#define GLB_DEBUG_RENDERER_H

#include "opengl/glb_shader2.h"
#include <kcl_base.h>
#include <vector>

namespace GLB
{
    class DebugRenderer
    {
    public:
        DebugRenderer();
        ~DebugRenderer();

        void SetTransform(const KCL::Matrix4x4 &matrix);
        void ClearTransform();

        void DrawLine(const KCL::Vector3D &v1, const KCL::Vector3D &v2, const KCL::Vector4D &color);
        void DrawTriangle(const KCL::Vector3D &v1, const KCL::Vector3D &v2, const KCL::Vector3D &v3, const KCL::Vector4D &color);
        void DrawQuad(const KCL::Vector3D &v1, const KCL::Vector3D &v2, const KCL::Vector3D &v3, const KCL::Vector3D &v4, const KCL::Vector4D &color);
        void DrawFrusum(const KCL::Vector3D v[8], const KCL::Vector4D &color);
        void DrawFrusum(const KCL::Camera2 *camera, const KCL::Vector4D &color, const KCL::Matrix4x4 &model_matrix = KCL::Matrix4x4());
        void DrawAABB(const KCL::Vector3D &min, const KCL::Vector3D &max, const KCL::Vector4D &color);
        void DrawSphere(const KCL::Vector3D &center, float radius, const KCL::Vector4D &color);
        void DrawCone(const KCL::Vector3D &center, float radius, const KCL::Vector4D &color);

        void Render(const KCL::Camera2 *camera);
        void Render(const KCL::Matrix4x4 &viewProjectionMatrix);
    private:
        struct DebugVertex
        {
            KCL::Vector3D m_pos;
            KCL::Vector4D m_color;

            DebugVertex() {}
            DebugVertex(const KCL::Vector3D &pos, const KCL::Vector4D &color)
            {
                m_pos = pos;
                m_color = color;
            }
        };
        std::vector<DebugVertex> m_vertices;
        void AddVertex(const KCL::Vector3D &pos, const KCL::Vector4D &color);

        GLBShader2 *m_shader;
        KCL::uint32 m_vao;
        KCL::uint32 m_vbo;

        KCL::uint32 m_pos_attrib;
        KCL::uint32 m_color_attrib;

        KCL::Matrix4x4 m_transform;
};
}

#endif