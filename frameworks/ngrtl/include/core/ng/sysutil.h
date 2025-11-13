/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_SYSUTIL_INCLUDED
#define NG_SYSUTIL_INCLUDED

namespace ng { namespace sysutil {

//On non-windows platform this function has no effect
//On Windows it configures Windows Error Reporting dialog box:
//Default: suppress, except when
// - the NG_DONT_SUPPRESS_WER environment variable is nonzero
void configWindowsErrorReporting();

}}

#endif
