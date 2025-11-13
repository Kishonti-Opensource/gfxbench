/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __ALU2_TEST_BASE__
#define __ALU2_TEST_BASE__


#include "test_base.h"
#include "kcl_math3d.h"

class ALUTest2_Base : public GLB::TestBase
{
public:
	ALUTest2_Base(const GlobalTestEnvironment* const gte);
	virtual ~ALUTest2_Base();

protected:
	static const KCL::uint32 G_BUFFER_WIDTH  = 1920;
	static const KCL::uint32 G_BUFFER_HEIGHT = 1080;
	static const KCL::uint32 ROBOT_LIGHTS = 16;
	static const KCL::uint32 FRONT_LIGHTS = 3;

	virtual const char* getUom() const { return "frames"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	virtual float getScore() const { return m_score; }

	virtual bool animate(const int time);

	virtual void FreeResources();

	struct Camera
	{
		KCL::Vector4D view_pos;
		KCL::Vector4D depth_parameters;
		KCL::Vector3D corners[4];
	};
	Camera m_camera;

	KCL::uint32 m_light_count;
	KCL::Vector3D m_actor_pos;
	KCL::Vector3D m_front_lights_pos;

	std::vector<KCL::Vector4D> m_light_pos_array;
	std::vector<KCL::Vector4D> m_animated_pos_array;
	std::vector<KCL::Vector3D> m_light_color_array;

	KCL::int32 m_score;

	KCL::Vector4D FitViewportToGBuffer(float viewport_width, float viewport_height, bool &landscape);
	KCL::KCL_Status LoadScene();
};


#endif // __ALU2_TEST_BASE__

