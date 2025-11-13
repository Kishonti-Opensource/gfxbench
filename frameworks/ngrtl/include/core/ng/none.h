/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_NONE
#define NG_NONE

namespace ng
{


namespace detail
{
	struct none_helper
	{};
}

typedef int detail::none_helper::*none_t;

none_t const none = (static_cast<none_t>(0));

}

#endif

