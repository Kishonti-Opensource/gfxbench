/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.theme.chart;

import java.util.ArrayList;

import net.kishonti.customcomponents.R;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Path;
import android.graphics.Path.Direction;
import android.graphics.Path.FillType;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.ColorDrawable;
import android.net.UrlQuerySanitizer.ValueSanitizer;
import android.support.v4.view.MotionEventCompat;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.MotionEvent;

@SuppressLint("DefaultLocale") 
public class THPieAreaChart extends THChart {
	private static final Boolean DRAW_DEBUG = true;
    
	//TODO from custom attributes
	private long animStart = 0;
	private long animDelay = 0;
	private long animLength = 0;
	
	private float mTextOffset = 0;
	
	protected final Rect drawedRect = new Rect();
	
	protected JSONObject mData;
    
    protected static class SliceData {
    	public float mValue = 0;
    	public float mScale = 0;
    	public Paint mMediumPaint;
    	public Paint mDarkPaint;
    	public Paint mLightPaint; 
    	public LinearGradient mGradient;
    	public String mName = "";
    	
    	public SliceData(Resources res, float value, String name, int color) {
    		mValue = value;
    		mName = name;
    		
    		mMediumPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    		mMediumPaint.setColor(color);
    		mMediumPaint.setStyle(Paint.Style.FILL_AND_STROKE);
    		mMediumPaint.setTextSize(
        			TypedValue.applyDimension(
        					TypedValue.COMPLEX_UNIT_SP,
        					10.0f, 
        					res.getDisplayMetrics()));
    		mMediumPaint.setTextAlign(Align.CENTER);
    		
    		float[] hsv = new float[3];
    		Color.colorToHSV(color, hsv);
    		
    		hsv[2] = hsv[2] * 0.8f;
    		
    		mDarkPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    		mDarkPaint.setColor(Color.HSVToColor(hsv));
    		mDarkPaint.setStyle(Paint.Style.FILL_AND_STROKE);
    		
    		Color.colorToHSV(color, hsv);
    		hsv[1] = hsv[1] * 0.7f;
    		mLightPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    		mLightPaint.setColor(Color.HSVToColor((int)(255 * 0.2f), hsv));
    		mLightPaint.setStyle(Paint.Style.FILL_AND_STROKE);
    	}
    	
    	public Paint getShadowedPaint(float fromX, float fromY, float toX, float toY) {
    		mDarkPaint.setShader(new LinearGradient(fromX, fromY, toX, toY, mMediumPaint.getColor(), mDarkPaint.getColor(), android.graphics.Shader.TileMode.CLAMP));
    		return mDarkPaint;
    	}
    }
    protected ArrayList<SliceData> mSlices = new ArrayList<SliceData>();
	
	/**
	 * Colors
	 */
	private int mBackgroundColor;
	
	/**
	 * Paints
	 */
    private Paint mBackgroundPaint;
    
    /**
     * Paths
     */
    private Path mSeparatorPath = new Path();
    
	
	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THPieAreaChart(Context context) {
		this(context, null);
	}
	
	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THPieAreaChart(Context context, AttributeSet attrs) {
		this(context, attrs, R.attr.THPieAreaChartStyle);
	}
	
	/**
	 * Default constructor.
	 * @param context Context to create a control.
	 */
	public THPieAreaChart(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		
		final TypedArray array = context.obtainStyledAttributes( 
				attrs, R.styleable.THPieAreaChart, defStyle, R.style.THPieAreaChart );
		
		
		try {
        	Resources res = getContext().getResources();
        	
            // Retrieve the values from the TypedArray and store into
            // fields of this class.
        	if(getBackground().getClass().equals(ColorDrawable.class))
        		mBackgroundColor = ((ColorDrawable)getBackground()).getColor();
        	else
        		mBackgroundColor = res.getColor(R.color.chart_background);
    	
        	animDelay = array.getInteger(R.styleable.THPieAreaChart_pieareachartAnimDelay, 150);
        	animLength = array.getInteger(R.styleable.THPieAreaChart_pieareachartAnimLength, 800);
        } finally {
            // release the TypedArray so that it can be reused.
            array.recycle();
        }
		
    	mBackgroundPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    	mBackgroundPaint.setColor(mBackgroundColor);
    	mBackgroundPaint.setStyle(Paint.Style.STROKE);
    	mBackgroundPaint.setStrokeWidth(4.0f);
	}
	
	@Override
	protected void loadData(String dataString) {
		try {
			JSONObject data = new JSONObject(dataString);
			mData = data;
			
			mSlices.clear();
			
			JSONArray values = data.getJSONArray("values");
			for(int i = 0; i < values.length(); ++i) {
				JSONObject object = values.getJSONObject(i);
				mSlices.add(new SliceData(
						getResources(),
						(float)object.getJSONArray("values").getDouble(0), 
						object.getString("name"), 
						value_colors[i%20]));
			}
			
		} catch (JSONException e) {
			e.printStackTrace();
		}
		
	}
    
    
    
    /**
     * ------------------------ draw and layout --------------------------------
     */

    /**
     * Do nothing. Do not call the superclass method--that would start a layout pass
     * on this view's children. This lays out its children in onSizeChanged().
     */
    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        // Do nothing. Do not call the superclass method--that would start a layout pass
        // on this view's children. This lays out its children in onSizeChanged().
    }
    
    /**
     * Calls the layers draw functions.
     */
    @Override
    protected void dispatchDraw(Canvas canvas) {
        super.dispatchDraw(canvas);
        canvas.save();
        
        getPieRect(drawedRect);
        RectF r = new RectF(drawedRect);
        drawPieSlices(canvas, r);
        canvas.drawPath(mSeparatorPath, mBackgroundPaint);
        getDrawingRect(drawedRect);
        r = new RectF(drawedRect);
        drawLabels(canvas, r);

        if(DRAW_DEBUG) {
        }
        canvas.restore();
    }
    
    /**
 	 * Measurement functions. 
     */
    @Override
    protected int getSuggestedMinimumWidth() {
        return (int) 200;
    }

    @Override
    protected int getSuggestedMinimumHeight() {
        return (int) 200;
    }
    
    /**
     * Measures the required space for the control.
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int minw = getPaddingLeft() + getPaddingRight() + getSuggestedMinimumWidth();
        int w = Math.max(minw, MeasureSpec.getSize(widthMeasureSpec));
        
        int minh = getSuggestedMinimumHeight() + getPaddingBottom() + getPaddingTop();
        int h = Math.max(minh, MeasureSpec.getSize(heightMeasureSpec));
        
        w = Math.min(w, h);

        setMeasuredDimension(w, w);
    }
    
    /**
     * Relayout size dependent things. 
     */
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        
    	if(mSlices.size() > 0) {
        	SliceData data = mSlices.get(0);
        	String dummy = "sumMug sit";
            data.mMediumPaint.getTextBounds(dummy, 0, dummy.length(), drawedRect);
            mTextOffset = Math.max(drawedRect.width()*0.5f, drawedRect.height()*0.5f);
    	}
    	
    	
    	getPieRect(drawedRect);
    	mSeparatorPath.reset();
    	for(int i = 0; i < mSlices.size(); ++i) {
    		float deg = -(360.0f/mSlices.size() * i - 90);
            float cos = (float)Math.cos(deg/360.0f * Math.PI*2);
            float sin = (float)Math.sin(deg/360.0f * Math.PI*2);
            
            mSeparatorPath.moveTo(drawedRect.centerX(), drawedRect.centerY());
            mSeparatorPath.lineTo(
            		(float)(drawedRect.centerX() + drawedRect.width() * 0.5 * cos), 
            		(float)(drawedRect.centerY() - drawedRect.width() * 0.5 * sin));
    	}
    	
        invalidate();
        super.onSizeChanged(w, h, oldw, oldh);
    }
    
    protected void drawPieSlices(Canvas canvas, RectF rect) {
        canvas.save();
        RectF clippedRect = new RectF();
        
        float slices = mSlices.size();
        for (int i = 0; i < slices; ++i) {
        	SliceData slice = mSlices.get(i);
        	clippedRect.set(
        			rect.centerX() - rect.width() * 0.5f * slice.mValue * slice.mScale,
        			rect.centerY() - rect.height() * 0.5f * slice.mValue * slice.mScale,
        			rect.centerX() + rect.width() * 0.5f * slice.mValue * slice.mScale,
        			rect.centerY() + rect.height() * 0.5f * slice.mValue * slice.mScale);
        	
            canvas.drawArc(rect, 360.0f/slices * i - 90, 360.0f/slices, true, slice.mLightPaint);
            
            float deg = -(360.0f/slices * (i+0.5f) - 90);
            float cos = (float)Math.cos(deg/360.0f * Math.PI*2);
            float sin = (float)Math.sin(deg/360.0f * Math.PI*2);
            canvas.drawArc(clippedRect, 360.0f/slices * i - 90, 360.0f/slices, true,
            		slice.getShadowedPaint(
            				clippedRect.centerX() + cos*(clippedRect.width() * 0.5f), 
            				clippedRect.centerY() - sin*(clippedRect.width() * 0.5f),
            				clippedRect.centerX(), 
            				clippedRect.centerY())
            );
		}
        
        canvas.restore();
    }
    
    protected void drawLabels(Canvas canvas, RectF rect) {
        canvas.save();
        
        float slices = mSlices.size();
        for (int i = 0; i < slices; ++i) {
    		float deg = -(360.0f/mSlices.size() * (i + 0.5f) - 90);
            float cos = (float)Math.cos(deg/360.0f * Math.PI*2);
            float sin = (float)Math.sin(deg/360.0f * Math.PI*2);
            
            SliceData data = mSlices.get(i);
            String label = data.mName;
            data.mMediumPaint.getTextBounds(label, 0, label.length(), drawedRect);
            
            float textCenterX = rect.centerX() + rect.width()*0.5f*cos;
            float textCenterY = rect.centerY() - rect.width()*0.5f*sin;
            textCenterX = Math.min(textCenterX, rect.centerX() + rect.width()*0.5f - (drawedRect.width() + 4.0f)*0.5f);
            textCenterX = Math.max(textCenterX, rect.centerX() - rect.width()*0.5f + (drawedRect.width() + 4.0f)*0.5f);
            textCenterY = Math.min(textCenterY, rect.centerY() + rect.height()*0.5f - (drawedRect.height() + 4.0f));
            textCenterY = Math.max(textCenterY, rect.centerY() - rect.height()*0.5f + (drawedRect.height() + 4.0f));
            
            
            // Get text height with text that has tall and under baseline characters too 
            data.mMediumPaint.getTextBounds("Mg", 0, "Mg".length(), drawedRect);
            canvas.drawText(
            		label, 
            		textCenterX, 
            		textCenterY - (drawedRect.height() + 4.0f)*0.5f, 
            		data.mMediumPaint);
            canvas.drawText(
            		"" + data.mValue*100.0f + "%", 
            		textCenterX, 
            		textCenterY + (drawedRect.height() + 8.0f)*0.5f, 
            		data.mMediumPaint);
		}
        
        canvas.restore();
    }
    

	@Override
	protected boolean animationUpdate(long now) {
		if(animStart == 0) animStart = now;
		
		boolean scheduleNew = false;
		for(int i = 0; i < mSlices.size(); ++i) {
			long current = Math.max(0, now - animStart - (i+1)*animDelay);
			float percent = Math.min((float)current/(float)animLength, 1);
			scheduleNew = percent != 1 || scheduleNew;
			
			//TODO hardcoded cubic easing inout
			percent *=2;
			if(percent < 1) {
				mSlices.get(i).mScale = 0.5f*percent*percent*percent;
			} else {
				percent -= 2;
				mSlices.get(i).mScale = 0.5f*(percent*percent*percent + 2);
			}
			
		}
		return scheduleNew;
	}
	
	protected void getPieRect(Rect outRect) {
        getDrawingRect(outRect);
        outRect.inset((int)mTextOffset*2, (int)mTextOffset*2);
	}
    
}
