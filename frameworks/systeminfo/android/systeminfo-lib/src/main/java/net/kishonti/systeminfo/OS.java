/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.util.HashMap;
import java.util.Map;

public class OS {

	String build() {
		return android.os.Build.VERSION.RELEASE;
	}

	String name() {
		return "Android";
	}

	Map<String, String> buildDetails() {
		Map<String, String> details = new HashMap<String, String>();
		details.put("GLBPD_OS_BUILD_BOARD", android.os.Build.BOARD);
		details.put("GLBPD_OS_BUILD_BRAND", android.os.Build.BRAND);
		details.put("GLBPD_OS_BUILD_DEVICE", android.os.Build.DEVICE);
		details.put("GLBPD_OS_BUILD_DISPLAY", android.os.Build.DISPLAY);
		details.put("GLBPD_OS_BUILD_FINGERPRINT", android.os.Build.FINGERPRINT);
		details.put("GLBPD_OS_BUILD_HOST", android.os.Build.HOST);
		details.put("GLBPD_OS_BUILD_ID", android.os.Build.ID);
		details.put("GLBPD_OS_BUILD_MODEL", android.os.Build.MODEL);
		details.put("GLBPD_OS_BUILD_PRODUCT", android.os.Build.PRODUCT);
		details.put("GLBPD_OS_BUILD_TAGS", android.os.Build.TAGS);
		details.put("GLBPD_OS_BUILD_TYPE", android.os.Build.TYPE);
		details.put("GLBPD_OS_BUILD_USER", android.os.Build.USER);
		details.put("GLBPD_OS_BUILD_ABI", android.os.Build.CPU_ABI + "");
		details.put("GLBPD_OS_BUILD_ABI2", android.os.Build.CPU_ABI2 + "");
		details.put("GLBPD_OS_BUILD_TIME", android.os.Build.TIME + "");
		details.put("GLBPD_OS_BUILD_VERSION.INCREMENTAL", android.os.Build.VERSION.INCREMENTAL);
		details.put("GLBPD_OS_BUILD_VERSION.RELEASE", android.os.Build.VERSION.RELEASE);
		details.put("GLBPD_OS_BUILD_VERSION.SDK", "" + android.os.Build.VERSION.SDK_INT);
		
		details.put("GLBPD_OS_BUILD_BOOTLOADER", android.os.Build.BOOTLOADER);
		details.put("GLBPD_OS_BUILD_HARDWARE", android.os.Build.HARDWARE);
		details.put("GLBPD_OS_BUILD_MANUFACTURER", android.os.Build.MANUFACTURER);
		details.put("GLBPD_OS_BUILD_SERIAL", android.os.Build.SERIAL);
		return details;
	}

}
