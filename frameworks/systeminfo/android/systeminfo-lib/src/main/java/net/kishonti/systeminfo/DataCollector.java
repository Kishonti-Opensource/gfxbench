/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Map.Entry;
import java.util.Vector;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.json.JSONArray;
import org.json.JSONException;

import android.content.Context;
import android.os.Build;
import android.util.Log;
import net.kishonti.systeminfo.swig.Properties;
import net.kishonti.systeminfo.swig.SystemInfo;
import net.kishonti.systeminfo.swig.systeminfolib;

public class DataCollector {

	private final Context mContext;
	private final long save_minutes = 60;

	public DataCollector(Context context) {
		mContext = context;
	}

	public void collectData(Properties props) {
//		Features features = new Features(mContext);
//		features.run();
//
//		Calendar cal = Calendar.getInstance();
//		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
//		sdf.format(cal.getTime());
//
//		Date saved_date = null;
//		Date now = null;
//
//		String date_string = mContext.getSharedPreferences("prefs", Context.MODE_PRIVATE).getString("device_information_record_date", "");
//		if (date_string.length() > 0) {
//			try {
//				saved_date = sdf.parse(date_string);
//				now = cal.getTime();
//
//				String result = mContext.getSharedPreferences("prefs", Context.MODE_PRIVATE).getString("device_information", "");
//				long diff = now.getTime() - saved_date.getTime();
//				if ((diff < save_minutes * 60 * 1000) && result.length() > 0) {
//					props.updateFromJsonString(result);
//					return;
//				}
//			} catch (ParseException e) {
//				Log.i("DataCollector", "There is no saved infos");
//			}
//		}
//
//		SystemInfo sysinf = new SystemInfo();
//		props.collect(new AndroidDeviceInfoCollector(mContext), sysinf);
//
//		mContext.getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putString("device_information_record_date", sdf.format(cal.getTime()).toString())
//				.commit();
//		mContext.getSharedPreferences("prefs", Context.MODE_PRIVATE).edit().putString("device_information", props.toJsonString()).commit();
	}

	public static List<String> getCBJNIOpenCLDeviceList() {
		List<String> openCLConfigList = new ArrayList<String>();
		try {
			System.loadLibrary("c++_shared");
			// TODO: THIS MODULE IS IMPLEMENTED IN COMPUBENCH CL
			System.loadLibrary("cbjni");
			String s = getOpenCLDeviceList();
			if (!s.equals("null")) {
				JSONArray jconfigs = new JSONArray(s);
				for (int i = 0; i < jconfigs.length(); ++i)	{
					openCLConfigList.add(jconfigs.getString(i));
				}
			} else {
				Log.d("systeminfo", "no OpenCL device found");
			}
		} catch(JSONException e) {
			Log.e("systeminfo", "Failed parse OpenCL device list: " + e.getLocalizedMessage() );
		} catch(UnsatisfiedLinkError e) {
			Log.d("systeminfo", "Failed to load cbjni: " + e.getLocalizedMessage() );
		}
		return openCLConfigList;
	}
	private static native String getOpenCLDeviceList();

	public static Boolean isCUDAcompatible() {
		Boolean ret = false;
		try {
			System.loadLibrary("cuda");
			ret = true;

		} catch(UnsatisfiedLinkError e) {
			Log.d("systeminfo", "Failed to load cuda: " + e.getLocalizedMessage() );
		}
		return ret;
	}
}
