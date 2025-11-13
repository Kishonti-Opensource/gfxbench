/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef OGGDECODER_H
#define OGGDECODER_H

#include <string>

class TheoraDecoder;

typedef unsigned char u8;

class _ogg_decoder
{
public:
	int width;
	int height;

	~_ogg_decoder();
	_ogg_decoder( const char *filename);

	void Play( float time, bool forceRefresh = 0);
	void Decode(void* dst);
	void decode(u8* &yChannel, u8* &uChannel, u8* &vChannel, int &yStride, int &uStride, int &vStride, int &hdec, int &vdec);
    inline const std::string& getName() { return m_name; }

	int m_need_refresh;
protected:
    std::string m_name;
	TheoraDecoder *m_decoder;
};


#endif
