/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "tess_base.h"

#include "kcl_camera2.h"
#include "kcl_io.h"


TessBase::TessBase(const GlobalTestEnvironment * const gte)
	: TestBase(gte)
	, m_score(0)
{
	m_camera = new KCL::Camera2;

	m_camera->Perspective(90, gte->GetTestDescriptor()->m_viewport_width, gte->GetTestDescriptor()->m_viewport_height, 0.1f, 100.0f);
}


TessBase::~TessBase()
{
	delete m_camera;
}


bool TessBase::animate(const int time)
{
	SetAnimationTime(time);

	float f = m_time * 0.0001f;
	float ff = KCL::Math::Rad(f);
	const float d = 0.3f;

	KCL::Vector3D eye(d * sinf(f), d * 0.5f, d * cosf(f));
	KCL::Vector3D ref(0.0f, 0.0f, 0.0f);
	KCL::Vector3D up(0.0f, 1.0f, 0.0f);

	m_camera->LookAt(eye, ref, up);

	m_camera->Update();

	if (m_frames > m_score)
	{
		m_score = m_frames;
	}

	return time < m_settings->m_play_time;
}


KCL::KCL_Status TessBase::init()
{
	KCL::AssetFile file("tess/B_0");

	if (!file.GetLastError())
	{
		KCL::uint32 num_vertices;
		KCL::uint32 num_indices;

		file.Read(&num_vertices, 4, 1);
		file.Read(&num_indices, 4, 1);

		m_vertices.resize(num_vertices);
		m_indices.resize(num_indices);

		file.Read(&m_indices[0], 2, num_indices);
		file.Read(&m_vertices[0].v, 4, num_vertices * 3);
	}
	else
	{
		INFO("Can not load tess/B0");
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	m_mesh1.model.identity();
	m_mesh1.model.scale(KCL::Vector3D(0.15f, 0.15f, 0.15f));

	m_mesh2.model.identity();
	m_mesh2.model.rotate(90.0f, KCL::Vector3D(0.0, 1.0, 0.0));
	m_mesh2.model.scale(KCL::Vector3D(3.0f, 3.0f, 3.0f));

	m_mesh3.model.identity();
	m_mesh3.model.scale(KCL::Vector3D(1.5f, 1.5f, 1.5f));

	return KCL::KCL_TESTERROR_NOERROR;
}

