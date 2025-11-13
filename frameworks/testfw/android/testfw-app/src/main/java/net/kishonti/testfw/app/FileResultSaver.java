/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw.app;

import java.io.File;
import java.io.IOException;

import net.kishonti.swig.ResultGroup;

import org.apache.commons.io.FileUtils;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class FileResultSaver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent data) {
		if (data != null && data.hasExtra("result")) {
			String result = data.getStringExtra("result");
			ResultGroup rg = new ResultGroup();
			rg.fromJsonString(result);
			File resultDir = new File(TestApplication.BASE_PATH + "/results/");
			try {
				resultDir.mkdirs();
				FileUtils.write(new File(resultDir, rg.testId() + ".json"), result);
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
		}
	}

}
