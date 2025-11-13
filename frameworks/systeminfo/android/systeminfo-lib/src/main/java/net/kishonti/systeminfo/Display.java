/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.os.Build;
import android.util.DisplayMetrics;
import android.view.WindowManager;

public class Display extends InfoProvider implements Runnable {

	// TODO: Display hardware test on different screens, eg on hdmi stick (no screen at all), resistive screen (is that fake touch?)

	static int xResolution = 0;
	static int yResolution = 0;
	static float xDpi = 0;
	static float yDpi = 0;
	static double screenInches = 0;

	public static enum TOUCH_SCREEN {
		Unsupported, Fake, Real
	}

	TOUCH_SCREEN touchScreen = TOUCH_SCREEN.Unsupported;
	int touchFingerCount = 0;

	boolean FEATURE_TOUCHSCREEN = false;
	boolean FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND = false;
	boolean FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT = false;
	boolean FEATURE_TOUCHSCREEN_MULTITOUCH = false;

	boolean FEATURE_FAKETOUCH = false;
	boolean FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND = false;
	boolean FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT = false;

	public Display(Context context) {
		super(context);
	}

	@SuppressLint("InlinedApi")
	private void checkFakeTouch() {
		FEATURE_FAKETOUCH = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_FAKETOUCH);
		FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND);
		FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT);
	}

	public static Method mGetRawH = null, mGetRawW = null;

	@SuppressLint("NewApi")
	@Override
	public void run() {
		if (context != null) {
			DisplayMetrics dm = Resources.getSystem().getDisplayMetrics();
			int xx = 0;
			int yy = 0;
			if (android.os.Build.VERSION.SDK_INT > 16) {
				DisplayMetrics size = new DisplayMetrics(); 
				WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
				android.view.Display display = windowManager.getDefaultDisplay();
				display.getRealMetrics(size);
				xx = size.widthPixels;
				yy = size.heightPixels;
			} else if(android.os.Build.VERSION.SDK_INT > 13) {
				WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
				android.view.Display display = windowManager.getDefaultDisplay();

				try {
					Method mGetRawW = Display.class.getMethod("getRawWidth");
					Method mGetRawH = Display.class.getMethod("getRawHeight");
					
					xx = (Integer) mGetRawW.invoke(display);
					yy = (Integer) mGetRawH.invoke(display);
				} catch (Exception e) {
					e.printStackTrace();

					xx = display.getWidth();
					yy = display.getHeight();
				}
			}
			xResolution = xx;
			yResolution = yy;
			xDpi = dm.xdpi;
			yDpi = dm.ydpi;
			double x = xResolution / xDpi;
			double y = yResolution / yDpi;
			screenInches = Math.sqrt(x * x + y * y);

			FEATURE_TOUCHSCREEN = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN);
			FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND);
			FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT);
			FEATURE_TOUCHSCREEN_MULTITOUCH = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN_MULTITOUCH);

			if (Build.VERSION.SDK_INT > 12) {
				checkFakeTouch();
			}
		}

		if (FEATURE_TOUCHSCREEN) {
			touchScreen = TOUCH_SCREEN.Real;
			if (FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND) {
				touchFingerCount = 5;
			} else if (FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT || FEATURE_TOUCHSCREEN_MULTITOUCH) {
				touchFingerCount = 2;
			} else {
				touchFingerCount = 1;
			}
		} else if (FEATURE_FAKETOUCH) {
			touchScreen = TOUCH_SCREEN.Fake;
			if (FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND) {
				touchFingerCount = 5;
			} else if (FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT) {
				touchFingerCount = 2;
			} else {
				touchFingerCount = 1;
			}
		}
	}

	public String getRawData() {
		StringBuilder sb = new StringBuilder();
		sb.append("/** Display **/").append("\n");
		sb.append("xResolution: ").append(xResolution).append("\n");
		sb.append("yResolution: ").append(yResolution).append("\n");
		sb.append("xDpi: ").append(xDpi).append("\n");
		sb.append("yDpi: ").append(yDpi).append("\n");
		sb.append("FEATURE_TOUCHSCREEN: ").append(FEATURE_TOUCHSCREEN).append("\n");
		sb.append("FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND: ").append(FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND).append("\n");
		sb.append("FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT: ").append(FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT).append("\n");
		sb.append("FEATURE_TOUCHSCREEN_MULTITOUCH: ").append(FEATURE_TOUCHSCREEN_MULTITOUCH).append("\n");

		sb.append("FEATURE_FAKETOUCH: ").append(FEATURE_FAKETOUCH).append("\n");
		sb.append("FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND: ").append(FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND).append("\n");
		sb.append("FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT: ").append(FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT).append("\n");
		sb.append("/** END **/").append("\n");

		return sb.toString();
	}

	public String getMajorString() {
		float roundedScreenSize = Math.round(screenInches * 10f) / 10f;
		return xResolution + " x " + yResolution + ", " + String.format("%.1f", roundedScreenSize) + "\"";

	}

	public String getMinorString() {
		String touchString = "";
		switch (touchScreen) {
			case Unsupported:
				touchString = "no touchscreen";
				break;
			case Fake:
				touchString = "resistive touchscreen";
				break;
			case Real:
				touchString = "capacitive touchscreen";
				break;
		}
		if (touchFingerCount > 0) {
			if (touchFingerCount >= 5) {
				touchString += " with at least 5 finger gesture support";
			} else if (touchFingerCount >= 2) {
				touchString += " with multitouch guesture support";
			} else {

			}
		}
		return touchString;

	}

	public String toString() {
		String touchString = "";
		switch (touchScreen) {
			case Unsupported:
				touchString = "no touchscreen";
				break;
			case Fake:
				touchString = "resistive touchscreen";
				break;
			case Real:
				touchString = "capacitive touchscreen";
				break;
		}
		if (touchFingerCount > 0) {
			if (touchFingerCount >= 5) {
				touchString += " with at least 5 finger gesture support";
			} else if (touchFingerCount >= 2) {
				touchString += " with multitouch guesture support";
			} else {

			}
		}
		return "Screen: " + xResolution + " x " + yResolution + ", " + screenInches + "\"" + " " + touchString;
	}

	Map<String, Boolean> getFeatures() {
		Map<String, Boolean> features = new HashMap<String, Boolean>();
		features.put("FEATURE_TOUCHSCREEN", FEATURE_TOUCHSCREEN);
		features.put("FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND", FEATURE_TOUCHSCREEN_MULTITOUCH_JAZZHAND);
		features.put("FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT", FEATURE_TOUCHSCREEN_MULTITOUCH_DISTINCT);
		features.put("FEATURE_TOUCHSCREEN_MULTITOUCH", FEATURE_TOUCHSCREEN_MULTITOUCH);
		features.put("FEATURE_FAKETOUCH", FEATURE_FAKETOUCH);
		features.put("FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND", FEATURE_FAKETOUCH_MULTITOUCH_JAZZHAND);
		features.put("FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT", FEATURE_FAKETOUCH_MULTITOUCH_DISTINCT);
		return features;
	}
}
