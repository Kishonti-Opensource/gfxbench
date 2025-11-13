/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.fragments;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.fragments.interfaces.BackButtonFragmentHandler;
import net.kishonti.benchui.fragments.interfaces.PageChangeRequestListener;
import net.kishonti.benchui.fragments.interfaces.PageChangeRequester;
import net.kishonti.benchui.fragments.interfaces.PageStateChangedHandler;
import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public abstract class BaseFragment extends Fragment implements 
	PageChangeRequester, BackButtonFragmentHandler, PageStateChangedHandler {
	
	private final boolean mLogLifecycle = true;
	private final String TAG_LIFECYCLE = "Lifecycle";
	
	/**
	 * Returns the default state of the fragment as string. 
	 * @return Default state of the fragment.
	 */
	public abstract String getDefaultStateString();
	
	@Override
	public PageChangeRequestListener getPageChangeRequestListener() {
		return BenchmarkApplication.getPageChangeRequestListener();
	}

	@Override
	public boolean HandleBackButton() {
		return false;
	}
	
	
	/**
	 * ***************************************************************
	 * Lifecycle logging.
	 * ***************************************************************
	 */
	
	/**
	 * Logs the lifecicle methods on logcat.
	 * @param msg
	 */
	private void LogLifecycle(String msg) {
		if(mLogLifecycle) {
			Log.i(TAG_LIFECYCLE, msg);
		}
	}
	
	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		LogLifecycle("OnAttach " + this + " to: " + activity);
		LogLifecycle("ONATTACH class:" + this.getClass() + ", id:" + this.getId());
	}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		LogLifecycle("onCreate " + this);
	}
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
		View v = super.onCreateView(inflater, container, savedInstanceState);
		LogLifecycle("onCreateView " + this);
		return v;
    }

	@Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
		super.onViewCreated(view, savedInstanceState);
		LogLifecycle("onViewCreated " + this);
    }
	
	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		LogLifecycle("onActivityCreated " + this);
    }
	
	@Override
    public void onStart() {
		super.onStart();
		LogLifecycle("onStart " + this);
	}
	
	@Override
	public void onResume() {
		super.onResume();
		LogLifecycle("OnResume " + this);
	}
	
	@Override
    public void onPause() {
		super.onPause();
		LogLifecycle("onPause " + this);
    }
	
	@Override
    public void onStop() {
		super.onStop();
		LogLifecycle("onStop " + this);
    }
	
	@Override
	public void onDestroy() {
		super.onDestroy();
		LogLifecycle("onDestroy " + this);
	}
}
