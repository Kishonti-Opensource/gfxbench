/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../gfxbench.h"
#include "ng/log.h"

template <class T>
class GFXBenchCorporateA : public GFXBenchA<T>
{
public:
	virtual void run()
	{
		GFXBenchA<T>::run();
		NGLOG_INFO(GFXBenchA<T>::result_nocharts_.c_str());
	}
};
