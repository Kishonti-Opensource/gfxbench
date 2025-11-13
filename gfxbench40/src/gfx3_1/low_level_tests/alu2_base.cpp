/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "alu2_base.h"

#include "kcl_io.h"

#include <sstream>

ALUTest2_Base::ALUTest2_Base(const GlobalTestEnvironment* const gte)
	: GLB::TestBase(gte)
{
	m_score = 0;
	m_light_count = 0;
}


ALUTest2_Base::~ALUTest2_Base()
{

}


bool ALUTest2_Base::animate(const int time)
{
	SetAnimationTime(time);
	if (m_frames > m_score)
	{
		m_score = m_frames;
	}

	// Lights on the walls	
	for (KCL::uint32 i = 5; i < m_light_pos_array.size(); i++)
	{
		m_animated_pos_array[i] = m_light_pos_array[i];
		m_animated_pos_array[i].y = m_animated_pos_array[i].y + 2.0f * cosf((time + i * 321) * 0.0025f);
	}

	KCL::uint32 light_counter = m_light_pos_array.size();

	// Lights around the robot
	static const float r = 5.5f;
	for (KCL::uint32 i = 0; i < ROBOT_LIGHTS; i++)
	{
		int t = (time + i * 521) % 3000;
		float alpha = (time + i * 1000) * 0.001f;

		float y = 26.0f / 3000.0f * t;
		float x = r * cos(alpha);
		float z = r * sin(alpha) * 0.7f;

		x += m_actor_pos.x;
		z += m_actor_pos.z;
		y += 0.1f;

		m_animated_pos_array[light_counter++] = KCL::Vector4D(x, y, z, 12.13f);
	}

	for (KCL::uint32 i = 0; i < FRONT_LIGHTS; i++)
	{
		float alpha = time * 0.001f + i * 2.09f;
		float x = 1.5f * cos(alpha) + m_front_lights_pos.x;
		float z = 1.5f * sin(alpha) + m_front_lights_pos.z;
		float y = m_front_lights_pos.y;

		m_animated_pos_array[light_counter++] = KCL::Vector4D(x, y, z, 1.029f);
	}

	if (WasKeyPressed(82)) // R key
	{
		INFO("Reload ALUTest2 resources...\n");
		FreeResources();
		init();
	}

	return time < m_settings->m_play_time;
}


KCL::Vector4D ALUTest2_Base::FitViewportToGBuffer(float viewport_width, float viewport_height, bool &landscape)
{
	const float g_buffer_width = float(G_BUFFER_WIDTH);
	const float g_buffer_height = float(G_BUFFER_HEIGHT);

	landscape = true;
	if (m_window_height > m_window_width && GetSetting().GetScreenMode() == 0 && !GetSetting().m_virtual_resolution)
	{
		// Calculate as it would be "landscape". We will rotate it later
		const float tmp = viewport_width;
		viewport_width = viewport_height;
		viewport_height = tmp;
		landscape = false;
	}

	// Determinate the aspect ratio dependent scale factor
	const float gbuffer_aspect = g_buffer_width / g_buffer_height;
	const float viewport_aspect = viewport_width / viewport_height;
	float scale;
	if (viewport_aspect >= gbuffer_aspect)
	{
		scale = g_buffer_width / viewport_width;
	}
	else
	{
		scale = g_buffer_height / viewport_height;
	}

	// The new size of the image and the offsets from the top-left corner
	const float size_x = viewport_width * scale;
	const float size_y = viewport_height * scale;
	const float offset_x = (float(G_BUFFER_WIDTH) - size_x) / 2.0f;
	const float offset_y = (float(G_BUFFER_HEIGHT) - size_y) / 2.0f;

	// Convert to UV space
	const float u0 = offset_x / g_buffer_width;
	const float v0 = offset_y / g_buffer_height;
	const float u1 = u0 + size_x / g_buffer_width;
	const float v1 = v0 + size_y / g_buffer_height;

	// Scale the camera corners according to the UV scale (corner order: bl, br, tl, tr)
	KCL::Vector3D bl = m_camera.corners[0]; // Bottom left
	KCL::Vector3D right = m_camera.corners[1] - m_camera.corners[0];
	KCL::Vector3D up = m_camera.corners[2] - m_camera.corners[0];

	m_camera.corners[0] = bl + right * u0 + up * v0;
	m_camera.corners[1] = bl + right * u1 + up * v0;
	m_camera.corners[2] = bl + right * u0 + up * v1;
	m_camera.corners[3] = bl + right * u1 + up * v1;

	return KCL::Vector4D(u0, v0, u1, v1);
}


KCL::KCL_Status ALUTest2_Base::LoadScene()
{
	KCL::AssetFile file("alu2/alu2_scene.txt");
	if (file.GetLastError())
	{
		INFO("ALUTest2 - ERROR: lowlevel2/alu2_scene.txt not found!\n");
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	std::stringstream ss(file.GetBuffer());
	std::string note;
	ss >> note;
	ss >> m_camera.view_pos.x;
	ss >> m_camera.view_pos.y;
	ss >> m_camera.view_pos.z;

	ss >> note;
	ss >> m_camera.depth_parameters.x;
	ss >> m_camera.depth_parameters.y;
	ss >> m_camera.depth_parameters.z;
	ss >> m_camera.depth_parameters.w;

	float tmp;
	for (KCL::uint32 i = 0; i < 4; i++)
	{
		ss >> note;
		ss >> m_camera.corners[i].x;
		ss >> m_camera.corners[i].y;
		ss >> m_camera.corners[i].z;
		ss >> tmp;
	}

	ss >> note;
	ss >> m_actor_pos.x;
	ss >> m_actor_pos.y;
	ss >> m_actor_pos.z;

	ss >> note;
	ss >> m_front_lights_pos.x;
	ss >> m_front_lights_pos.y;
	ss >> m_front_lights_pos.z;

	KCL::uint32 color_count;
	ss >> note;
	ss >> color_count;
	std::vector<KCL::Vector3D> colors(color_count);
	for (KCL::uint32 i = 0; i < color_count; i++)
	{
		ss >> colors[i].x;
		ss >> colors[i].y;
		ss >> colors[i].z;
	}

	KCL::uint32 light_count;
	ss >> note;
	ss >> light_count;
	m_light_pos_array.resize(light_count);
	for (KCL::uint32 i = 0; i < light_count; i++)
	{
		ss >> m_light_pos_array[i].x;
		ss >> m_light_pos_array[i].y;
		ss >> m_light_pos_array[i].z;
		ss >> m_light_pos_array[i].w;
	}

	m_light_count = m_light_pos_array.size() + ROBOT_LIGHTS + FRONT_LIGHTS;
	//INFO("ALUTest2 - Using %d lights and %d colors", m_light_count,  m_light_color_array.size());

	m_animated_pos_array.resize(m_light_count);

	// "Full-screen" and "Quater-screen" static lights
	for (KCL::uint32 i = 0; i < 5; i++)
	{
		m_animated_pos_array[i] = m_light_pos_array[i];
	}

	for (KCL::uint32 i = 0; i <m_animated_pos_array.size(); i++)
	{
		m_light_color_array.push_back(colors[i % colors.size()]);
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


void ALUTest2_Base::FreeResources()
{
	m_animated_pos_array.clear();
	m_light_pos_array.clear();
	m_light_color_array.clear();
}

