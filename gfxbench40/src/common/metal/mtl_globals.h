/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTLGLOBALS_H
#define MTLGLOBALS_H

#import <Metal/Metal.h>

#if __has_feature(objc_arc)
#define releaseObj(obj) obj = nil
#define releaseObjInBlock(__obj__)
#else // if ! __has_feature(objc_arc)

#error automatic reference counting must be enabled!
//#define releaseObj(obj) [obj release]; obj = nil
//#define releaseObjInBlock(__obj__) [__obj__ release];

#endif // ! __has_feature(objc_arc)

//#define FORCE_DEBUG_SHADERS
#ifdef FORCE_DEBUG_SHADERS
#endif // FORCE_DEBUG_SHADERS

#if !TARGET_OS_EMBEDDED
	#define STORAGE_MODE_MANAGED_OR_SHARED	MTLResourceStorageModeManaged
#else
	#define STORAGE_MODE_MANAGED_OR_SHARED	MTLResourceStorageModeShared
#endif

//#define FORCE_SINGLE_QUAD_MESH_RENDERING_ONLY


#endif // MTLGLOBALS_H
