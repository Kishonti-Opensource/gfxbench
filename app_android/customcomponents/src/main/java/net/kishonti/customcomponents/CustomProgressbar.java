/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class CustomProgressbar extends View{
	
	Paint mBackPaint;
	Paint mFrontPaint;
	
	RectF mBackRect;
	RectF mFrontRect;
	
	int mBackColor = 0xffffffff;
	int mFrontColor = 0xff657349;
	int mPadding = 4;
	int mScrollerWidth = 30;
	
	boolean mInfinite = true;
	private static Handler mHandler;
	private Runnable mLoadingLoop;
	
	float mRelPos = 0.5f;

	public CustomProgressbar(Context context) {
		super(context);
		init();
	}
	
	public CustomProgressbar(Context context, AttributeSet attrs) {
		super(context, attrs);
		
		TypedArray a = context.getTheme().obtainStyledAttributes(
                attrs,
                R.styleable.CustomProgressbar,
                0, 0
        );
		
		try {
            // Retrieve the values from the TypedArray and store into
            // fields of this class.
			mBackColor = a.getColor(R.styleable.CustomProgressbar_backColor, 0xffffffff);
			mFrontColor = a.getColor(R.styleable.CustomProgressbar_frontColor, 0xff657349);
			mPadding = a.getDimensionPixelSize(R.styleable.CustomProgressbar_padding, 4);
			mScrollerWidth = a.getDimensionPixelSize(R.styleable.CustomProgressbar_scrollerWidth, 30);
        } finally {
            // release the TypedArray so that it can be reused.
            a.recycle();
        }

        init();
	}
	
	public CustomProgressbar(Context context, int backColor, int frontColor, int padding, int width) {
		super(context);
		
		mBackColor = backColor;
		mFrontColor = frontColor;
		mPadding = padding;
		mScrollerWidth = width;

        init();
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int widthMode = MeasureSpec.getMode(widthMeasureSpec);
		int widthSize = MeasureSpec.getSize(widthMeasureSpec);
		int heightMode = MeasureSpec.getMode(heightMeasureSpec);
		int heightSize = MeasureSpec.getSize(heightMeasureSpec);

		int maxWidth = 0;
		int maxHeight = 0;

		if (widthMode != MeasureSpec.UNSPECIFIED) {
			maxWidth = widthSize;
		}
		if (heightMode != MeasureSpec.UNSPECIFIED) {
			maxHeight = heightSize;
		}

		setMeasuredDimension(maxWidth, maxHeight);
	}
	
	@Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        
        canvas.drawRect(mBackRect, mBackPaint);
        canvas.drawRect(mFrontRect, mFrontPaint);
    }
	
	@Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        
        // Account for padding
        float xpad = (float) (getPaddingLeft() + getPaddingRight());
        float ypad = (float) (getPaddingTop() + getPaddingBottom());

        mBackRect = new RectF(getPaddingLeft(), getPaddingTop(), w - xpad, h - ypad);
        
        setInfiniteFrontRect(mRelPos);
    }
	
	/**
	 * Allows the user to set the current progress on the progressbar.
	 * @param progress Sets the progress to the given amount.
	 * @param exactProgress Set false to infinite else 
	 */
	public void onProgressChanged(double progress, boolean exactProgress) {
		if(!exactProgress) {
			if(mLoadingLoop == null) {
				StartLoop();
			}
			onProgressChanged(0.01f);
		} else {
			if(mLoadingLoop != null) {
				StopLoop();
			}
			setPercentFrontRect(progress);
			invalidate();
		}
	}
	
	/**
	 * This function animates the progressbar in an infinite loop.
	 * @param delta The relative speed of the progress mark on the [0,1].
	 */
	private void onProgressChanged(float delta) {
		mRelPos = mRelPos + delta > 1 ? 0 : mRelPos + delta;
		setInfiniteFrontRect(mRelPos);
		invalidate();
	}
	
	private void setInfiniteFrontRect(float relPos) {
		int scrollingLength = mScrollerWidth;
        int scrollingSpace = (int)mBackRect.width() - mPadding*2;
        float offset = scrollingSpace * relPos;
        float remainder = scrollingSpace - offset;
        
        if(offset < scrollingLength/2.0f) {
        	scrollingLength = (int)(mScrollerWidth/2.0f + offset);
        }
        
        if(remainder < scrollingLength/2.0f) {
        	scrollingLength = (int)(mScrollerWidth/2.0f + remainder);
        }
        
        mFrontRect = new RectF(
        		mBackRect.left + mPadding + offset - scrollingLength/2.0f, 
        		mBackRect.top + mPadding, 
        		mBackRect.left + mPadding + offset + scrollingLength/2.0f, 
        		mBackRect.bottom - mPadding);
	}
	
	private void setPercentFrontRect(double percent) {
		//Sanity check
		double realPercent = percent > 1 ? 1 : percent;
		realPercent = percent < 0 ? 0 : percent; 
				
        int scrollingSpace = (int)mBackRect.width() - mPadding*2;
        
        mFrontRect = new RectF(
        		mBackRect.left + mPadding, 
        		mBackRect.top + mPadding, 
        		mBackRect.left + mPadding + scrollingSpace * (float)realPercent, 
        		mBackRect.bottom - mPadding);
	}
	
	private void init()
	{
		mFrontRect = new RectF(0,0,0,0);
		mBackRect = new RectF(0,0,0,0);
		
		mBackPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		mBackPaint.setColor(mBackColor);
		
		mFrontPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		mFrontPaint.setColor(mFrontColor);
	}
	
	public void StartLoop() {
		mHandler = new Handler();
		mLoadingLoop = new Runnable() {
			
			@Override
			public void run() {
				onProgressChanged(0.01f);
				mHandler.postDelayed(this, 10);
			}
		};
		mHandler.post(mLoadingLoop);
	}
	
	public void StopLoop() {
		mHandler.removeCallbacks(mLoadingLoop);
		mLoadingLoop = null;
	}
}
