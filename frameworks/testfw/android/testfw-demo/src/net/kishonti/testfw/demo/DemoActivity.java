/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw.demo;

import java.io.IOException;

import org.apache.commons.io.IOUtils;

import net.kishonti.platform.Platform;
import net.kishonti.testfw.TfwDemoActivity;
import android.content.Intent;
import android.os.Bundle;

public class DemoActivity extends TfwDemoActivity {
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		Platform.setApplicationContext(getApplicationContext());
		
		Intent intent = getIntent();
		Bundle b = intent.getExtras();
		if (b == null) {
			b = new Bundle();
		}
		if (!b.containsKey("config")) {
			try {
				String config = IOUtils.toString(getAssets().open("dummy_basic.json"), "UTF-8");
				b.putString("config", config);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		intent.putExtras(b);
		super.onCreate(savedInstanceState);
	}
}
