/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import android.app.Activity;

public class Device extends InfoProvider implements Runnable {

	private Properties props;
	private Map<String, String> env;

	public Device(Activity activity) {
		super(activity);
	}

	@Override
	public void run() {
		props = System.getProperties();
		env = System.getenv();
	}

	public String getMajorString() {
		return android.os.Build.BRAND + " " + android.os.Build.MODEL;
	}

	public String getMinorString() {
		return "";
	}

	public String toString() {
		return android.os.Build.BRAND + " " + android.os.Build.MODEL;
	}

	public String getRawData() {
		StringBuilder sb = new StringBuilder();
		sb.append("/** Device **/").append("\n");
		sb.append("android.os.Build.BRAND: ").append(android.os.Build.BRAND).append("\n");
		sb.append("android.os.Build.MODEL: ").append(android.os.Build.MODEL).append("\n");

		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		props.list(pw);
		pw.flush();
		sb.append("-- System.getProperties(): ").append("\n").append(sw.toString()).append("\n");
		try {
			sw.close();
		} catch (Throwable t) {
		}

		sb.append("-- System.getenv(): ").append("\n");
		Iterator<String> envKeysIterator = env.keySet().iterator();
		while (envKeysIterator.hasNext()) {
			String key = (String) envKeysIterator.next();
			String value = env.get(key);
			sb.append(key).append(": ").append(value).append("\n");
		}


		sb.append("/** END **/").append("\n");

		return sb.toString();
	}
}
