/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/macros.h"

#include <assert.h>
#include <stdexcept>

namespace ng {
namespace
{
	//ctor of global object (see below) will run at startup to check endianness
	struct EndiannessChecker
	{
		EndiannessChecker()
		{
			static const int c_one = 1;
			bool bLittleEndian = (*(const char*)&c_one == 1);
#ifdef NG_LITTLE_ENDIAN
			assert(bLittleEndian);
			if ( !bLittleEndian )
				throw std::runtime_error("Architecture is big-endian but code compiled as little-endian, fix the endianness macros in ng/macros.h");
#else
			assert(!bLittleEndian);
			if ( bLittleEndian )
				throw std::runtime_error("Architecture is little-endian but code compiled as big-endian, fix the endianness macros in ng/macros.h");
#endif
		}
	};
	
	const EndiannessChecker g_endiannessChecker;


}}
