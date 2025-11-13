/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.app;

import net.kishonti.benchui.DescriptorFactory;
import net.kishonti.benchui.corporate.BenchmarkApplicationCorporate;
import net.kishonti.benchui.corporate.FileDescriptorFactory;
import net.kishonti.benchui.lists.resultlist.FPSResultFormatter;
import net.kishonti.benchui.lists.resultlist.ResultFormatter;

public class App extends BenchmarkApplicationCorporate {

	public App() {
		super();
	}

	@Override
	public void onCreate() {
		super.onCreate();
	}

	@Override
	public ResultFormatter getFormatter() {
		return new FPSResultFormatter();
	}

	@Override
	public DescriptorFactory getDescriptorFactory() {
		return new FileDescriptorFactory(this);
	}

	@Override
	public Boolean isDetailDiagramCompatible() {
		return true;
	}
	
}
