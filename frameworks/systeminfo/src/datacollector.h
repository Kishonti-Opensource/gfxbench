/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  datacollector.h
//  GFXBench
//
//  Created by Kishonti Kft on 12/11/2013.
//
//

#ifndef __GFXBench__datacollector__
#define __GFXBench__datacollector__

#include "properties.h"
#include "SystemInfoCommonKeys.h"

namespace sysinf
{
	class DataCollector
	{
	public:
		DataCollector(): m_properties(0) {}
		void Collect(); // must implement
		const Properties* GetProperties() const { return m_properties; }
		void SetProperties(Properties *props) { m_properties = props; }
	
	private:
		Properties *m_properties;
	};
}

#endif /* defined(__GFXBench__datacollector__) */
