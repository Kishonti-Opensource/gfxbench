/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw.app;

import net.kishonti.testfw.NativeLibraryLoader;


//import org.acra.ACRA;
import org.acra.annotation.ReportsCrashes;

import android.app.Application;
import android.os.Environment;

@ReportsCrashes(
		formKey = "",
		formUri = "https://kishonti.cloudant.com/acra-tfw/_design/acra-storage/_update/report",
		reportType = org.acra.sender.HttpSender.Type.JSON,
		httpMethod = org.acra.sender.HttpSender.Method.PUT,
		formUriBasicAuthLogin="strutheracomatshemainfie",
		formUriBasicAuthPassword="jA7n8rWtJoW4wJaDGjP472YG"
)
public class TestApplication extends Application {
	public static final String BASE_PATH = Environment.getExternalStorageDirectory().getAbsolutePath() + "/kishonti/tfw";

	@Override
	public void onCreate() {
		super.onCreate();
		//ACRA.init(this);
		net.kishonti.platform.Platform.setApplicationContext(this);
		NativeLibraryLoader.enableCopyToAppLibDir = true;
	}

}
