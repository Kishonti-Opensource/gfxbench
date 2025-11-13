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

public class BestListAdapter extends BaseAdapter {

	private Context mContext;
	private BestListDataProvider mDataProvider;
	
	public BestListAdapter(Context context, BestListDataProvider dataprovider) {
		mContext = context;
		mDataProvider = dataprovider;
		dataprovider.getBestList();
	}
	
	@Override
	public int getCount() {
		return (int)mDataProvider.getBestCount();
	}

	@Override
	public Object getItem(int position) {
		return mDataProvider.getBestResultForPosition(position);
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
		
		ResultItem item = mDataProvider.getBestResultForPosition(position);
		return !item.isFirstInGroup();
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View view;
		ThemeListItemViewHolder holder = new ResultListItemViewHolder();
		
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

}
