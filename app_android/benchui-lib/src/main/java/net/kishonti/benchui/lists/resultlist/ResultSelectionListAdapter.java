/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import net.kishonti.swig.ResultItem;
import net.kishonti.theme.ThemeListItemViewHolder;
import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class ResultSelectionListAdapter extends BaseAdapter {

	private Context mContext;
	private ResultSelectionListDataProvider mDataProvider;
	private boolean mIsHeadered;
	

	public ResultSelectionListAdapter(Context context, ResultSelectionListDataProvider dataprovider, boolean isHeadered) {
		mContext = context;
		mDataProvider = dataprovider;
		mIsHeadered = isHeadered;
		dataprovider.getResultSelectorList(isHeadered);
	}
	
	@Override
	public int getCount() {
		return (int)mDataProvider.getResultSelectorCount();
	}

	@Override
	public Object getItem(int position) {
		return mDataProvider.getResultSelectorItemForPosition(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	@Override
	public boolean areAllItemsEnabled() {
		return false;
	}
	
	@Override
	public boolean isEnabled(int position) { 
		if(mDataProvider == null) return false;
		
		ResultItem item = mDataProvider.getResultSelectorItemForPosition(position);
		return !item.isFirstInGroup();
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View view;
		ThemeListItemViewHolder holder = new ResultListItemViewHolder(false);
		
		Object item = getItem(position);
		
		if (convertView == null) {
			view = holder.inflateAndInit(mContext, parent, item, position);
			view.setTag(holder);
			
		} else {
			view = convertView;
			holder = (ThemeListItemViewHolder) view.getTag();
			holder.refreshFromItem(item, position);
		}
		
		return view;
	}
	
	public Object getFirstSelectableItem() {
		if(getCount() < 1 || (getCount() < 2 && mIsHeadered)) return null;
		
		if(mIsHeadered) return getItem(1);
		else return getItem(0);
	}

}
