/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.ViewGroup;

public class GridLayout extends ViewGroup {

	private static final int DEFAULT_COUNT = 1;
	private static final int ROW_COUNT = R.styleable.GridLayout_rowCount;
	private static final int COLUMN_COUNT = R.styleable.GridLayout_columnCount;
	private int mCols;
	private int mRows;

	public GridLayout(Context context) {
		super(context);
		init(context, null);
	}

	public GridLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
		init(context, attrs);
	}

	public GridLayout(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		init(context, attrs);
	}

	private void init(Context context, AttributeSet attrs) {
		mCols = DEFAULT_COUNT;
		mRows = DEFAULT_COUNT;
		if (attrs != null) {
	        TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.GridLayout);
            try {
                setRowCount(a.getInt(ROW_COUNT, DEFAULT_COUNT));
                setColumnCount(a.getInt(COLUMN_COUNT, DEFAULT_COUNT));
            } finally {
                a.recycle();
            }
		}
	}

	void setColumnCount(int cols) {
		mCols = cols;
	}

	void setRowCount(int rows) {
		mRows = rows;
	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		int childCount = getChildCount();
		int w = r - l;
		int h = b - t;
		int dx = Math.round(w / (float)mCols);
		int dy = Math.round(h / (float)mRows);
		int i = 0;
		for (int y = 0; y < mRows; ++y) {
			for (int x = 0; x < mCols; ++x) {
				if (i < childCount) {
					getChildAt(i).layout(
						l + x*dx, t + (y)*dy,
						l + (x+1)*dx, t + (y+1)*dy
					);
					++i;
				} else {
					break;
				}
			}
		}
	}
}
