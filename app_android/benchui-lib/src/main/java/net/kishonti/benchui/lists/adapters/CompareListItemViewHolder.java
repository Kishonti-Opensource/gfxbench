/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.adapters;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.R;
import net.kishonti.benchui.lists.resultlist.ResultFormatter;
import net.kishonti.swig.CompareResult;
import net.kishonti.theme.Localizator;
import net.kishonti.theme.ThemeListItemViewHolder;

public class CompareListItemViewHolder implements ThemeListItemViewHolder
{
	
	private View mRoot;
	private ProgressBar mProgressBar;
	private ImageView mDeviceImage;
	
	private TextView mScore;
	private TextView mUnit;
	private TextView mName;
	private TextView mDesc;
	
	private Context mContext;
	
	private double mMax;
	private String mUnitName;
	
	public CompareListItemViewHolder(double max, String unit) {
		mMax = max;
		mUnitName = unit;
	}

	@SuppressLint("InflateParams") 
	@Override
	public View inflateAndInit(Context context, ViewGroup parent, Object o, int index) {
		mContext = context;
		mRoot = LayoutInflater.from(context).inflate(R.layout.cell_compare_device, null);
		
		mProgressBar = (ProgressBar)mRoot.findViewById(R.id.cell_compare_progressBar);
		mDeviceImage = (ImageView)mRoot.findViewById(R.id.cell_compare_deviceImage);
		mScore = (TextView)mRoot.findViewById(R.id.cell_compare_resultScore);
		mUnit = (TextView)mRoot.findViewById(R.id.cell_compare_resultUnit);
		mName = (TextView)mRoot.findViewById(R.id.cell_compare_deviceName);
		mDesc = (TextView)mRoot.findViewById(R.id.cell_compare_deviceDesc);
		
		refreshFromItem(o, index);
		
		return mRoot;
	}

	@Override
	public void refreshFromItem(Object o, int index) throws ClassCastException {
		if(!(o instanceof CompareResult))
			throw new ClassCastException(this.getClass() + " needs CompareResult objects as items.");
		
		CompareResult item = (CompareResult)o;
		
		mProgressBar.setProgressDrawable(mContext.getResources().getDrawable(R.drawable.compare_device_bar));
		
		mName.setText("");
		mDesc.setText("");
		mScore.setText("");
		mUnit.setText("");
		mDeviceImage.setImageDrawable(null);
		mProgressBar.setMax(0);
		mProgressBar.setProgress(0);
		
		if(item != null) {
			String deviceImageString = item.deviceImage();
			int lastSeparatorIndex = deviceImageString.lastIndexOf("/");
			String path = BenchmarkApplication.instance.getWorkingDir() + "/image/device/";
			String fullpath = path + deviceImageString.substring(lastSeparatorIndex + 1);
			Bitmap bitmap = BitmapFactory.decodeFile(fullpath);
			
			mName.setText(item.deviceName());
			mDesc.setText(item.vendor() + " | " + Localizator.getString(mContext, item.api()));
			mProgressBar.setMax(1);
			mProgressBar.setProgress(0);
			mDeviceImage.setImageBitmap(bitmap);
			
			if(item.score() < 0) {
				mScore.setText("");
				mScore.setVisibility(View.GONE);
				mUnit.setText(Localizator.getString(mContext, "Results_NA"));
			} else {
				String itemUnit = mUnitName;
				ResultFormatter.FormattedResult formattedResult = 
						new ResultFormatter().getFormattedResult(item.score(), itemUnit);
				itemUnit = formattedResult.getUnit().contentEquals(itemUnit) ? formattedResult.getUnit() : null;

				int max = 1000;
				int score = (int)(item.score() / mMax * max);
				mProgressBar.setMax(max);
				mProgressBar.setProgress(score);
				
				mUnit.setText(itemUnit == null ? mUnitName : itemUnit);
				mScore.setVisibility(View.VISIBLE);
				mScore.setText(formattedResult.getScore());
			}
		}
	}
}
