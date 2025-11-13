/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/base64.h"

namespace base64
{
	encoder::
		encoder(int buffersize_in)
		: _buffersize(buffersize_in)
	{
        base64_init_encodestate(&_state);
    }

	int encoder::
		encode(char value_in)
	{
		return base64_encode_value(value_in);
	}

	int encoder::
		encode(const char* code_in, const int length_in, char* plaintext_out)
	{
		return base64_encode_block(code_in, length_in, plaintext_out, &_state);
	}

	int encoder::
		encode_end(char* plaintext_out)
	{
		return base64_encode_blockend(plaintext_out, &_state);
	}

	void encoder::
		encode(std::istream& istream_in, std::ostream& ostream_in)
	{
		base64_init_encodestate(&_state);
		//
		const int N = _buffersize;
		char* plaintext = new char[N];
		char* code = new char[2*N];
		int plainlength;
		int codelength;

		do
		{
			istream_in.read(plaintext, N);
			plainlength = (int)istream_in.gcount();
			//
			codelength = encode(plaintext, plainlength, code);
			ostream_in.write(code, codelength);
		}
		while (istream_in.good() && plainlength > 0);

		codelength = encode_end(code);
		ostream_in.write(code, codelength);
		//
		base64_init_encodestate(&_state);

		delete [] code;
		delete [] plaintext;
	}
}
