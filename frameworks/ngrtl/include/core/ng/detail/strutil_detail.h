/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_strutil_detail_1361139503
#define INCLUDE_GUARD_strutil_detail_1361139503

namespace ng
{
	class itoabuf_t;
	namespace strutil_detail
	{
		template<typename T>
		const char* itoa(T value, itoabuf_t& buf, int base);

		template<typename T>
		substring itoa_ss(T value, itoabuf_t& buf, int base);
	}
}

#endif

