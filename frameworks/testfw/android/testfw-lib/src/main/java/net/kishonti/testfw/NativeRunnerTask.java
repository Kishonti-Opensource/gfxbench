/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import net.kishonti.swig.TestBase;
import android.os.AsyncTask;
import android.util.Log;

public class NativeRunnerTask extends AsyncTask<Void, Void, Void> {
	private static final String TAG = NativeRunnerTask.class.getName();
	private TestBase mTest = null;
	
	public NativeRunnerTask(TestBase test) {
		mTest = test;
	}
	
	@Override
	protected Void doInBackground(Void... params) {
		Log.i(TAG, "running test: " + mTest.name());
		mTest.run();
		return null;
	}
	
	public void cancelTest() {
		if (mTest != null) {
			mTest.cancel();
			Log.i(TAG, "native test cancelled");
			cancel(false);
		}
	}
}