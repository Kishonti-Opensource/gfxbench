/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "newnotificationmanager.h"
#include "d3d11/shader.h"
#include "kcl_buffer.h"

namespace GLB {

class NewNotificationManagerDX: public NewNotificationManager
{
protected:
		Shader* m_shader;
		KCL::uint32 m_vbo;
		KCL::uint32 m_ebo;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;
		KCL::ConstantBuffer *m_constantBuffer;
public:
		NewNotificationManagerDX();
		virtual ~NewNotificationManagerDX();
		virtual void ShowLogo(bool stretch, bool blend);
		virtual KCL::Texture *CreateTexture(const KCL::Image* img, bool releaseUponCommit = false);
};

}