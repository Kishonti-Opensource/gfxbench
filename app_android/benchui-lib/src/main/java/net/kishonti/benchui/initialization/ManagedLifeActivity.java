/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;

/**
 * Activity that logs its lifecycle methods and asks the app for initialization
 */
public class ManagedLifeActivity extends Activity {
	
	private final boolean mLogLifecycle = true;
	private final String TAG_LIFECYCLE = "Lifecycle";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		LogLifecycle("OnCreate " + this);
	}

	@Override
	protected void onResume() {
		super.onResume();
		LogLifecycle("OnResume " + this);
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		LogLifecycle("OnPause " + this);
	}
	
	@Override
	protected void onStop() {
		super.onStop();
		LogLifecycle("OnStop " + this);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		LogLifecycle("OnDestroy " + this);
	}
	
	@Override
	protected void onStart() {
		super.onStart();
		LogLifecycle("OnStart " + this);
	}
	
	private void LogLifecycle(String msg) {
		if(mLogLifecycle) {
			Log.i(TAG_LIFECYCLE, msg);
		}
	}

}
