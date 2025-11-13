/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import java.util.ArrayList;
import java.util.List;


import net.kishonti.customcomponents.CustomProgressbar;
import net.kishonti.testfw.TestRunner;
import net.kishonti.testfw.TfwActivity;
import net.kishonti.theme.Localizator;
import net.kishonti.swig.Descriptor;
import android.graphics.Point;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.GridLayout;
import android.view.ViewGroup.LayoutParams;

//public class BenchTestActivity extends TfwActivity {}

public class BenchTestActivity extends TfwActivity {
	private View mLoadingView;

	private ImageView mBackgroundImage;
	private TextView mTestName;
	private TextView mTestInfo;
	private TextView mLoadingMessage;
	private CustomProgressbar mProgress;

	@Override
    protected List<SurfaceView> initContentView(List<Descriptor> descriptors) {
        List<SurfaceView> surfaces = new ArrayList<SurfaceView>();

		setContentView(Utils.getLayoutId(this, "layout_bench_test_activity"));
		SurfaceView surface = (SurfaceView) findViewById(R.id.surfaceView);
		mLoadingView = findViewById(R.id.bench_test_activity_init);
		mBackgroundImage = (ImageView)mLoadingView.findViewById(R.id.loadingscreen_backimage);
		mLoadingMessage = (TextView)mLoadingView.findViewById(R.id.loadingscreen_loadingtext);
		mTestName = (TextView)mLoadingView.findViewById(R.id.loadingscreen_testname);
		mTestInfo = (TextView)mLoadingView.findViewById(R.id.loadingscreen_testinfo);
		mProgress = (CustomProgressbar)mLoadingView.findViewById(R.id.loadingscreen_progress);
		Point screenSize = new Point();
		getWindowManager().getDefaultDisplay().getSize(screenSize);

		String appName = getResources().getString(R.string.app_name);
		mLoadingMessage.setText(appName + " is now loading");
		String testId = descriptors.get(0).testId();
		String testName = Localizator.getString(this, testId);
		mTestName.setText(testName);

		mTestInfo.setText("");
		if(testId != null) {
			if(testId.contains("_off")) {
				try {
					mTestInfo.setText(String.format(Localizator.getString(this, "TestLoading"), testName));
				} catch (Exception e) {
					e.printStackTrace();
					mTestInfo.setText(Localizator.getString(this, "TestLoading"));
				}
			}
			int back_id = getResources().getIdentifier(testId + "_loading", "drawable", getPackageName());
			if(back_id != 0) {
				mBackgroundImage.setImageDrawable(getResources().getDrawable(back_id));
			}
		} else {
			mBackgroundImage.setImageResource(R.drawable.dummy_loading);
		}

		mProgress.onProgressChanged(0, true);

		surfaces.add(surface);
		return surfaces;
	}

	@Override
	public void onTestInitialized(TestRunner t) {
		ViewGroup root = (ViewGroup) findViewById(R.id.tfw_root_frame);
		root.removeView(mLoadingView);

		super.onTestInitialized(t);
	}

	@Override
	public void onProgressChanged(TestRunner runner, float progress) {
		mProgress.onProgressChanged(progress, true);
	}
}
