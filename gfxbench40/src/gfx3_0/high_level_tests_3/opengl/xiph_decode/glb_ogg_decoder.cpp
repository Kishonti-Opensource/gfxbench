/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_ogg_decoder.h"
#include "platform.h"
#include <kcl_math3d.h>


GLB_ogg_decoder::GLB_ogg_decoder(const char *filename) : _ogg_decoder( filename), m_filename(filename), m_actual_pbo( false), m_data( 0)
{
#if defined HAVE_GLES3 || defined __glew_h__
	glGenTextures( 1, &m_tbo);
	glBindTexture( GL_TEXTURE_2D, m_tbo);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexStorage2D( GL_TEXTURE_2D, KCL::texture_levels(width, height), GL_RGBA8, width, height);

	glBindTexture( GL_TEXTURE_2D, 0);

	int data_size = width * height * 4;
	m_data = new unsigned char[data_size];
	memset( m_data, 0, data_size);

	glGenBuffers( 2, m_pbos);

	for( KCL::uint32 i=0; i<2; i++)
	{
		m_pbotimes[i]=-1;

		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_pbos[i]);
		glBufferData( GL_PIXEL_UNPACK_BUFFER, data_size, m_data, GL_STREAM_DRAW);
	}

	glBindTexture( GL_TEXTURE_2D, m_tbo);
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glGenerateMipmap( GL_TEXTURE_2D);

	glBindTexture( GL_TEXTURE_2D, 0);
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0);

	delete [] m_data;
	m_data = 0;
#endif
}


GLB_ogg_decoder::~GLB_ogg_decoder()
{
	glDeleteBuffers( 2, m_pbos);
	glDeleteTextures( 1, &m_tbo);
	delete [] m_data;
}


bool GLB_ogg_decoder::DecodeDirect( float time)
{
	if( m_need_refresh > 0)
	{
		if( !m_data)
		{
			m_data = new unsigned char[width * height * 4];
		}

		Decode( m_data);

		glBindTexture( GL_TEXTURE_2D, m_tbo);
		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
		glGenerateMipmap( GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, 0);
	}

	return true;
}


bool GLB_ogg_decoder::DecodePBO( float time)
{
#if defined HAVE_GLES3 || defined __glew_h__

#ifdef ENABLE_FRAME_CAPTURE
#pragma message("Framecapture: DecodePBO m_need_refresh")
	{
		m_need_refresh = 1;
	}
#endif


	if( m_need_refresh > 0 || ((m_need_refresh == 0) && (m_pbotimes[m_actual_pbo]!=time)) )
	{
		glBindTexture( GL_TEXTURE_2D, m_tbo);
		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_pbos[m_actual_pbo]);

#ifdef ENABLE_FRAME_CAPTURE
#pragma message("Framecapture: DecodePBO uses glBufferData")
		{
			unsigned char *pbo_data = new unsigned char[width * height * 4];
			Decode( pbo_data);

			glBufferData( GL_PIXEL_UNPACK_BUFFER, width * height * 4, pbo_data, GL_STREAM_DRAW);

			delete [] pbo_data;
		}
#endif


		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glGenerateMipmap( GL_TEXTURE_2D);
		glBindTexture( GL_TEXTURE_2D, 0);


		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, m_pbos[!m_actual_pbo]);
		unsigned char *pbo_data = (unsigned char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, width * height * 4, GL_MAP_WRITE_BIT);

		Decode( pbo_data);

		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); 
		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0);

		m_pbotimes[m_actual_pbo]=time;
		m_actual_pbo = !m_actual_pbo;
	}
	return true;
#endif
    return false;
}
