/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "properties.h"
#include "datacollector.h"
#include <iostream>

using namespace std;
using namespace sysinf;

int main()
{
	Properties props;
	props.setString("test/string", "val001");
	props.setDouble("test/sub/double", 0.1234);
	props.setInt("test/sub/integer", 1234);
	props.setLong("test/sub/long", 1234);

	DataCollector dc;
	dc.SetProperties(&props);
	dc.Collect();

	for (PropertyIter it = props.iterator(); !it.done(); it.next())
	{
		cout << "Name: " << it.name()
		     << " Value: " << it.value().getString()
		     << " Type: " << it.value().getType()
		     << endl;
	}

	return 0;
}

