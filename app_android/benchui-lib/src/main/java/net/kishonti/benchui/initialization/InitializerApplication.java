/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import net.kishonti.benchui.lists.resultlist.ResultFormatter;
import net.kishonti.systeminfo.swig.Properties;
import android.app.Application;
import android.content.Intent;

public class InitializerApplication extends Application {
	public static final String KEY_PREFSNAME = "mainPreferences";
	public boolean mIsInitialized = false;
	public boolean mInitStarted = false;
	protected String mBigDataDir = null;
	protected Properties mSystemInfo;
	protected boolean mBigDataNeedsManualUninstall = false;
	public static InitializerApplication instance = null;

	public interface OnApplicationInitializationListener {
		public void onApplicationInitialized();
		public void onApplicationInitFailed();
	}
	public List<OnApplicationInitializationListener> mListeners = new ArrayList<OnApplicationInitializationListener>();
	
	public InitializerApplication() {
		super();
		instance = this;
	}

	public synchronized void initialize(OnApplicationInitializationListener requester) {
		if (!mIsInitialized) {
			mListeners.add(requester);

			if (!mInitStarted) {
				mInitStarted = true;
				Intent initIntent = new Intent(this, InitActivity.class);
				initIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				startActivity(initIntent);
			}
		} else {
			requester.onApplicationInitialized();
		}
	}

	public synchronized void initEnd(boolean success, HashMap<String, Object> params) {
		
		if(success) {
			mSystemInfo = (Properties)params.get(CollectDeviceInfoTask.KEY_SYSTEMINFO);
			mBigDataDir = (String)params.get(DetermineBigDataDirTask.KEY_BIGDATADIR);
			getSharedPreferences(KEY_PREFSNAME, MODE_PRIVATE).edit().putString(DetermineBigDataDirTask.KEY_BIGDATADIR, mBigDataDir);
			
			mIsInitialized = true;
			for (OnApplicationInitializationListener listener : mListeners) {
				listener.onApplicationInitialized();
			}
		} else {
			mIsInitialized = false;
			mInitStarted = false;
			for (OnApplicationInitializationListener listener : mListeners) {
				listener.onApplicationInitFailed();
			}
		}
		mListeners.clear();
	}
	
	public synchronized ArrayList<InitTask> getInitTasks() {
		return new ArrayList<InitTask>();
	}
	
	public String getBigDataDir() {
		if(mBigDataDir == null) {
			if (getExternalFilesDir(null) != null) {
				return getExternalFilesDir(null).getPath();
			} else {
				return getFilesDir().getPath();
			}
		} else {
			return mBigDataDir;
		}
	}
	
	
	
	
	
	/**
	 * @category Compatibility
	 */
	public String getWorkingDir() {
		return getBigDataDir();
	}
	
	/**
	 * @category Compatibility
	 */
	public String getVersionString()
	{
		return "";
	}
	
	/**
	 * @category Compatibility
	 */
	public Properties getProperties() {
		return mSystemInfo;
	}
	
	/**
	 * @category Compatibility
	 */
	// should be overridden if the formatter is not the default
	public ResultFormatter getFormatter() {
		return new ResultFormatter();
	}
	public boolean isResultSavingEnabled() {
		return false;
	}
}
