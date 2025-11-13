/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import net.kishonti.benchui.db.CompareSQLiteHelper.FormFactorFilter;
import net.kishonti.benchui.lists.adapters.CompareListItemViewHolder;
import net.kishonti.benchui.model.BenchmarkTestModel;
import net.kishonti.theme.ThemeListItemViewHolder;
import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

public class CompareListAdapter extends BaseAdapter {

	private Context mContext;
	private BenchmarkTestModel mDataProvider;
	
	private String resultId;
	private String filter;
	private FormFactorFilter formFilter;
	private double max;
	private String unit;
	

	public CompareListAdapter(Context context, BenchmarkTestModel dataprovider, String result_id, FormFactorFilter formFilter, String filter) {
		mContext = context;
		mDataProvider = dataprovider;
		
		this.resultId = result_id;
		this.filter = filter;
		this.formFilter = formFilter;
		
		dataprovider.getCompareList(this.resultId, this.filter, this.formFilter);

		double compMax = getCount() > 0 ? mDataProvider.getCompareItemForPosition(0).score() : 0;
		max = Math.max(mDataProvider.getBestResultForId(this.resultId).score(), compMax);
		unit = dataprovider.getBestResultForId(result_id).unit();
	}
	
	@Override
	public int getCount() {
		return (int)mDataProvider.getCompareListCount();
	}

	@Override
	public Object getItem(int position) {
		return mDataProvider.getCompareItemForPosition(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	@Override
	public boolean areAllItemsEnabled() {
		return true;
	}
	
	public void setFilter(String filter) {
		this.filter = filter;
		refresh();
	}
	
	public void setFormFilter(FormFactorFilter formFilter) {
		this.formFilter = formFilter;
		refresh();
	}
	
	public void setResultId(String resultId) {
		this.resultId = resultId;
		refresh();
	}
	
	private void refresh() {
		mDataProvider.getCompareList(this.resultId, this.filter, this.formFilter);
		double compMax = getCount() > 0 ? mDataProvider.getCompareItemForPosition(0).score() : 0;
		max = Math.max(mDataProvider.getBestResultForId(this.resultId).score(), compMax);
		notifyDataSetChanged();
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View view;
		Object item = getItem(position);
		
		ThemeListItemViewHolder holder = new CompareListItemViewHolder(max, unit);
		
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
	
	public double getMaxScore() {
		return max;
	}

}
