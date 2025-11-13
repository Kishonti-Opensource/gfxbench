/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __CPU_OVERHEAD_TEST_BASE__
#define __CPU_OVERHEAD_TEST_BASE__

#include "test_base.h"

class CPUOverheadTest_Base : public GLB::TestBase
{
private:

public:
	CPUOverheadTest_Base(const GlobalTestEnvironment * const gte) ;
	virtual ~CPUOverheadTest_Base() ;

protected:
    virtual const char* getUom() const { return "frames"; }
    virtual bool isWarmup() const { return false; }
    virtual KCL::uint32 indexCount() const { return 0; }
    
	int m_score;

	virtual bool animate (const int time);
	virtual float getScore () const { return m_score; }
};

#endif // __CPU_OVERHEAD_TEST_BASE__