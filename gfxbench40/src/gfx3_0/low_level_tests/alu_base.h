/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __ALU_TEST_BASE__
#define __ALU_TEST_BASE__

#include "test_base.h"

class ALUTest_Base : public GLB::TestBase
{
private:
	KCL::uint32 m_score;

public:
	ALUTest_Base(const GlobalTestEnvironment * const gte) ;
	virtual ~ALUTest_Base() ;

protected:
	virtual bool animate (const int time);
	virtual float getScore () const { return m_score; }
};


#endif // __ALU_TEST_BASE__