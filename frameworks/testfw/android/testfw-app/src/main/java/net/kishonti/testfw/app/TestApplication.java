/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw.app;

import net.kishonti.testfw.NativeLibraryLoader;
import android.app.Application;
import android.os.Environment;

public class TestApplication extends Application {
	public static final String BASE_PATH = Environment.getExternalStorageDirectory().getAbsolutePath() + "/kishonti/tfw";

	@Override
	public void onCreate() {
		super.onCreate();
		net.kishonti.platform.Platform.setApplicationContext(this);
		NativeLibraryLoader.enableCopyToAppLibDir = true;
	}
}
