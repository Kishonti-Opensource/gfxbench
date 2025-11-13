/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.lists.resultlist.ResultFormatter.FormattedResult;
import net.kishonti.swig.Result;
import net.kishonti.swig.ResultItem;
import net.kishonti.swig.Result.Status;
import net.kishonti.theme.Localizator;
import net.kishonti.theme.ThemeListItemViewHolder;

public class ResultListItemViewHolder implements 
	ThemeListItemViewHolder,
	ThemeListItemViewHolder.ThemeHeaderedLIVH
{
	private View mRoot;
	private View mSeparatorView;
	private View mHeaderView;
	private View mBodyView;
	
	private TextView mHeaderTextView;
	private TextView mNameTextView;
	private TextView mDescTextView;
	private TextView mResultTextView;
	private TextView mSubresultTextView;
	
	private RelativeLayout mScoreWrapper;
	
	private ImageView mImageView;
	
	private Context mContext;
	private boolean mDisplayScore = true;

	public ResultListItemViewHolder() {
	}
	
	public ResultListItemViewHolder(boolean displayScore) {
		mDisplayScore = displayScore;
	}

	@SuppressLint("InflateParams") 
	@Override
	public View inflateAndInit(Context context, ViewGroup parent, Object o,
			int index) {
		mContext = context;
		mRoot = LayoutInflater.from(context).inflate(R.layout.updated_result_item, null);
		
		mSeparatorView = mRoot.findViewById(R.id.updated_result_item_separator);
		mHeaderView = mRoot.findViewById(R.id.updated_result_item_header);
		mBodyView = mRoot.findViewById(R.id.updated_result_item_body);
		
		mHeaderTextView = (TextView)mRoot.findViewById(R.id.updated_result_item_group_title);
		mNameTextView = (TextView)mRoot.findViewById(R.id.updated_result_item_name);
		mDescTextView = (TextView)mRoot.findViewById(R.id.updated_result_item_desc);
		mResultTextView = (TextView)mRoot.findViewById(R.id.updated_result_item_result);
		mSubresultTextView = (TextView)mRoot.findViewById(R.id.updated_result_item_subresult);
		
		mScoreWrapper = (RelativeLayout)mRoot.findViewById(R.id.updated_result_item_score_wrapper);
		
		mImageView = (ImageView)mRoot.findViewById(R.id.updated_result_item_testicon);
		
		refreshFromItem(o, index);
		
		return mRoot;
	}

	@Override
	public void refreshFromItem(Object o, int index)
			throws ClassCastException {
		if(!(o instanceof ResultItem))
			throw new ClassCastException(this.getClass() + " needs TestItem objects as items.");
		
		ResultItem item = (ResultItem)o;
		if(item != null) {
			mSeparatorView.setVisibility(View.GONE);
			mBodyView.setVisibility(View.VISIBLE);
			mHeaderView.setVisibility(View.GONE);
			mBodyView.setBackgroundColor(mContext.getResources().getColor(R.color.list_cell));
			mHeaderView.setBackgroundColor(mContext.getResources().getColor(R.color.list_header));
			
			mHeaderTextView.setText(Localizator.getString(mContext, item.groupId()));
			mNameTextView.setText(Localizator.getString(mContext, item.resultId()));
			mDescTextView.setText(Localizator.getString(mContext, Utils.getDescString(mContext, item)));
			
			if(mDisplayScore) {
				mScoreWrapper.setVisibility(View.VISIBLE);
				boolean hasScore = item.score() >= 0 && item.errorString().equals("") && item.status() == Result.Status.OK;
				String errorString = item.errorString().equals("") ? getStatusString(item.status()) : item.errorString();
				if(!hasScore && item.status() == Result.Status.OK && item.unit().equals("chart")) {
					errorString = "Results_ShowBatteryDiagram";
				}
				
				ResultFormatter formatter = new ResultFormatter();
				FormattedResult f_result = formatter.getFormattedResult(item.score(), item.unit());
				
				if(hasScore) {
					if(item.isFlagged())
						mResultTextView.setText(f_result.getScore() + "*");
					else
						mResultTextView.setText(f_result.getScore());
					
				} else {
					mResultTextView.setText(Localizator.getString(mContext, errorString));
				}
				
				if(hasScore) {
					mSubresultTextView.setVisibility(View.VISIBLE);
					String unit = item.unit();
					if (item.hasSecondaryScore()) {
						double secondaryScore = item.secondaryScore();
						int frac = Math.max(0, 1 - (int) Math.floor(Math.log10(secondaryScore)));
						mResultTextView.setText(mResultTextView.getText() + " " + unit);
						unit = String.format("(%,."+frac+"f\u00A0%s)", secondaryScore, item.secondaryUnit()); 
					}
					mSubresultTextView.setText(unit);
				} else {
					mSubresultTextView.setVisibility(View.GONE);
				}
				
			} else {
				mScoreWrapper.setVisibility(View.GONE);
			}
			
			int iconID = mContext.getResources().getIdentifier(item.testId(), "drawable", mContext.getPackageName());
			if(iconID == 0) iconID = mContext.getResources().getIdentifier("dummy_icon" , "drawable", mContext.getPackageName());
			mImageView.setImageDrawable(mContext.getResources().getDrawable(iconID));
			
			if(item.isFirstInGroup()) layoutAsHeadered(index > 0);
		}
	}

	@Override
	public void layoutAsHeadered(boolean separated) {
		mSeparatorView.setVisibility(separated ? View.VISIBLE : View.GONE);
		mBodyView.setVisibility(View.GONE);
		mHeaderView.setVisibility(View.VISIBLE);
	}
	
	private String getStatusString(Result.Status status) {
		if(status == Status.CANCELLED) return "Cancelled";
		if(status == Status.FAILED) return "Failed";
		if(status == Status.INCOMPATIBLE) return "Incompatible";
		if(status == Status.OK) return "OK";
		
		return "Failed";
	}

}
