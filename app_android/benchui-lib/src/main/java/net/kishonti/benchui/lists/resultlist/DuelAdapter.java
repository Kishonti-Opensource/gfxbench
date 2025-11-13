/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.swig.CompareItem;
import net.kishonti.swig.ResultItem;
import net.kishonti.theme.Localizator;
import gfxc.dao.Result;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;

public class DuelAdapter extends BaseAdapter implements ListAdapter {

	private Context mContext;
	private ResultFormatter mFormatter;
	private DuelListDataProvider mDataProvider;
	
	public DuelAdapter(Context context, DuelListDataProvider dataProvider) {
		mContext = context;
		mDataProvider = dataProvider;
		mFormatter = new ResultFormatter();
	}

	@Override
	public int getCount() {
		return (int)mDataProvider.getDuelCount();
	}

	@Override
	public Object getItem(int position) {
		return mDataProvider.getDuelItemForPosition(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View v, ViewGroup parent) {
		CompareItem item = (CompareItem)getItem(position);

		if (v == null)
			v = newView(mContext, item, parent);
		
		DuelCellContainer c = (DuelCellContainer)v.getTag();
		
		int img_id = mContext.getResources().getIdentifier(item.resultId(), "drawable", mContext.getPackageName());
		if (img_id == 0) img_id = net.kishonti.benchui.R.drawable.dummy_icon;
		
		c.mTestImage.setImageResource(img_id);
		c.mTestTitle.setText(Localizator.getString(mContext, item.resultId()));

		if (item.score() <= 0 || item.status() != net.kishonti.swig.Result.Status.OK) {
			c.mYourScore.setText("");
			c.mYourScore.setVisibility(View.GONE);
			c.mYourUnit.setText(Localizator.getString(mContext, "Results_NA"));
		} else {
			String itemUnit = item.unit();
			ResultFormatter.FormattedResult formattedResult = 
					mFormatter.getFormattedResult(item.score(), itemUnit);
			c.mYourScore.setText(formattedResult.getScore());
			c.mYourScore.setVisibility(View.VISIBLE);
			c.mYourUnit.setText(formattedResult.getUnit());
		}
		
		if (item.compareScore() < 0) {
			c.mOppoScore.setText("");
			c.mOppoScore.setVisibility(View.GONE);
			c.mOppoUnit.setText(Localizator.getString(mContext, "Results_NA"));
		} else {
			String itemUnit = item.unit();
			ResultFormatter.FormattedResult formattedResult = 
					mFormatter.getFormattedResult(item.compareScore(), itemUnit);
			c.mOppoScore.setText(formattedResult.getScore());
			c.mOppoScore.setVisibility(View.VISIBLE);
			c.mOppoUnit.setText(formattedResult.getUnit());
		}
		
		double yScore = item.score();
		double oScore = item.compareScore();
		if ((yScore > 0) && (oScore > 0)) {
			double x;

			if (yScore > oScore) {
				x = yScore / oScore;
			} else {
				x = -oScore / yScore;
			}
			
			String duelString = "";

			if (Math.abs(x) >= 1000) {
				duelString = "INF";
			} else if (Math.abs(x) >= 2) {
				int digits = (int) Math.floor(Math.log10(Math.abs(x))) + 1;
				if (digits < 1) digits = 1;
				duelString = "x" + String.format("%,." + (3 - digits) + "f", Math.abs(x));
			} else if (Math.abs(x) >= 1.05) {
				double percentageValue = (Math.abs(x) - 1) * 100;
				int digits = (int) Math.floor(Math.log10(percentageValue)) + 1;
				if (digits < 1) digits = 1;
				duelString = String.format("%,." + (3 - digits) + "f", percentageValue) + "%";
			} else {
				duelString = "";
			}

			if (x >= 1.05) {
				c.mLeftArrow.setImageResource(R.drawable.duel_arrow_green);
				c.mLeftPercent.setText(duelString);
				c.mLeftArrow.setVisibility(View.VISIBLE);
				c.mLeftPercent.setVisibility(View.VISIBLE);
				c.mRightArrow.setVisibility(View.INVISIBLE);
				c.mRightPercent.setVisibility(View.INVISIBLE);
				
			} else if (x > -1.05) {
				c.mLeftArrow.setVisibility(View.VISIBLE);
				c.mRightArrow.setVisibility(View.VISIBLE);
				c.mLeftArrow.setImageResource(R.drawable.duel_arrow_blue_left);
				c.mLeftPercent.setVisibility(View.INVISIBLE);
				c.mRightArrow.setImageResource(R.drawable.duel_arrow_blue_right);
				c.mRightPercent.setVisibility(View.INVISIBLE);
				
			} else {
				c.mLeftArrow.setVisibility(View.INVISIBLE);
				c.mLeftPercent.setVisibility(View.INVISIBLE);
				c.mRightArrow.setVisibility(View.VISIBLE);
				c.mRightPercent.setVisibility(View.VISIBLE);
				c.mRightArrow.setImageResource(R.drawable.duel_arrow_red);
				c.mRightPercent.setText(duelString);
			}

		} else {
			c.mLeftArrow.setVisibility(View.INVISIBLE);
			c.mLeftPercent.setVisibility(View.INVISIBLE);
			c.mRightArrow.setVisibility(View.INVISIBLE);
			c.mRightPercent.setVisibility(View.INVISIBLE);
		}

		return v;
	}
	
	public View newView(Context context, CompareItem item, ViewGroup parent) {
		if (item == null) return null;
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		
		DuelCellContainer c = new DuelCellContainer();
//		View v = inflater.inflate(R.layout.cell_duel, parent, false);
		View v = inflater.inflate(Utils.getLayoutId(context, "cell_duel"), parent, false);
		
		c.mYourScore = (TextView)v.findViewById(R.id.cell_duel_ownScore);
		c.mYourUnit = (TextView)v.findViewById(R.id.cell_duel_ownUnit);    
		c.mOppoScore = (TextView)v.findViewById(R.id.cell_duel_theirScore);   
		c.mOppoUnit = (TextView)v.findViewById(R.id.cell_duel_theirUnit);    
		              
		c.mTestTitle = (TextView)v.findViewById(R.id.cell_duel_testName);  
		c.mLeftPercent = (TextView)v.findViewById(R.id.cell_duel_leftPercent); 
		c.mRightPercent = (TextView)v.findViewById(R.id.cell_duel_rightPercent);
		c.mRightArrow = (ImageView)v.findViewById(R.id.cell_duel_rightArrow); 
		c.mLeftArrow = (ImageView)v.findViewById(R.id.cell_duel_leftArrow);  
		c.mTestImage = (ImageView)v.findViewById(R.id.cell_duel_testImage);  
		
		v.setTag(c);
		
		return v;
		
	}
	
	private class DuelCellContainer {
		public TextView mYourScore;
		public TextView mYourUnit;
		public TextView mOppoScore;
		public TextView mOppoUnit;

		public TextView mTestTitle;
		public TextView mLeftPercent;
		public TextView mRightPercent;
		public ImageView mRightArrow;
		public ImageView mLeftArrow;
		public ImageView mTestImage;
	}
	
}
