/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.theme;

import android.content.Context;
import android.database.DataSetObserver;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ListAdapter;
import android.widget.ListView;

public class ThemeListView extends ListView {
	
	private static final String TAG = ThemeListView.class.getName();
	private DataSetObserver mDataSetObserver = new AdapterDataSetObserver();

	public ThemeListView(Context context) {
		super(context);
	}
	
	public ThemeListView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public ThemeListView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}
	

	
	
	/**
	 * -------------------------------------------------------------------------------------------
	 * Refresh only visible cells (prevent reloading all cells).
	 * This part assumes that all listItemView's tag implements the ThemeListItemView interface
	 * to be refreshable. Throws exception if used with other type of itemView tags. 
	 * -------------------------------------------------------------------------------------------
	 */
	/**
	 * A helper class that provides the missing onChanged and onInvalidated listeners for the
	 * listAdapters.
	 * @author balazs hajagos
	 */
	class AdapterDataSetObserver extends DataSetObserver {
        @Override
        public void onChanged() {
            super.onChanged();

            refreshVisibleItems();
        }

        @Override
        public void onInvalidated() {
            super.onInvalidated();

            refreshVisibleItems();
        }
    }
	
	/**
	 * Updated setAdapter implementation that registers the datasetObserver which updates
	 * the visible cells should that happen that we don't want to refresh the whole f*** list.
	 */
	@Override
    public void setAdapter(ListAdapter adapter) {
        if (getAdapter() != null) {
        	getAdapter().unregisterDataSetObserver(mDataSetObserver);
        }

        super.setAdapter(adapter);

        if(getAdapter() != null) {
            getAdapter().registerDataSetObserver(mDataSetObserver);
        }
    }
	
	/**
	 * Refreshes the visible cells in the ListView. Throws exception if the ListItemView's tag is not
	 * refreshable.
	 */
	public void refreshVisibleItems() {
        if (getAdapter() != null) {
            for (int i = getFirstVisiblePosition(); i <= getLastVisiblePosition(); i ++) {
                final int dataPosition = i - getHeaderViewsCount();
                final int childPosition = i - getFirstVisiblePosition();
                if (dataPosition >= 0 && dataPosition < getAdapter().getCount()
                        && getChildAt(childPosition) != null) {
                    Log.v(TAG, "Refreshing view (data=" + dataPosition + ",child=" + childPosition + ")");
                    
                    View v = getChildAt(childPosition);
                    if(v.getTag() instanceof ThemeListItemViewHolder) {
                    	Object item = getAdapter().getItem(dataPosition);
                    	
                    	((ThemeListItemViewHolder)v.getTag()).refreshFromItem(item, dataPosition);
                    	
                    } else {
                    	throw new ClassCastException("All ListItemViews passed to ThemeListView " +
                    			"must implement ThemeListItemView interface!");
                    }
                }
            }
        }
	}
}
