/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include "kcl_base.h"

class Result
{
private:
	unsigned int m_test_id;
	std::string m_test_title;
	std::string m_test_type;
	std::string m_texture_type;
	std::string m_uom;
	float m_test_length;
	float m_frame_step_time;

	KCL::KCL_Status m_error;
	float m_score;
	float m_fps;
	float m_minfps;
	float m_maxfps;

	bool m_is_vsync_triggered;
	bool m_is_warmup;
	unsigned int m_viewport_width;
	unsigned int m_viewport_height;
	
	bool m_is_uploaded;

public:
	unsigned int GetTestId() const;
	std::string GetTestTitle() const;
	std::string GetTestType() const;
	std::string GetTextureType() const;
	std::string GetUom() const;
	float GetTestLength() const;
	float GetFrameStepTime() const;
	KCL::KCL_Status GetError() const;
	float GetScore() const;
	float GetFps() const;
	float GetMinFps() const;
	float GetMaxFps() const;
	bool IsVsyncTriggered() const;
	bool IsWarmup() const;
	unsigned int GetViewportWidth() const;
	unsigned int GetViewportHeight() const;
	bool IsUploaded() const;
	
	void SetTestId( unsigned int value );
	void SetTestTitle( std::string value );
	void SetTestType( std::string value );
	void SetTextureType( std::string value );
	void SetUom( std::string value );
	void SetTestLength( float value );
	void SetFrameStepTime( float value );
	void SetError( KCL::KCL_Status value );
	void SetScore( float value );
	void SetFps( float value );
	void SetMinFps( float value );
	void SetMaxFps( float value );
	void SetVsyncTriggered( bool value );
	void SetWarmup( bool value );
	void SetViewportWidth( unsigned int value );
	void SetViewportHeight( unsigned int value );
	void SetUploaded( bool value );

	Result(
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
			bool is_uploaded = false
		);

};

typedef std::vector<Result> ResultsContainer;

#endif //RESULT_H
