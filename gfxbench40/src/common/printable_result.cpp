/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "printable_result.h"
#include "xml_utils.h"
#include "offscrman.h"

#include <fstream>


using namespace std;

PrintableResult::PrintableResult()
{
}


PrintableResult::PrintableResult( 
	const std::string &texture_type,
	float score,
	const std::string &error_string,
	float fps,
	const std::string &uom,
	const bool vsync_triggered,

	bool iswarmup,
	const std::string &result_id,
	unsigned int frame_step_time,
	unsigned int viewport_width,
	unsigned int viewport_height,
	unsigned int num_frames,
	int eglconfig_id,
	int	test_length,
	KCL::KCL_Status error,
    std::vector<std::string> extraData
	)
{
	char tmp[1025] = {0};

	m_texture_type = texture_type;
	m_score = score;
	m_is_warmup = iswarmup;
	if(!m_is_warmup)
	{
		sprintf( tmp, "%.1f fps", fps);
	}
    m_num_frames = num_frames;
	m_fps = tmp;
    m_fps_d = fps;
	m_uom = uom;
	sprintf( tmp, "%s", vsync_triggered ? "true" : "false");
	m_vsync_triggered = tmp;
	m_result_id = result_id;
	m_frame_step_time= frame_step_time;
	m_viewport_width = viewport_width;
	m_viewport_height = viewport_height;
	m_eglconfig_id = eglconfig_id;
	m_test_length = test_length;
	m_error = error;
	//m_error_string = error_string;
    m_extraData = extraData;
}


PrintableResult::~PrintableResult()
{
}


void PrintableResult::SetToDefault()
{
	m_score = 0.0f;
	m_fps = "";
    m_num_frames = 0;
	m_uom = "";
	m_vsync_triggered = "";
	m_texture_type.clear();

	m_error = KCL::KCL_TESTERROR_NOERROR;
	m_eglconfig_id = -1;
	m_test_length = -1;
	m_frame_step_time = 0;
	m_viewport_width = 0;
	m_viewport_height = 0;
	m_frame_times.clear();
	m_is_vsync_triggered = false;
	m_test_descriptor.SetDefaults();
    m_extraData.clear();
    
    m_result_id = m_test_descriptor.m_name;
    
}


void PrintableResult::Load( const TiXmlElement *element)
{
	std::string v = element->Value();
	const char* t = element->GetText();

	if( t)
	{
		if( v == "texture_type")
		{
			m_texture_type = t;
		}
		else if( v == "score")
		{
			m_score = atof( t);
		}
		else if( v == "fps")
		{
			m_fps = t;
		}
		else if( v == "uom")
		{
			m_uom = t;
		}
		else if( v == "vsync_triggered")
		{
			m_vsync_triggered = t;
		}
		else if( v == "error")
		{
			m_error = (KCL::KCL_Status)atoi(t);
		}
//		else if( v == "error_string")
//		{
//			m_error_string = t;
//		}
		else if( "result_id" == v )
		{
			m_result_id = t;
		}
	}
}


void PrintableResult::Save( const TiXmlElement *root)
{
	TiXmlElement *elem0;

	elem0 = addelem( (TiXmlElement *)root, "test_result", "");

	addelem( elem0, "texture_type", m_texture_type.c_str());
	addelem( elem0, "score", m_score);
	addelem( elem0, "fps", m_fps.c_str());
	addelem( elem0, "minfps", m_minfps.c_str());
	addelem( elem0, "maxfps", m_maxfps.c_str());
	addelem( elem0, "uom", m_uom.c_str());
	addelem( elem0, "vsync_triggered", m_vsync_triggered.c_str());
	addelem( elem0, "error", m_error);
	//addelem( elem0, "error_string", m_error_string.c_str());
	addelem( elem0, "result_id", m_result_id.c_str() );
}
