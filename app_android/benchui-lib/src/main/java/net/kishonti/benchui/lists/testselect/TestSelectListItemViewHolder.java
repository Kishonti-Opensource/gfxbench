/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.testselect;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.swig.TestItem;
import net.kishonti.theme.Localizator;
import net.kishonti.theme.ThemeListItemViewHolder;

public class TestSelectListItemViewHolder implements ThemeListItemViewHolder,
	ThemeListItemViewHolder.ThemeHeaderedLIVH,
	ThemeListItemViewHolder.ThemeDisableableLIVH 
{
	
	private View mRoot;
	private View mSeparatorView;
	private View mHeaderView;
	private View mBodyView;
	
	private TextView mHeaderTextView;
	private TextView mNameTextView;
	private TextView mDescTextView;
	private TextView mDisableReasonTextView;
	
	private CheckBox mHeaderCB;
	private CheckBox mBodyCB;
	
	private ImageView mImageView;
	
	private Context mContext;
	

	@SuppressLint("InflateParams") 
	@Override
	public View inflateAndInit(Context context, ViewGroup parent, Object o, int index) {
		mContext = context;
		mRoot = LayoutInflater.from(context).inflate(R.layout.updated_test_select_item, null);
		
		mSeparatorView = mRoot.findViewById(R.id.updated_test_select_item_separator);
		mHeaderView = mRoot.findViewById(R.id.updated_test_select_item_header);
		mBodyView = mRoot.findViewById(R.id.updated_test_select_item_body);
		
		mHeaderTextView = (TextView)mRoot.findViewById(R.id.updated_test_select_item_group_title);
		mNameTextView = (TextView)mRoot.findViewById(R.id.updated_test_select_item_name);
		mDescTextView = (TextView)mRoot.findViewById(R.id.updated_test_select_item_desc);
		mDisableReasonTextView = (TextView)mRoot.findViewById(R.id.updated_test_select_item_disable_reason);
		
		mHeaderCB = (CheckBox)mRoot.findViewById(R.id.updated_test_select_item_group_checkbox);
		mBodyCB = (CheckBox)mRoot.findViewById(R.id.updated_test_select_item_checkbox);
		
		mImageView = (ImageView)mRoot.findViewById(R.id.updated_test_select_item_testicon);
		
		refreshFromItem(o, index);
		
		mHeaderCB.setClickable(false);
		mBodyCB.setClickable(false);
		
		return mRoot;
	}

	@Override
	public void refreshFromItem(Object o, int index) throws ClassCastException {
		if(!(o instanceof TestItem))
			throw new ClassCastException(this.getClass() + " needs TestItem objects as items.");
		
		TestItem item = (TestItem)o;
		if(item != null) {
			mSeparatorView.setVisibility(View.GONE);
			mBodyView.setVisibility(View.VISIBLE);
			mHeaderView.setVisibility(View.GONE);
			mBodyView.setBackgroundColor(mContext.getResources().getColor(R.color.list_cell));
			mHeaderView.setBackgroundColor(mContext.getResources().getColor(R.color.list_header));
			
			mHeaderTextView.setText(Localizator.getString(mContext, item.groupId()));
			mNameTextView.setText(Localizator.getString(mContext, item.testId()));
			mDescTextView.setText(Utils.getDescString(mContext, item));
			
			int iconID = mContext.getResources().getIdentifier(item.testId(), "drawable", mContext.getPackageName());
			if(iconID == 0) iconID = mContext.getResources().getIdentifier("dummy_icon" , "drawable", mContext.getPackageName());
			mImageView.setImageDrawable(mContext.getResources().getDrawable(iconID));
			
			mHeaderCB.setChecked(item.isGroupSelected());
			mBodyCB.setEnabled(true);
			mBodyCB.setChecked(item.isSelected() && item.isAvailable());
			mBodyCB.setVisibility(View.VISIBLE);
			mDisableReasonTextView.setVisibility(View.GONE);
			mDisableReasonTextView.setText(item.incompatibleText());
			
			if(item.isFirstInGroup()) layoutAsHeadered(index > 0);
			if(!item.isAvailable()) layoutAsDisabled();
		}
	}

	@Override
	public void layoutAsHeadered(boolean separated) {
		mSeparatorView.setVisibility(separated ? View.VISIBLE : View.GONE);
		mBodyView.setVisibility(View.GONE);
		mHeaderView.setVisibility(View.VISIBLE);
	}

	@Override
	public void layoutAsDisabled() {
		mBodyView.setBackgroundColor(mContext.getResources().getColor(R.color.list_cell_disabled));
		mBodyCB.setEnabled(false);
		mBodyCB.setVisibility(View.GONE);
		mDisableReasonTextView.setVisibility(View.VISIBLE);
	}

}
