/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_OGGDECODER_H
#define GLB_OGGDECODER_H

#include "ogg_decoder.h"
#include <string>

class GLB_ogg_decoder : public _ogg_decoder
{
public:
	unsigned int m_tbo;

	std::string m_filename; //for serialization

	GLB_ogg_decoder( const char *filename);
	~GLB_ogg_decoder();

	bool DecodeDirect( float time);
	bool DecodePBO( float time);
private:
	unsigned int m_pbos[2];
	float m_pbotimes[2];
	bool m_actual_pbo;
	unsigned char *m_data;
};


#endif
