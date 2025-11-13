/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef PRINTABLE_RESULT_H
#define PRINTABLE_RESULT_H

#include "kcl_base.h"
#include "test_descriptor.h"

#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

class TiXmlElement;

struct PrintableResult
{
	float m_score;
    unsigned int m_num_frames;
	std::string m_fps;
    double m_fps_d;
	std::string m_minfps;
	std::string m_maxfps;
	std::string m_uom;
	std::string m_vsync_triggered;
	bool m_is_warmup;
    std::string m_result_id;
	int m_frame_step_time;
	unsigned int m_viewport_width;
	unsigned int m_viewport_height;
	int m_eglconfig_id;
	int	m_test_length;
	KCL::KCL_Status m_error;
	bool m_is_vsync_triggered;
	std::string m_texture_type;
	//std::string m_error_string;
	std::vector<double> m_frame_times;
	TestDescriptor m_test_descriptor;
    std::vector<std::string> m_extraData;

	PrintableResult();

	PrintableResult(
		const std::string &m_texture_type,
		float score,
		const std::string &error_string,
		float fps,
		const std::string &uom,
		const bool vsync_triggered,
		bool iswarmup,
        const std::string &m_result_id,
		unsigned int m_frame_step_time,
		unsigned int m_viewport_width,
		unsigned int m_viewport_height,
		unsigned int m_num_frames,
		int m_eglconfig_id,
		int	m_test_length,
		KCL::KCL_Status m_error,
        std::vector<std::string> m_extraData
		);
	
	~PrintableResult();

	void SetToDefault();


	void Load( const TiXmlElement *element);
	void Save( const TiXmlElement *root);


	PrintableResult(const PrintableResult &source)
	{
		m_score = source.m_score;
		m_fps = source.m_fps;
        m_fps_d = source.m_fps_d;
        m_num_frames = source.m_num_frames;
		m_uom = source.m_uom;
		m_vsync_triggered = source.m_vsync_triggered;
		m_result_id = source.m_result_id;
		m_frame_step_time = source.m_frame_step_time;
		m_viewport_width = source.m_viewport_width;
		m_viewport_height = source.m_viewport_height;
		m_eglconfig_id = source.m_eglconfig_id;
		m_test_length = source.m_test_length;
		m_error = source.m_error;
		m_is_vsync_triggered = source.m_is_vsync_triggered;
		//m_error_string = source.m_error_string;
		m_frame_times = source.m_frame_times;
		m_texture_type = source.m_texture_type;
		m_test_descriptor = source.m_test_descriptor;
		m_minfps = source.m_minfps;
		m_maxfps = source.m_maxfps;
        m_extraData = source.m_extraData;
	}

	PrintableResult& operator=(const PrintableResult &source)
	{
		if(this != &source)
		{
			m_score = source.m_score;
			m_fps = source.m_fps;
            m_fps_d = source.m_fps_d;
            m_num_frames = source.m_num_frames;
			m_uom = source.m_uom;
			m_vsync_triggered = source.m_vsync_triggered;
			m_result_id = source.m_result_id;
			m_frame_step_time = source.m_frame_step_time;
			m_viewport_width = source.m_viewport_width;
			m_viewport_height = source.m_viewport_height;
			m_eglconfig_id = source.m_eglconfig_id;
			m_test_length = source.m_test_length;
			m_error = source.m_error;
			m_is_vsync_triggered = source.m_is_vsync_triggered;
			//m_error_string = source.m_error_string;
			m_frame_times = source.m_frame_times;
			m_texture_type = source.m_texture_type;
			m_test_descriptor = source.m_test_descriptor;
			m_minfps = source.m_minfps;
			m_maxfps = source.m_maxfps;
            m_extraData = source.m_extraData;
		}
		return *this;
	}

};


struct PrintableResultArray
{
	std::vector<PrintableResult> m_data;
};


#endif //PRINTABLE_RESULT_H
