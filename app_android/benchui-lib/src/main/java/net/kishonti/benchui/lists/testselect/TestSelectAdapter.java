/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.testselect;

import net.kishonti.swig.TestItem;
import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class TestSelectAdapter extends BaseAdapter {
	
	private Context mContext;
	private TestListDataProvider mDataProvider;
	
	public TestSelectAdapter(Context context, TestListDataProvider dataprovider) {
		mContext = context;
		mDataProvider = dataprovider;
		dataprovider.getTestList();
	}

	@Override
	public int getCount() {
		return (int)mDataProvider.getTestCount();
	}

	@Override
	public Object getItem(int position) {
		return mDataProvider.getTestForPosition(position);
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
		
		TestItem item = mDataProvider.getTestForPosition(position);
		return item != null && (item.isAvailable() || item.isFirstInGroup());
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View view;
		TestSelectListItemViewHolder holder = new TestSelectListItemViewHolder();
		
		Object item = getItem(position);
		
		if (convertView == null) {
			view = holder.inflateAndInit(mContext, parent, item, position);
			view.setTag(holder);
			
		} else {
			view = convertView;
			holder = (TestSelectListItemViewHolder) view.getTag();
			holder.refreshFromItem(item, position);
		}
		
		return view;
	}
	
	@Override
	public void notifyDataSetChanged () {
		mDataProvider.getTestList();
		super.notifyDataSetChanged();
	}

}
