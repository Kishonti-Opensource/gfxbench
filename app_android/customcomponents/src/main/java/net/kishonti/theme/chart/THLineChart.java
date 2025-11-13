/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.theme.chart;

import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Path;
import android.graphics.Path.FillType;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;

public class THLineChart extends THGridChart {
	
    /**
     * Paths
     */
    private ArrayList<Path> mSamplePaths = new ArrayList<Path>();
    private ArrayList<RectF> mSampleClips = new ArrayList<RectF>();
	
	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THLineChart(Context context) {
		this(context, null);
	}
	
	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THLineChart(Context context, AttributeSet attr) {
		super(context, attr);
	}
	
	@Override
	protected void loadData(String dataString) {
		super.loadData(dataString);
		
		mSampleClips = new ArrayList<RectF>();
		
		for(int i = 0; i < mSampleTitles.size(); ++i) {
			mSampleClips.add(new RectF());
		}
	}
	
	@Override
	protected void updateGridRect() {
		super.updateGridRect();

		for(int i = 0; i < mSampleClips.size(); ++i) {
			if(needsAnimation)
				mSampleClips.get(i).set(mGridRect.left, mGridRect.top, mGridRect.left, mGridRect.bottom);
			else
				mSampleClips.get(i).set(mGridRect);
		}
	}
	
	@Override
	protected boolean animationUpdate(long now) {
		if(animStart == 0) animStart = now;
		
		boolean scheduleNew = false;
		for(int i = 0; i < mSampleClips.size(); ++i) {
			long current = Math.max(0, now - animStart - i*animDelay);
			float percent = Math.min((float)current/(float)animLength, 1);
			scheduleNew = percent != 1 || scheduleNew;
			
			mSampleClips.get(i).right = mGridRect.left + percent*mGridRect.width();
		}
		return scheduleNew;
	}
    
    
    
    /**
     * ------------------------ draw and layout --------------------------------
     */
    
    /**
     * Calls the layers draw functions.
     */
    @Override
    protected void dispatchDraw(Canvas canvas) {
        super.dispatchDraw(canvas);

        canvas.save();
        for(int i = 0; i < mSamplePaths.size(); ++i) {
            canvas.clipRect(mSampleClips.get(i));
        	canvas.drawPath(mSamplePaths.get(i), mSamplePaints.get(i));
        }
        canvas.restore();
    }
    
    /**
     * Relayout size dependent things. 
     */
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        
        updatePaths();
        
        invalidate();
    }
    
    private void updatePaths() {
    	try {
    		
    		mSamplePaths.clear();
    		
    		if(mData != null) {
        		
        		JSONArray domain_values = mData.getJSONObject("domain").getJSONArray("values");
            	JSONArray ranges = mData.getJSONArray("values");
            	
            	
        		for(int i = 0; i < ranges.length(); ++i) {
        			JSONArray range_values = ranges.getJSONObject(i).getJSONArray("values");
        			Path path = new Path();

                	boolean first = true;
                    int range_values_length = range_values.length() < domain_values.length() ? range_values.length() : domain_values.length();
        			for(int j = 0; j < range_values_length; ++j) {
        				float[] p = new float[]{
        						(float)domain_values.getDouble(j),
        						(float)range_values.getDouble(j)
        						};

        				float prevToCheckX = j > 0 ? (float)domain_values.getDouble(j - 1) : p[0];
        				float nextToCheckX = j < range_values_length - 1 ? (float)domain_values.getDouble(j + 1) : p[0];

        				if (nextToCheckX >= currentMin[0] &&
    						prevToCheckX <= currentMax[0]) {

            				mValuesToChartSpace.mapPoints(p);

            				if (first) {
            					path.moveTo(p[0], p[1]);
            					first = false;
            				} else {
            					path.lineTo(p[0], p[1]);
                            }
        				}
        			}
        			
        			mSamplePaths.add(path);
        		}
    		}
    		
    	} catch(JSONException e) {
			e.printStackTrace();
    	}
    }
    
    @Override
    protected void innerChartChange() {
    	super.innerChartChange();
    	updatePaths();
    }
    

//    @Override
//    @SuppressLint("ClickableViewAccessibility") 
//    public boolean onTouchEvent(MotionEvent ev) {
//    	Log.i("THLIneChart", "CICAAAA");
//    	
//    	
//	   return super.onTouchEvent(ev);
//	}
    
}
