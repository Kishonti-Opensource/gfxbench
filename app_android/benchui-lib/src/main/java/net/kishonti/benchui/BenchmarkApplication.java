/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import android.content.Context;
import net.kishonti.benchui.fragments.interfaces.PageChangeRequestListener;
import net.kishonti.benchui.initialization.InitializerApplication;
import net.kishonti.benchui.model.BenchmarkModel;
import net.kishonti.benchui.model.BenchmarkTestModel;
import net.kishonti.benchui.model.MinimalProps;
import net.kishonti.platform.Platform;
import net.kishonti.swig.ApiDefinitionVector;

import net.kishonti.testfw.TfwActivity;
import android.app.Activity;
import android.os.Bundle;
import android.app.Application.ActivityLifecycleCallbacks;

public abstract class BenchmarkApplication extends InitializerApplication {
	protected static final String TAG = "BenchmarkApplication";
	protected String workingDir = null;

	static {
		System.loadLibrary("c++_shared");
		System.loadLibrary("systeminfo_jni");
	}

	class TestLifecycleHandler implements ActivityLifecycleCallbacks {

		private int isTfwActivityRunning = 0;

		public TestLifecycleHandler()
		{
			isTfwActivityRunning = 0;
		}

		@Override
		public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
			if (activity instanceof TfwActivity)
			{
				isTfwActivityRunning++;
			}
		}

		@Override
		public void onActivityDestroyed(Activity activity) {
			if (activity instanceof TfwActivity)
			{
				isTfwActivityRunning--;
			}
		}

		@Override
		public void onActivityStarted(Activity activity) { }

		@Override
		public void onActivityStopped(Activity activity) { }

		@Override
		public void onActivityResumed(Activity activity) { }

		@Override
		public void onActivityPaused(Activity activity) { }

		@Override
		public void onActivitySaveInstanceState(Activity activity, Bundle outState) { }

		public boolean isTfwActivityRunning()
		{
			return isTfwActivityRunning > 0;
		}
	}

	private TestLifecycleHandler tlh = null;

	public BenchmarkApplication() {
		super();
		instance = this;

		tlh = new TestLifecycleHandler();
		registerActivityLifecycleCallbacks(tlh);
	}

	public boolean isTfwActivityRunning()
	{
		return tlh != null && tlh.isTfwActivityRunning();
	}

	@Override
	public void onCreate() {
		super.onCreate();
		Platform.setApplicationContext(this);
	}

	public abstract String getVersionString();
	public abstract DescriptorFactory getDescriptorFactory();

	public String getCompareDatabasePath(String base)
	{
		return base + "/data/top-results.sqlite";
	}

	private static BenchmarkTestModel model = null;
	public static BenchmarkTestModel getModel(Context context) {
		boolean isTablet = context.getResources().getBoolean(R.bool.isMultiColumn);
		if(model == null) {
			model = new BenchmarkTestModel(context, isTablet);
			model.init(context, isTablet);
		}
//		if(((BenchmarkApplication)context.getApplicationContext()).mIsInitialized && !model.isInitialized())

		return model;
	}

	private static MinimalProps mMinimalProps = null;
	public static MinimalProps getMinimalProps(Context context) {
		if(mMinimalProps == null) mMinimalProps = new MinimalProps(context, false);
		return mMinimalProps;
	}
	public static MinimalProps getMinimalProps(Context context, Boolean cmdline) {
		if(mMinimalProps == null) mMinimalProps = new MinimalProps(context, cmdline);
		return mMinimalProps;
	}

	private static BenchmarkModel appmodel = null;
	public static BenchmarkModel getAppModel() {
		if(appmodel == null) appmodel = new BenchmarkModel();
		return appmodel;
	}

	private static ApiDefinitionVector apiDefinitions = null;
	public static ApiDefinitionVector getApiDefinitions() {
		if(apiDefinitions == null) apiDefinitions = new ApiDefinitionVector();
		return apiDefinitions;
	}

	private static PageChangeRequestListener mPageChangeListener = null;
	public static PageChangeRequestListener getPageChangeRequestListener() {
		return mPageChangeListener;
	}

	public static void setPageChangeRequestListener(PageChangeRequestListener l) {
		mPageChangeListener = l;
	}

	public Boolean isDetailDiagramCompatible() {
		return false;
	}
}
