/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __GL_VOL__
#define __GL_VOL__

#include "test_base.h"
#include "kcl_math3d.h"

namespace GLB
{
class GLBShader2;
}

namespace KCL
{
	class Camera2;

}


class VolTest : public GLB::TestBase
{
public:
	VolTest(const GlobalTestEnvironment * const gte) ;
	virtual ~VolTest() ;


private:
	struct _particle
	{
		KCL::Vector4D Pos; // particle instance's position
		KCL::Vector4D Velocity;
		KCL::Vector4D T;
		KCL::Vector4D B;
		KCL::Vector4D N;
		KCL::Vector4D Phase;
		KCL::Vector4D Frequency;
		KCL::Vector4D Amplitude;
		KCL::Vector4D Age_Speed_Accel;
	};
	struct _emitter
	{
		KCL::Matrix4x4 pom;
		_particle minp;
		_particle maxp;
		KCL::Vector4D external_velocity;
		KCL::Vector4D aperture;
		KCL::Vector4D Focus_Life_Rate_Gravity;
	};


	KCL::uint32 m_vbo;
	KCL::uint32 m_ebo;
	KCL::uint32 m_vao;
	
	KCL::uint32 particles_vbo;
	KCL::uint32 emitter_ubo;
	KCL::uint32 volume_texture;
	KCL::uint32 volume_texture_size;
	KCL::uint32 noise_cube_texture;

	GLB::GLBShader2 *m_emitter_shader;
	GLB::GLBShader2 *m_render_to_volume_shader;
	GLB::GLBShader2 *m_volume_render_shader;
	KCL::Camera2 *m_camera;
	KCL::uint32 m_score;

	virtual float getScore () const { return m_score; }
	virtual const char* getUom() const { return "frames"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	
	virtual KCL::KCL_Status init ();
	virtual bool animate (const int time);
	virtual bool render ();
	
	virtual void FreeResources();

};

#endif
