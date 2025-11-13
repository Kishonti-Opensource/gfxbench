/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __METAL_WRAPPER__
#define __METAL_WRAPPER__

#ifdef USE_METAL
#if defined OPT_TEST_GFX30 || defined OPT_TEST_GFX31 || defined OPT_TEST_GFX40
#include "test_wrapper.h"

#ifdef OPT_TEST_GFX30
#include "metal/mtl_low_level.h"
#endif

#ifdef OPT_TEST_GFX31
#include "metal/mtl_low_level2.h"
#endif

#define METAL_LOW_LEVEL_TESS_ENABLED 0

#if defined(OPT_TEST_GFX40) && METAL_LOW_LEVEL_TESS_ENABLED
#include "metal/mtl_low_level4.h"
#endif

class MetalWrapper : public TestWrapper
{
public:
    MetalWrapper(const GlobalTestEnvironment* const gte) : TestWrapper(gte, NULL)
    {
#ifdef OPT_TEST_GFX30
        if (gte->IsEngine("cpu_overhead_test_metal"))
        {
            if (gte->IsGraphicsContextMetal())
            {
                m_wrappedTest = CreateCPUOverheadTestMetal(gte);
            }
        }
        else if (gte->IsEngine("alu_test_metal"))
        {
            if (gte->IsGraphicsContextMetal())
            {
                m_wrappedTest = CreateALUTestMetal(gte);
            }
        }
        else if (gte->IsEngine("ui_test_metal"))
        {
            if (gte->IsGraphicsContextMetal())
            {
                m_wrappedTest = CreateUITestMetal(gte);
            }
        }
        else if (gte->IsEngine("compressed_fill_test_metal"))
        {
            if (gte->IsGraphicsContextMetal())
            {
                m_wrappedTest = CreateCompressedFillTestMetal(gte);
            }
        }
#endif
#ifdef OPT_TEST_GFX31
        if (gte->IsEngine("driver_overhead_test2_metal"))
        {
            if (gte->IsGraphicsContextMetal())
            {
                m_wrappedTest = CreateDriverOverhead2TestMetal(gte);
            }
        }
        if (gte->IsEngine("alu_test2_metal"))
        {
            if (gte->IsGraphicsContextMetal())
            {
                m_wrappedTest = CreateALU2TestMetal(gte);
            }
        }
        if (gte->IsEngine("compressed_fill_test2_metal"))
        {
            if (gte->IsGraphicsContextMetal())
            {
                m_wrappedTest = CreateCompressedFillTestMetal2(gte);
            }
        }
#endif
#if defined(OPT_TEST_GFX40) && METAL_LOW_LEVEL_TESS_ENABLED
		if (gte->IsEngine("tess_test_metal"))
		{
			if (gte->IsGraphicsContextMetal())
			{
				m_wrappedTest = CreateTessTestMetal(gte);
			}
		}
#endif
    }
    virtual float getScore () const { return m_wrappedTest->getScore(); };
    
    virtual const char* getUom() const { return m_wrappedTest->getUom(); };
    
};
#endif
#endif

#endif // __METAL_WRAPPER__

