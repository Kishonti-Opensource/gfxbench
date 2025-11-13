/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_base.h"

#include "kcl_math3d.h"

#include "d3d11/dxb_image.h"
#include "d3d11/dxb_buffer.h"
#include "d3d11/dxb_texture.h"

#include <kcl_os.h>

using namespace KCL;

void DXB::Initialize()
{
	Release();
	KCL::Initialize(true);
	KCL::VertexBuffer::factory = new DXB::DXBVertexBuffer::Factory();
	KCL::IndexBuffer::factory = new DXB::DXBIndexBuffer::Factory();
	KCL::ConstantBuffer::factory = new DXB::DXBConstantBuffer::Factory();
}

void DXB::Release()
{
	KCL::Release();
	delete KCL::VertexBuffer::factory;
	delete KCL::IndexBuffer::factory;
	delete KCL::ConstantBuffer::factory;
}