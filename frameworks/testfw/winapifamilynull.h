/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef WINAPIFAMILYFIX_H
#define WINAPIFAMILYFIX_H

// for older SDKs which does not have WINAPI_FAMILY_PARTITION defined

#if !defined WINAPI_FAMILY_PARTITION
#define WINAPI_PARTITION_DESKTOP 100
#define WINAPI_FAMILY_PARTITION(x) ((x) == WINAPI_PARTITION_DESKTOP)
#endif

#endif