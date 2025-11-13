/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <cassert>
#include <cmath>
#include <map>

#include "kcl_io.h"
#include "kcl_math3d.h"

extern "C"
{
#include <ogg/ogg.h>
#include <theora/theora.h>
#include <theora/theoradec.h>
}

#include "ogg_decoder.h"


inline int OC_CLAMP255( int x)
{
	if( x > 255) 
		return 255;
	if( x < 0)
		return 0;
	return x;
}


class TheoraDecoder
{
public:
	TheoraDecoder( const char *filename);
	~TheoraDecoder();

	int play( float time, bool forceRefresh);

	void decode( unsigned char *picture);

	bool decode(u8* &yChannel, u8* &uChannel, u8* &vChannel, int &yStride, int &uStride, int &vStride, int &hdec, int &vdec);

	int width()
	{
		return m_tinfo.frame_width;
	}
	int height()
	{
		return m_tinfo.frame_height;
	}
private:
	int m_tpackets;

	ogg_stream_state m_tstream;
	ogg_packet m_packet;
	ogg_sync_state m_sync;
	ogg_page m_page;

	th_info m_tinfo;
	th_comment m_tcomment;
	th_dec_ctx *m_tdec;
	th_setup_info *m_tsetup;
	th_ycbcr_buffer m_buffer;

	float m_video_time;
	float m_full_video_time;
	float m_last_video_time;
	bool m_have_frame;

	KCL::AssetFile *m_io;

	void queue_ogg_page();

	int read_ogg_buff( ogg_sync_state *sync);
};


void TheoraDecoder::queue_ogg_page()
{
	if (m_tpackets) 
	{
		ogg_stream_pagein(&m_tstream, &m_page);
	}
}



int TheoraDecoder::read_ogg_buff( ogg_sync_state *sync)
{
	long buflen = 4096;
	char *buffer = ogg_sync_buffer(sync, buflen);
	if (buffer == NULL)
		return -1;

	buflen = static_cast<long>(m_io->Read( buffer, 1, buflen));
	if (buflen <= 0)
		return 0;

	return (ogg_sync_wrote(sync, buflen) == 0) ? 1 : -1;
}


bool TheoraDecoder::decode(u8* &yChannel, u8* &uChannel, u8* &vChannel, int &yStride, int &uStride, int &vStride, int &hdec, int &vdec)
{
	if (th_decode_ycbcr_out(m_tdec, m_buffer) == 0) {
		yChannel = m_buffer[0].data;
		uChannel = m_buffer[1].data;
		vChannel = m_buffer[2].data;

		yStride = m_buffer[0].stride;
		uStride = m_buffer[1].stride;
		vStride = m_buffer[2].stride;

		hdec = !(m_tinfo.pixel_fmt&1);
		vdec = !(m_tinfo.pixel_fmt&2);

		return true;
	}

	return false;
}


void TheoraDecoder::decode( unsigned char *picture)
{
	unsigned char   *y_row;
	unsigned char   *u_row;
	unsigned char   *v_row;
	unsigned char   *rgb_row;
	int              cstride;
	int              x;
	int              y;
	int              w;
	int              h;
	int              hdec;
	int              vdec;

	if (th_decode_ycbcr_out(m_tdec, m_buffer) == 0)
	{
		hdec=!(m_tinfo.pixel_fmt&1);
		vdec=!(m_tinfo.pixel_fmt&2);


		w = m_buffer[0].width;
		h = m_buffer[0].height;

		y_row = m_buffer[0].data;
		u_row = m_buffer[1].data;
		v_row = m_buffer[2].data;

		rgb_row = picture;

		cstride = w * 4;

		for(y=0;y<h;y++)
		{
			for(x=0;x<w;x++)
			{
				int r;
				int g;
				int b;
				r = (1904000 * y_row[x] + 2609823 * v_row[x>>hdec] - 363703744) / 1635200;
				g = (3827562 * y_row[x] - 1287801 * u_row[x>>hdec] - 2672387 * v_row[x>>hdec] + 447306710) / 3287200;
				b = (952000  * y_row[x] + 1649289 * u_row[x>>hdec] - 225932192) / 817600;
				rgb_row[4*x+0]=OC_CLAMP255(r);
				rgb_row[4*x+1]=OC_CLAMP255(g);
				rgb_row[4*x+2]=OC_CLAMP255(b);
				rgb_row[4*x+3]=255;
			}

			int y_stride = m_buffer[0].stride;
			int u_stride = m_buffer[1].stride&-((y&1)|!vdec);
			int v_stride = m_buffer[2].stride&-((y&1)|!vdec);

			y_row += y_stride;
			u_row += u_stride;
			v_row += v_stride;

			rgb_row += cstride;
		}
	}
}



TheoraDecoder::TheoraDecoder( const char *filename) : 
	m_tpackets( 0), 
	m_tdec(NULL), 
	m_tsetup(NULL), 
	m_video_time(-1), 
	m_full_video_time(-1), 
	m_last_video_time(0),
	m_have_frame(false)
{
	m_io = new KCL::AssetFile( filename);

	if( m_io->GetLastError())
	{
		INFO("!!!error: no video found - %s\n", filename);
		return;
	}

	ogg_sync_init(&m_sync);

	th_comment_init(&m_tcomment);
	th_info_init(&m_tinfo);

	int bos = 1;
	while (bos)
	{
		if (read_ogg_buff( &m_sync) <= 0)
		{
			return;
		}

		while ( (ogg_sync_pageout(&m_sync, &m_page) > 0) )
		{
			ogg_stream_state test;

			if (!ogg_page_bos(&m_page))
			{
				queue_ogg_page();
				bos = 0;
				break;
			}

			ogg_stream_init(&test, ogg_page_serialno(&m_page));
			ogg_stream_pagein(&test, &m_page);
			ogg_stream_packetout(&test, &m_packet);

			if (!m_tpackets && (th_decode_headerin(&m_tinfo, &m_tcomment, &m_tsetup, &m_packet) >= 0))
			{
				memcpy(&m_tstream, &test, sizeof (test));
				m_tpackets = 1;
			}
			else
			{
				ogg_stream_clear(&test);
			}
		}
	}

	if ((!m_tpackets))
	{
		return;
	}

	while (((m_tpackets && (m_tpackets < 3)) ))
	{
		while (m_tpackets && (m_tpackets < 3))
		{
			if (ogg_stream_packetout(&m_tstream, &m_packet) != 1)
			{
				break;
			}
			if (!th_decode_headerin(&m_tinfo, &m_tcomment, &m_tsetup, &m_packet))
			{
				return;
			}
			m_tpackets++;
		}

		if (ogg_sync_pageout(&m_sync, &m_page) > 0)
		{
			queue_ogg_page();
		}
		else if (read_ogg_buff( &m_sync) <= 0)
		{
			return;
		}
	}

	if (m_tpackets)
	{
		if ((m_tinfo.frame_width > 99999) || (m_tinfo.frame_height > 99999))
		{
			return;
		}

		if ( (m_tinfo.colorspace != TH_CS_UNSPECIFIED) && (m_tinfo.colorspace != TH_CS_ITU_REC_470M) && ((m_tinfo.colorspace != TH_CS_ITU_REC_470BG) ))
		{
			return;
		}

		if (m_tinfo.pixel_fmt != TH_PF_420) 
		{ 
			return;
		}

		m_tdec = th_decode_alloc(&m_tinfo, m_tsetup);
		if (!m_tdec)
		{
			return;
		}

		int pp_level_max = 0;
		th_decode_ctl(m_tdec, TH_DECCTL_SET_PPLEVEL, &pp_level_max, sizeof(pp_level_max));
	}

	if (m_tsetup != NULL)
	{
		th_setup_free(m_tsetup);
		m_tsetup = NULL;
	}
}


int TheoraDecoder::play( float time, bool forceRefresh)
{
	//bool need_refresh = false;
	bool need_refresh = forceRefresh;

	if (m_tpackets)
	{
		if (m_full_video_time!=-1)
			while (time > m_full_video_time) time -= m_full_video_time;

		// video_time<=time always. If not...
		if (m_video_time > time)
		{
			ogg_int64_t i = 0;
			th_decode_ctl(m_tdec,TH_DECCTL_SET_GRANPOS,&i,sizeof(ogg_int64_t));
			m_video_time = 0;
			m_io->Seek(0,SEEK_SET);
		}
		while( (time - m_video_time) > 0.5f)
		{
			while( 1)
			{
				if (ogg_stream_packetout(&m_tstream, &m_packet) > 0)
				{
					break;
				}

				const int ret = read_ogg_buff( &m_sync);
				if (ret == 0)
				{
					m_full_video_time = m_last_video_time;
					m_io->Seek( 0, SEEK_SET);
					ogg_int64_t i = 0;
					th_decode_ctl(m_tdec,TH_DECCTL_SET_GRANPOS,&i,sizeof(ogg_int64_t));
					m_video_time = 0;
					while (time > m_full_video_time) time -= m_full_video_time;
				}
				else if (ret < 0)
				{
					return m_have_frame?0:-1;
				}
				while ((ogg_sync_pageout(&m_sync, &m_page) > 0))
				{
					queue_ogg_page();
				}
			}

			ogg_int64_t granulepos = 0;

			const int ret = th_decode_packetin(m_tdec, &m_packet, &granulepos);

			if (ret == 0)
			{
				m_have_frame=true;
				need_refresh = true;
			}

			m_video_time = th_granule_time(m_tdec, granulepos);
			m_last_video_time = m_video_time;
		}	
	}

	return need_refresh ? 1 : (m_have_frame ? 0 : -1);
}


TheoraDecoder::~TheoraDecoder()
{
	if (m_tdec)
	{
		th_decode_free(m_tdec);
	}
	if (m_tsetup) 
	{
		th_setup_free(m_tsetup);
	}
	if (m_tpackets)
	{
		ogg_stream_clear(&m_tstream);
	}

	th_info_clear(&m_tinfo);
	th_comment_clear(&m_tcomment);

	ogg_sync_clear(&m_sync);

	delete m_io;
}


_ogg_decoder::_ogg_decoder(const char *filename): m_need_refresh( 1)
{
	m_decoder = new TheoraDecoder( filename);

	width = m_decoder->width();
	height = m_decoder->height();

    m_name = std::string(filename);
}


_ogg_decoder::~_ogg_decoder()
{
	delete m_decoder;
}


void _ogg_decoder::Play( float time, bool forceRefresh)
{
	m_need_refresh = m_decoder->play( time, forceRefresh);
}


void _ogg_decoder::Decode(void *data)
{ 
	unsigned char *dataptr = (unsigned char*)data;
	m_decoder->decode(dataptr);
}


void _ogg_decoder::decode(u8* &yChannel, u8* &uChannel, u8* &vChannel, int &yStride, int &uStride, int &vStride, int &hdec, int &vdec)
{
	m_decoder->decode(yChannel, uChannel, vChannel, yStride, uStride, vStride, hdec, vdec);
}