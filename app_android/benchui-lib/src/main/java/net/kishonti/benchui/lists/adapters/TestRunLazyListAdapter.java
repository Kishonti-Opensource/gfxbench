/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.adapters;

import java.text.DateFormat;
import java.util.Date;
import net.kishonti.benchui.R;
import net.kishonti.benchui.Utils;
import net.kishonti.benchui.model.BenchmarkTestModel;
import net.kishonti.swig.Session;
import net.kishonti.swig.SessionVector;
import net.kishonti.theme.Localizator;
//import gfxr.dao.Session;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListAdapter;
import android.widget.TextView;

public class TestRunLazyListAdapter extends BaseAdapter implements ListAdapter {
	protected boolean mDataValid;
    protected SessionVector mTheList;
    protected Context mContext;
	
	public TestRunLazyListAdapter(Context context, SessionVector list) {
		mContext = context;
		mTheList = list;
		mDataValid = list != null;
	}
	
	@Override
	public boolean areAllItemsEnabled() {
		return true;
	}

	@Override
	public boolean isEnabled(int position) {
		return true;
	}
	
	public View newView(Context context,  Session item, ViewGroup parent) {
		if (item == null) return null;
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		
		HistoryCellContainer c = new HistoryCellContainer();
//		View v = inflater.inflate(R.layout.cell_history, parent, false);
		View v = inflater.inflate(Utils.getLayoutId(context, "cell_history"), parent, false);
		c.mTitle = (TextView) v.findViewById(R.id.cell_history_title);
		v.setTag(c);
		
		return v;
	}
	
	public void bindView(View view, Context context, Session item) {
		HistoryCellContainer c = (HistoryCellContainer) view.getTag();
//		if(item.getDescriptor().equals("Best")) {
		if(item.sessionId() == BenchmarkTestModel.BEST_SESSION_ID) {
			c.mTitle.setText(Localizator.getString(context, "BestResults"));
		} else {
			c.mTitle.setText(DateFormat.getDateTimeInstance().format(new Date(item.sessionId())).toString());
		}
	}
	
	private class HistoryCellContainer {
		public TextView mTitle;
	}

	@Override
	public int getCount() {
        if (mDataValid && mTheList != null) {
            return (int)mTheList.size();
        } else {
            return 0;
        }
	}

	@Override
	public Object getItem(int position) {
        if (mDataValid && mTheList != null) {
            return mTheList.get(position);
        } else {
            return null;
        }
	}

	@Override
	public long getItemId(int position) {
        if (mDataValid && mTheList != null) {
        	return position;
        } else {
            return 0;
        }
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
        if (!mDataValid) {
            throw new IllegalStateException("this should only be called when lazylist is populated");
        }
        
        Session item = mTheList.get(position);
        if (item == null) {
            throw new IllegalStateException("Item at position " + position + " is null");
        }
        
        View v;
        if (convertView == null) {
            v = newView(mContext, item, parent);
        } else {
            v = convertView;
        }
        bindView(v, mContext, item);
        return v;
	}
}	