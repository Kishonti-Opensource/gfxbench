/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXBench30_mtl_lensflare_h
#define GFXBench30_mtl_lensflare_h

#include "kcl_light2.h"

#include "mtl_types.h"
#include "mtl_globals.h"


namespace MetalRender
{
    struct OcclusionQueryBuffer
    {
        uint32_t m_frame;
        volatile uint32_t m_queriesRetired;
        id <MTLBuffer> m_queryResults;
        uint32_t m_size;
        
        ~OcclusionQueryBuffer()
        {
            releaseObj(m_queryResults);
        }
    };
	
}

#endif //GFXBench30_mtl_lensflare_h
