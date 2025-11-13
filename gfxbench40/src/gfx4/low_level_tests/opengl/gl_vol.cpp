/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_vol.h"
#include "opengl/glb_shader2.h"
#include "kcl_io.h"
#include "kcl_camera2.h"
#include "platform.h"
#include "opengl/ext.h"
#include "opengl/glb_discard_functions.h"
#include "kcl_image.h"


static int seed = 0;

static float rand_unsigned_clamped()
{
	return KCL::Math::randomf( &seed);
}
static float rand_signed_clamped()
{
	return KCL::Math::randomf_signed( &seed);
}

inline float lrp (float i, float a, float b)
{
	return ( a * (1.0f - i) + b * i);
}


VolTest::VolTest(const GlobalTestEnvironment * const gte) : TestBase(gte), m_score(0), m_vbo( 0), m_ebo( 0), m_vao( 0), particles_vbo( 0), emitter_ubo( 0), volume_texture( 0), noise_cube_texture( 0)
{
	m_camera = new KCL::Camera2;

	m_camera->Perspective( 45, gte->GetTestDescriptor()->m_viewport_width, gte->GetTestDescriptor()->m_viewport_height, 0.1f, 100.0f);
}


VolTest::~VolTest()
{
	delete m_camera;
}


bool VolTest::animate(const int time)
{
	SetAnimationTime(time);

	float f = m_time * 0.001f ;
	float ff = KCL::Math::Rad( f);
	const float d = 2.2f;

	KCL::Vector3D eye( d * sinf( f), d * 0.5f, d * cosf( f));
	KCL::Vector3D ref( 0.0f, 0.0f, 0.0f);
	KCL::Vector3D up( 0.0f, 1.0f, 0.0f);

	m_camera->LookAt( eye, ref, up);

	m_camera->Update();

	if (m_frames > m_score)
	{
		m_score = m_frames;
	}

	return time < m_settings->m_play_time;
}


KCL::KCL_Status VolTest::init ()
{
	_particle *p = new _particle[1024];
	_emitter e;

	e.pom.identity();
	e.external_velocity.set( 0.01f, 0.0f, 0.0f, 0.0f);
	e.aperture.set( 0.2f, 0.2f, 0.0f, 0.0f);

	for( KCL::uint32 i=0; i<1024; i++)
	{
		e.Focus_Life_Rate_Gravity.set( -2.0f, 2.5f, 0.0f, 0.005f);

		p[i].Pos.set( 0,0,0,0);
		p[i].Phase.set( rand_unsigned_clamped(), rand_unsigned_clamped(), rand_unsigned_clamped(), 0);
		p[i].Frequency.set( 
			lrp( rand_unsigned_clamped(), 0.5, 1.0),
			lrp( rand_unsigned_clamped(), 0.5, 1.0),
			0.0, 0
			);
		p[i].Amplitude.set(
			lrp( rand_unsigned_clamped(), 0.0005f, 0.002f),
			lrp( rand_unsigned_clamped(), 0.0005f, 0.002f),
			0.0, 0
			);

		p[i].Age_Speed_Accel.set(
			rand_unsigned_clamped() * e.Focus_Life_Rate_Gravity.y, 
			lrp( rand_unsigned_clamped(), 2*0.00125f, 2*0.0025f),
			lrp( rand_unsigned_clamped(), 0.005f, 0.008f), 0
			);

		KCL::Vector3D side;
		KCL::Vector3D focus_offset;

		focus_offset.set( 0, e.Focus_Life_Rate_Gravity.x, 0);
		side.set( 1,0,0);

		KCL::Vector3D t;
		KCL::Vector3D b;
		KCL::Vector3D n;

		if( e.Focus_Life_Rate_Gravity.x > 0)
		{
			n.x = focus_offset.x - p[i].Pos.x;
			n.y = focus_offset.y - p[i].Pos.y;
			n.z = focus_offset.z - p[i].Pos.z;
		}
		else
		{
			n.x = p[i].Pos.x - focus_offset.x;
			n.y = p[i].Pos.y - focus_offset.y;
			n.z = p[i].Pos.z - focus_offset.z;
		}

		n.normalize();

		b = KCL::Vector3D::cross( n, side);
		b.normalize();

		t = KCL::Vector3D::cross( n, b);
		t.normalize();

		p[i].T.set( t.x, t.y, t.z , 0);
		p[i].B.set( b.x, b.y, b.z , 0);
		p[i].N.set( n.x, n.y, n.z , 0);
	}

	glGenBuffers( 1, &particles_vbo);
	glBindBuffer( GL_ARRAY_BUFFER, particles_vbo);
	glBufferData( GL_ARRAY_BUFFER, 1024 * sizeof(_particle), p, GL_STATIC_DRAW);
	glBindBuffer( GL_ARRAY_BUFFER, 0);

	delete [] p;

	glGenBuffers( 1, &emitter_ubo);
	glBindBuffer( GL_ARRAY_BUFFER, emitter_ubo);
	glBufferData( GL_ARRAY_BUFFER, sizeof(_emitter), &e, GL_STATIC_DRAW);
	glBindBuffer( GL_ARRAY_BUFFER, 0);

	{
		KCL::Image2D *img = new KCL::Image2D;
	
		img->load( "noise.png");

		glGenTextures( 1, &noise_cube_texture);
		glBindTexture( GL_TEXTURE_CUBE_MAP, noise_cube_texture);
		glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for( int i=0; i<6; i++)
		{
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, img->getBpp() == 32 ? GL_RGBA8 : GL_RGB8, img->getWidth(), img->getHeight(), 0, img->getBpp() == 32 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, img->data());
		}
		
		glBindTexture( GL_TEXTURE_CUBE_MAP, 0);

		delete img;
	}


	volume_texture_size = 32;
	
	glGenTextures( 1, &volume_texture);
	glBindTexture( GL_TEXTURE_3D, volume_texture);
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	KCL::uint32 size = volume_texture_size*volume_texture_size*volume_texture_size;

    glTexStorage3D( GL_TEXTURE_3D, 1, GL_RGBA8, volume_texture_size, volume_texture_size, volume_texture_size);
	glBindTexture( GL_TEXTURE_3D, 0);

	GLB::GLBShader2::InitShaders((KCL::SceneVersion)KCL::SV_40, GetSetting().m_force_highp);

	GLB::GLBShaderBuilder sb;
	KCL::KCL_Status error;

	m_emitter_shader = sb.ShaderFile( "emitter_simulate.shader").Build(error);
	m_render_to_volume_shader = sb.ShaderFile( "render_to_volume.shader").Build(error);
	m_volume_render_shader = sb.ShaderFile( "volume_render.shader").Build(error);

	std::vector<KCL::Vector3D> vertices;
	std::vector<KCL::uint8> indices;

	{
		KCL::AABB aabb;

		aabb.Set( KCL::Vector3D( 0,0,0), KCL::Vector3D( 1,1,1));

		vertices.resize( 8);
		indices.resize( 36);

		aabb.ConvertToBox( &vertices[0], &indices[0]);
	}

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(KCL::Vector3D) * vertices.size(), vertices[0].v, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * indices.size(), &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	glGenVertexArrays( 1, &m_vao);

	glBindVertexArray( m_vao);
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ebo);

	glEnableVertexAttribArray( 0);
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray( 0);

	glBindVertexArray( 0);
	glBindBuffer( GL_ARRAY_BUFFER, 0);
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);

	return KCL::KCL_TESTERROR_NOERROR;
}


bool VolTest::render ()
{
	GLB::GLBShader2 *s;
	glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	s = m_emitter_shader;

	glUseProgram( s->m_p);
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, particles_vbo);
	glBindBufferBase( GL_UNIFORM_BUFFER, 1, emitter_ubo);
	glDispatchComputeProc( 1, 1, 1);
	glMemoryBarrierProc( GL_BUFFER_UPDATE_BARRIER_BIT);
	
	s = m_render_to_volume_shader;

	glUseProgram( s->m_p);
	glActiveTexture( GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_CUBE_MAP, noise_cube_texture);
	glUniform1i( s->m_uniform_locations[GLB::uniforms::envmap0], 0);
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, particles_vbo);
	glBindImageTextureProc( 0, volume_texture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glDispatchComputeProc( volume_texture_size / 16, volume_texture_size / 8, 1);
	glMemoryBarrierProc( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	s = m_volume_render_shader;

	glUseProgram( s->m_p);

	glEnable(GL_DEPTH_TEST);
	glEnable( GL_BLEND);
	glEnable( GL_CULL_FACE);
	glBlendFunc( 1, GL_SRC_ALPHA);

	glActiveTexture( GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_3D, volume_texture);
	glUniform1i( s->m_uniform_locations[GLB::uniforms::texture_3D_unit0], 0);
	glUniformMatrix4fv( s->m_uniform_locations[GLB::uniforms::mvp], 1, 0, m_camera->GetViewProjection().v);
	glUniform3fv( s->m_uniform_locations[GLB::uniforms::view_pos], 1, m_camera->GetEye().v);
	glBindVertexArray( m_vao);
	glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
	glBindVertexArray( 0);

	glDisable( GL_CULL_FACE);
	glDisable( GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	return true;
}


void VolTest::FreeResources()
{
	if( m_vbo)
	{
		glDeleteBuffers( 1, &m_vbo);
	}
	if( m_ebo)
	{
		glDeleteBuffers( 1, &m_ebo);
	}
	if( m_vao)
	{
		glDeleteVertexArrays( 1, &m_vao);
	}
	if( particles_vbo)
	{
		glDeleteBuffers( 1, &particles_vbo);
	}
	if( m_vbo)
	{
		glDeleteBuffers( 1, &m_vbo);
	}
	if( emitter_ubo)
	{
		glDeleteBuffers( 1, &emitter_ubo);
	}
	if( volume_texture)
	{
		glDeleteTextures( 1, &volume_texture);
	}
	if( noise_cube_texture)
	{
		glDeleteTextures( 1, &noise_cube_texture);
	}
}
