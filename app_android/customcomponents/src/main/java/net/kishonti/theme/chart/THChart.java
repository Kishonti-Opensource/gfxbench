/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.theme.chart;

import net.kishonti.customcomponents.R;
import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.animation.AnimationUtils;

public class THChart extends View {
	
	protected Context mContext;
	protected int[] value_colors;
	
	protected boolean needsAnimation = true;
	
	protected int mMaxHeight = Integer.MAX_VALUE;
	protected int mMaxWidth = Integer.MAX_VALUE;
	
	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THChart(Context context) {
		this(context, null);
	}

	public THChart(Context context, AttributeSet attr) {
		super(context, attr, R.attr.THChartStyle);
		mContext = context;
	}

    public THChart( Context context, AttributeSet attrs, int defStyle ) {
        super( context, attrs, defStyle );
    	
    	value_colors = context.getResources().getIntArray(R.array.chart_values);
    }
	
	protected boolean animationUpdate(long now) {
		return false;
	}
	
	public void setData(String dataString) {
		loadData(dataString);
		removeCallbacks(ChartAnimatior);
		post(ChartAnimatior);
	}
	
	protected void loadData(String dataString) {
		
	}

	protected Runnable ChartAnimatior = new Runnable() {
	    @Override
	    public void run() {
	        boolean scheduleNewFrame = false;
	        long now = AnimationUtils.currentAnimationTimeMillis();
	        scheduleNewFrame = animationUpdate(now);
	        if (scheduleNewFrame) {
	            postDelayed(this, 15);
	        } else {
	        	needsAnimation = false;
	        }
	        invalidate();
	    }
	};
	
	public void setMaxWidth(int max) {
		mMaxWidth = max;
		invalidate();
	}
	
	public void setMaxHeight(int max) {
		mMaxHeight = max;
		invalidate();
	}

    
}
