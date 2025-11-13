/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import net.kishonti.benchui.initialization.InitializerApplication.OnApplicationInitializationListener;
import android.app.Fragment;
import android.util.Log;

public class InitializationDependentFragment extends Fragment implements OnApplicationInitializationListener {

	protected boolean mInitialized = false;
	
	public boolean isInitialized() {
		return mInitialized;
	}
	
	@Override
	public void onResume() {
		super.onResume();
		((InitializerApplication)getActivity().getApplication()).initialize(this);
		Log.i("Initialization", "After Init " + this);
	}
	
	@Override
	public void onApplicationInitialized() {
		if(!mInitialized) {
			mInitialized = true;
			Log.i("Initialization", "Initializing with data...");
		}
	}
	
	@Override
	public void onApplicationInitFailed() {
	}
}
