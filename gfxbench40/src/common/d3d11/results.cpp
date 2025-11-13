/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "results.h"

unsigned int Result::GetTestId() const
{
	return m_test_id;
}

std::string Result::GetTestTitle() const
{
	return m_test_title;
}

std::string Result::GetTestType() const
{
	return m_test_type;
}

std::string Result::GetTextureType() const
{
	return m_texture_type;
}

std::string Result::GetUom() const
{
	return m_uom;
}

float Result::GetTestLength() const
{
	return m_test_length;
}

float Result::GetFrameStepTime() const
{
	return m_frame_step_time;
}

KCL::KCL_Status Result::GetError() const
{
	return m_error;
}

float Result::GetScore() const
{
	return m_score;
}

float Result::GetFps() const
{
	return m_fps;
}

float Result::GetMinFps() const
{
	return m_fps;
}

float Result::GetMaxFps() const
{
	return m_maxfps;
}

bool Result::IsVsyncTriggered() const
{
	return m_is_vsync_triggered;
}

bool Result::IsWarmup() const
{
	return m_is_warmup;
}

unsigned int Result::GetViewportWidth() const
{
	return m_viewport_width;
}

unsigned int Result::GetViewportHeight() const
{
	return m_viewport_height;
}

bool Result::IsUploaded() const
{
	return m_is_uploaded;
}

	
void Result::SetTestId( unsigned int value )
{
	m_test_id = value;
}

void Result::SetTestTitle( std::string value )
{
	m_test_title = value;
}

void Result::SetTestType( std::string value )
{
	m_test_type = value;
}

void Result::SetTextureType( std::string value )
{
	m_texture_type = value;
}

void Result::SetUom( std::string value )
{
	m_uom = value;
}

void Result::SetTestLength( float value )
{
	m_test_length = value;
}

void Result::SetFrameStepTime( float value )
{
	m_frame_step_time = value;
}

void Result::SetError( KCL::KCL_Status value )
{
	m_error = value;
}

void Result::SetScore( float value )
{
	m_score = value;
}

void Result::SetFps( float value )
{
	m_fps = value;
}

void Result::SetMinFps( float value )
{
	m_minfps = value;
}

void Result::SetMaxFps( float value )
{
	m_maxfps = value;
}

void Result::SetVsyncTriggered( bool value )
{
	m_is_vsync_triggered = value;
}

void Result::SetWarmup( bool value )
{
	m_is_warmup = value;
}

void Result::SetViewportWidth( unsigned int value )
{
	m_viewport_width = value;
}

void Result::SetViewportHeight( unsigned int value )
{
	m_viewport_height = value;
}

void Result::SetUploaded( bool value )
{
	m_is_uploaded = value;
}


Result::Result(
		unsigned int test_id,
		const std::string &test_title,
		const std::string &test_type,
		const std::string &texture_type,
		const std::string &uom,
		float test_length,
		float frame_step_time,
		KCL::KCL_Status error,
		float score,
		float fps,
		float minfps,
		float maxfps,
		bool is_vsync_triggered,
		bool is_warmup,
		unsigned int viewport_width,
		unsigned int viewport_height,
		bool is_uploaded
	)
:
m_test_id(test_id),
m_test_title(test_title),
m_test_type(test_type),
m_texture_type(texture_type),
m_uom(uom),
m_test_length(test_length),
m_frame_step_time(frame_step_time),
m_error(error),
m_score(score),
m_fps(fps),
m_minfps(minfps),
m_maxfps(maxfps),
m_is_vsync_triggered(is_vsync_triggered),
m_is_warmup(is_warmup),
m_viewport_width(viewport_width),
m_viewport_height(viewport_height),
m_is_uploaded(is_uploaded)
{
}
