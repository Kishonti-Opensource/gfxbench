/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/base64.h"

namespace base64
{
	decoder::
		decoder(int buffersize_in)
	: _buffersize(buffersize_in)
	{
		base64_init_decodestate(&_state);
	}

	int decoder::
		decode(char value_in)
	{
		const int result = base64_decode_value(value_in);
		base64_init_decodestate(&_state);
		return result;
	}

	int decoder::
		decode(const char* code_in, const int length_in, char* plaintext_out)
	{
		int length = base64_decode_block(code_in, length_in, plaintext_out, &_state);
		base64_init_decodestate(&_state);		
		return length;
	}

	void decoder::
		decode(std::istream& istream_in, std::ostream& ostream_in)
	{
		base64_init_decodestate(&_state);
		//
		const int N = _buffersize;
		char* code = new char[N];
		char* plaintext = new char[N];
		int codelength;
		int plainlength;

		do
		{
			istream_in.read((char*)code, N);
			codelength = (int) istream_in.gcount();
			plainlength = decode(code, codelength, plaintext);
			ostream_in.write((const char*)plaintext, plainlength);
		}
		while (istream_in.good() && codelength > 0);
		//
		base64_init_decodestate(&_state);

		delete [] code;
		delete [] plaintext;
	}
}

