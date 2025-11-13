/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import net.kishonti.ResultDetailView.ResultDetailData;
import net.kishonti.ResultDetailView.ResultDetailDataProvider;
import net.kishonti.customcomponents.NavigationBar;
import net.kishonti.swig.ResultItem;
import net.kishonti.theme.Localizator;
import net.kishonti.theme.chart.THLineChart;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewTreeObserver.OnGlobalLayoutListener;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.TableLayout.LayoutParams;

@SuppressLint("SetJavaScriptEnabled")
public class BatteryActivity extends Activity implements NavigationBar.OnNavBarClickListener, ResultDetailDataProvider {

    private ResultDetailData mDetailData;
    private LinearLayout mDetailList;
    private ScrollView mScroller;

    @Override
    protected void onDestroy() {
    	super.onDestroy();
    }

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

//		setContentView(R.layout.activity_battery);
		setContentView(Utils.getLayoutId(this, "activity_battery"));
		Intent i = getIntent();
		Bundle b = i.getExtras();
		if (!b.containsKey("result")) {
			return;
		}
		String resultString = b.getString("result");
		String resultName = b.getString("resultName");
		String resultFlags = b.getString("resultFlags");

		mDetailData = new ResultDetailData(resultString, resultName, resultFlags);

		NavigationBar navbar = (NavigationBar) findViewById(R.id.pageHeader);
		navbar.setOnNavBarClickListener(this);
		navbar.setLeftText(Localizator.getString(this, "Back"));
		navbar.setTitleText(Localizator.getString(this, "ResultDetailPageTitle"));

		mDetailList = (LinearLayout) findViewById(R.id.resultdetail_detailList);
		mScroller = (ScrollView) findViewById(R.id.resultdetail_detailScroller);

		addTitle();

		try {
			JSONObject data = new JSONObject(resultString);
			JSONObject result = data.getJSONArray("results").getJSONObject(0);

			addDetail("Score", "" + result.getDouble("score"));
			if(result.has("gfx_result")) {
				JSONObject gfx_result = result.getJSONObject("gfx_result");
				addDetail("Renderer", gfx_result.getString("renderer"));
				addDetail("Graphics Version", gfx_result.getString("graphics_version"));
				addDetail("Surface Width", "" + gfx_result.getLong("surface_width"));
				addDetail("Surface Height", "" + gfx_result.getLong("surface_height"));
			}
			addDetail("Benchmark Version", "" + result.getString("benchmark_version"));

			JSONArray charts = data.getJSONArray("charts");

			for (int j = 0; j < charts.length(); j++) {
				addChart(charts.getJSONObject(j).toString());
			}

		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}



	}

	private void addTitle() {
		TextView title = new TextView(this);
		title.setText(mDetailData.resultName);
		title.setTextSize(TypedValue.COMPLEX_UNIT_SP, 30);
		title.setTextColor(getResources().getColor(R.color.result_detail_title));
		mDetailList.addView(title);
	}

	private void addDetail(String name, String value) {
		LinearLayout layout = new LinearLayout(this);
		layout.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));
		layout.setOrientation(LinearLayout.HORIZONTAL);

		TextView detail = new TextView(this);
		detail.setTextAppearance(this, android.R.style.TextAppearance_Small);
		detail.setText(name + ": ");
		detail.setTextColor(getResources().getColor(R.color.result_detail_detail));

		TextView v = new TextView(this);
		v.setText(value);
		v.setTextAppearance(this, android.R.style.TextAppearance_Small);
		v.setTextColor(getResources().getColor(R.color.result_detail_value));

		layout.addView(detail);
		layout.addView(v);
		mDetailList.addView(layout);
	}

	private void addChart(String chartData) {
		final THLineChart chart = new THLineChart(this);
		mScroller.getViewTreeObserver().addOnGlobalLayoutListener(new OnGlobalLayoutListener() {
		    @Override
		    public void onGlobalLayout() {
		    	mScroller.getViewTreeObserver().removeOnGlobalLayoutListener(this);
		    	chart.setMaxHeight(mScroller.getHeight());
		    	chart.setMaxWidth(mScroller.getWidth());
		    }
		});
		chart.setData(chartData);
		LinearLayout.LayoutParams params = new LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1);
		chart.setLayoutParams(params);
		mDetailList.addView(chart);
	}

	@Override
	public void onLeftClick(View view) {
		finish();
	}

	@Override
	public void onRightClick(View view) {

	}

	public static Intent createBatteryIntent(Context context, ResultItem item) {
		Intent intents = new Intent(context, BatteryActivity.class);

		intents.putExtra("result", item.resultGroup().toString());
		intents.putExtra("resultName", Localizator.getString(context, item.resultId()));

		String flags = "";
		if(item.isFlagged()) {
			for(int i = 0; i < item.flags().size(); ++i) {
				flags = flags + " " + Localizator.getString(context, item.flags().get(i));
			}
		}
		intents.putExtra("resultFlags", flags);
		return intents;
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	public ResultDetailData getData() {
		return mDetailData;
	}
}
