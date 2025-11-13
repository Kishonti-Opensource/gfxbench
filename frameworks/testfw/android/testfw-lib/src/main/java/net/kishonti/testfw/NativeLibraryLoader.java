/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;

import android.content.Context;
import android.util.Log;

public class NativeLibraryLoader {
	public static boolean enableCopyToAppLibDir;

	static void load(Context context, String lib) throws IOException {
		Log.i("NativeLoader", "NativeLoader Loading: " + lib);
		
		File f = new File(lib);
		if (f.isAbsolute()) {
			try {
				System.load(f.getAbsolutePath());
			} catch(UnsatisfiedLinkError e) {
				if (enableCopyToAppLibDir) {
					File dir = context.getDir("lib", Context.MODE_PRIVATE);
					File applib = new File(dir, f.getName());
					FileUtils.copyFile(f, applib);
					System.load(applib.getAbsolutePath());
				}
			}
		} else {
			System.loadLibrary(lib);
		}
	}
}
