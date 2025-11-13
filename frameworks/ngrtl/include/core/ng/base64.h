/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// :mode=c++:
/*
decode.h - c++ wrapper for a base64 decoding algorithm

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64

Files refactored 20120719 tkenez
*/

#ifndef NG_BASE64_INCLUDED
#define NG_BASE64_INCLUDED

#include "ng/ngrtl_core_export.h"
#include <iostream>

extern "C"
{
	#include "ng/base64/cencode.h"
	#include "ng/base64/cdecode.h"
}

namespace base64
{
	struct NGRTL_EXPORT decoder
	{
		base64_decodestate _state;
		int _buffersize;

		decoder(int buffersize_in);
		int decode(char value_in);
		int decode(const char* code_in, const int length_in, char* plaintext_out);
		void decode(std::istream& istream_in, std::ostream& ostream_in);
	};

	struct NGRTL_EXPORT encoder
	{
		base64_encodestate _state;
		int _buffersize;

		encoder(int buffersize_in = 4096);
		int encode(char value_in);
		int encode(const char* code_in, const int length_in, char* plaintext_out);
		int encode_end(char* plaintext_out);
		void encode(std::istream& istream_in, std::ostream& ostream_in);
	};

} // namespace base64

#endif // BASE64_ENCODE_H

