/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Path.FillType;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SoundEffectConstants;
import android.widget.RelativeLayout;

public class CirclesControl extends RelativeLayout {
	
	private enum CirclesControlState {
		NO_FOCUS,
		MAIN_FOCUS,
		SIDE_FOCUS
	}
	
	private float mDefaultMainSize = 760f;
	
	private float mMainSize = 760.0f;
	private float mMainRadiusPercent = 250.0f/mMainSize;
	private float mSideRadiusPercent = 138.0f/mMainSize;
	private float mPaddingPercent = 40.0f/mMainSize;
	private float mXDistancePercent = 292.0f/mMainSize;
	private float mFullRequiredPercent = 940.0f/mMainSize;
	private float mMainRadius = mMainSize * mMainRadiusPercent;
	private float mSideRadius = mMainSize * mSideRadiusPercent;
	private float mPadding = mMainSize * mPaddingPercent;
	private float mXDistance = mMainSize * mXDistancePercent;
	
	private PointF mMainCenter;
	private PointF mSideCenter;
	private PointF mTangentMain_1;
	private PointF mTangentMain_2;
	private PointF mTangentSide_1;
	private PointF mTangentSide_2;
	
	private StaticLayout mMainTextLayout;
	private	StaticLayout mSideTextLayout;
	private RectF mMainTextBounds;
	private RectF mSideTextBounds;
	
	private boolean mHasTouch;

    private Paint mMainPaint;
    private Paint mSidePaint;
    private Paint mPaddingPaint;
    private TextPaint mMainTextPaint;
    private TextPaint mSideTextPaint;
    
    private Path mPaddingPath;
    
    private int mMainColor;
    private int mSideColor;
    private int mMainActiveColor;
    private int mSideActiveColor;
    private int mPaddingColor;
    private int mMainTextColor;
    private int mSideTextColor;
    
    private String mMainText;
    private String mSideText;
    
    private Runnable mMainTouchHandler;
    private Runnable mSideTouchHandler0;
    
    private CirclesControlState mState;
    
    private void setMainSize(float size) {
    	mMainSize = size;
    	mMainRadius = mMainSize * mMainRadiusPercent;
    	mSideRadius = mMainSize * mSideRadiusPercent;
    	mPadding = mMainSize * mPaddingPercent;      
    	mXDistance = mMainSize * mXDistancePercent;  
    }
	
	public CirclesControl(Context context) {
		super(context);
		init();
	}
	
	/**
	 * Creates a CircelsControl with given parameters.
	 * @param context Context to create a control.
	 * @param mainColor Color of the main button. 
	 * @param sideColor Color of the side button.
	 * @param paddingColor Color of the background and padding.
	 * @param mainTextColor Color of the main button's text.
	 * @param sideTextColor Color of the side buttons text.
	 * @param mainText Text of the main button.
	 * @param sideText Text of the side button.
	 */
	public CirclesControl(Context context, int mainColor, int sideColor, int mainActiveColor, int sideActiveColor, int paddingColor, int mainTextColor, int sideTextColor, String mainText, String sideText) {
		super(context);

        mMainColor = mainColor;
        mSideColor = sideColor;
        mMainActiveColor = mainActiveColor;
        mSideActiveColor = sideActiveColor;
        mPaddingColor = paddingColor;
        mMainTextColor = mainTextColor;
        mSideTextColor = sideTextColor;
        mMainText = mainText;
        mSideText = sideText;
        
		init();
	}

	public CirclesControl(Context context, AttributeSet attrs) {
		super(context, attrs);
		
		TypedArray a = context.getTheme().obtainStyledAttributes(
                attrs,
                R.styleable.CirclesControl,
                0, 0
        );

        try {
        	Resources res = getContext().getResources();
        	
            // Retrieve the values from the TypedArray and store into
            // fields of this class.
            mMainColor = a.getColor(R.styleable.CirclesControl_CC_main_color, res.getColor(R.color.startbutton_background));
            mSideColor = a.getColor(R.styleable.CirclesControl_CC_side_color, res.getColor(R.color.selectbutton_background));
            mMainActiveColor = a.getColor(R.styleable.CirclesControl_CC_main_active_color, res.getColor(R.color.startbutton_active_background));
            mSideActiveColor = a.getColor(R.styleable.CirclesControl_CC_side_active_color, res.getColor(R.color.selectbutton_active_background));
            mPaddingColor = a.getColor(R.styleable.CirclesControl_CC_padding_color, res.getColor(R.color.mainbuttons_background));
            mMainTextColor = a.getColor(R.styleable.CirclesControl_CC_main_text_color, res.getColor(R.color.startbutton_text));
            mSideTextColor = a.getColor(R.styleable.CirclesControl_CC_side_text_color, res.getColor(R.color.selectbutton_text));
            mMainText = a.getString(R.styleable.CirclesControl_CC_main_text);
            mSideText = a.getString(R.styleable.CirclesControl_CC_side_text);
        } finally {
            // release the TypedArray so that it can be reused.
            a.recycle();
        }

        init();
	}
	
	/**
	 * Sets the touch handler the main button of the control.
	 * @param r The handler.
	 */
	public void setMainOnTouchHandler(Runnable r) {
		mMainTouchHandler = r;
	}
	
	/**
	 * Sets the touch handler for the first side button of the control.
	 * @param r The handler.
	 */
	public void setSideOnTouchHandler0(Runnable r) {
		mSideTouchHandler0 = r;
	}
	
	/**
	 * Sets the string for the main button.
	 * @param s The desired label of the main button.
	 */
	public void setMainText(String s) {
		mMainText = s;
		layoutText();
		invalidate();
	}
	
	/**
	 * Sets the string for the side button.
	 * @param s The desired label of the side button.
	 */
	public void setSideText(String s) {
		mSideText = s;
		layoutText();
		invalidate();
	}

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        // Do nothing. Do not call the superclass method--that would start a layout pass
        // on this view's children. This lays out its children in onSizeChanged().
    }
    
    @Override
    protected void dispatchDraw(Canvas canvas) {
        super.dispatchDraw(canvas);
        
        canvas.drawCircle(mMainCenter.x, mMainCenter.y, mMainRadius + mPadding, mPaddingPaint);
        canvas.drawCircle(mSideCenter.x, mSideCenter.y, mSideRadius + mPadding, mPaddingPaint);
        
        canvas.drawPath(mPaddingPath, mPaddingPaint);
        
        canvas.drawCircle(mMainCenter.x, mMainCenter.y, mMainRadius, mMainPaint);
        canvas.drawCircle(mSideCenter.x, mSideCenter.y, mSideRadius, mSidePaint);
        
        canvas.save();
        canvas.translate(mMainTextBounds.left, mMainTextBounds.top);
        mMainTextLayout.draw(canvas);
        canvas.restore();
        
        canvas.save();
        canvas.translate(mSideTextBounds.left, mSideTextBounds.top);
        mSideTextLayout.draw(canvas);
        canvas.restore();
    }
    
    //
    // Measurement functions. This example uses a simple heuristic: it assumes that
    // the pie chart should be at least as wide as its label.
    //
    @Override
    protected int getSuggestedMinimumWidth() {
        return (int) 100;
    }

    @Override
    protected int getSuggestedMinimumHeight() {
        return (int) 100;
    }
    
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        // Try for a width based on our minimum
        int minw = getPaddingLeft() + getPaddingRight() + getSuggestedMinimumWidth();

        int w = Math.max(minw, MeasureSpec.getSize(widthMeasureSpec));

        // Whatever the width ends up being, ask for a height that would let the pie
        // get as big as it can
        int minh = getSuggestedMinimumHeight() + getPaddingBottom() + getPaddingTop();
        int h = Math.max(MeasureSpec.getSize(heightMeasureSpec), minh);

        setMeasuredDimension(w, h);
    }
    
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        
        setMainSize(mDefaultMainSize);
        
        // Account for padding
        float xpad = (float) (getPaddingLeft() + getPaddingRight());
        float ypad = (float) (getPaddingTop() + getPaddingBottom());
        
        //Check the available size
        RectF mainRect = new RectF(getPaddingLeft(), getPaddingTop(), w - xpad, h - ypad);
        float x = mMainSize * mFullRequiredPercent;
        if(x > mainRect.width()) {
        	x = mainRect.width();
        	setMainSize(x / mFullRequiredPercent);
        }
        float y = mMainSize;
        if(y > mainRect.height()) {
        	y = mainRect.height();
        	x = y * mFullRequiredPercent;
        	setMainSize(y);
        }
        
        float offsetX = getPaddingLeft() + (mainRect.width() - x) / 2.0f + (x - mMainSize);
        float offsetY = getPaddingTop() + (mainRect.height() - y) / 2.0f;
        
        RectF finalRect = new RectF(offsetX, offsetY, mMainSize, mMainSize);
        
        mMainCenter = new PointF(finalRect.left + mPadding + mMainRadius, finalRect.top + mPadding + mMainRadius);
        mSideCenter = new PointF(finalRect.left + mPadding + mMainRadius + mXDistance, finalRect.top + mPadding + mMainRadius + mXDistance);
        
        //compute the tangents
        float R1 = mMainRadius + mPadding;
        float R2 = mSideRadius + mPadding;
        double D = Math.sqrt((mSideCenter.x - mMainCenter.x) * (mSideCenter.x - mMainCenter.x) + (mSideCenter.y - mMainCenter.y) * (mSideCenter.y - mMainCenter.y));
        double H = Math.sqrt(D*D + (R1 - R2) * (R1 - R2));
        double Y = Math.sqrt(H*H + R2 * R2);
        double theta = Math.acos((R1 * R1 + D*D - Y*Y) / (2 * R2 * D));
        float cs1 = (float)Math.cos(theta);
        float sn1 = (float)Math.sin(theta);
        float cs2 = (float)Math.cos(-theta);
        float sn2 = (float)Math.sin(-theta);
        
        PointF vecMiddle = new PointF(mSideCenter.x - mMainCenter.x, mSideCenter.y - mMainCenter.y);
        vecMiddle = new PointF(vecMiddle.x/vecMiddle.length(), vecMiddle.y/vecMiddle.length());
        PointF vecTan1 = new PointF(vecMiddle.x * cs1 - vecMiddle.y * sn1, vecMiddle.x * sn1 + vecMiddle.y * cs1);
        PointF vecTan2 = new PointF(vecMiddle.x * cs2 - vecMiddle.y * sn2, vecMiddle.x * sn2 + vecMiddle.y * cs2);

        mTangentMain_1 = new PointF(mMainCenter.x + vecTan1.x * R1, mMainCenter.y + vecTan1.y * R1);
        mTangentMain_2 = new PointF(mMainCenter.x + vecTan2.x * R1, mMainCenter.y + vecTan2.y * R1);
        mTangentSide_1 = new PointF(mSideCenter.x + vecTan1.x * R2, mSideCenter.y + vecTan1.y * R2);
        mTangentSide_2 = new PointF(mSideCenter.x + vecTan2.x * R2, mSideCenter.y + vecTan2.y * R2);
        
        mPaddingPath = new Path();
        mPaddingPath.setFillType(FillType.EVEN_ODD);
        mPaddingPath.moveTo(mTangentMain_1.x, mTangentMain_1.y);
        mPaddingPath.lineTo(mTangentSide_1.x, mTangentSide_1.y);
        mPaddingPath.lineTo(mTangentSide_2.x, mTangentSide_2.y);
        mPaddingPath.lineTo(mTangentMain_2.x, mTangentMain_2.y);
        mPaddingPath.lineTo(mTangentMain_1.x, mTangentMain_1.y);
        mPaddingPath.close();
        
        layoutText();
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) {
    	
    	float x = event.getX();
        float y = event.getY();
        
        PointF mainToTouch = new PointF(mMainCenter.x - x, mMainCenter.y - y);
        PointF side0ToTouch = new PointF(mSideCenter.x - x, mSideCenter.y - y);
        boolean isNearMainBtn = mainToTouch.length() <= mMainRadius;
        boolean isNearSideButton0 = side0ToTouch.length() <= mSideRadius;
    	
        switch (event.getAction()) {
	        case MotionEvent.ACTION_DOWN:
	        	if(isNearMainBtn) {
	        		mHasTouch = true;
	        		focusMainBtn();
	        		return true;
	        	} else if (isNearSideButton0) {
	        		mHasTouch = true;
	        		focusSideBtn();
	        		return true;
	        	}
	        	break;
	        case MotionEvent.ACTION_UP:
	        	if(isNearMainBtn) {
	        		focusMainBtn();
	        		performClick();
	        		removeFocus();
	        		mHasTouch = false;
	        		return true;
	        	} else if (isNearSideButton0) {
	        		focusSideBtn();
	        		performClick();
	        		removeFocus();
	        		mHasTouch = false;
	        		return true;
	        	} else if(mHasTouch) {
	        		removeFocus();
	        		mHasTouch = false;
	        		return false;
	        	}
	        	break;
	        case MotionEvent.ACTION_MOVE:
	        	if(isNearMainBtn) {
	        		mSidePaint.setColor(mSideColor);
	        		invalidate();
	        		return true;
	        	} else if(isNearSideButton0) {
	        		mMainPaint.setColor(mMainColor);
	        		invalidate();
	        		return true;
	        	} else if(mHasTouch) {
	        		removeFocus();
	        		mHasTouch = false;
	        		return false;
	        	}
	        	break;
	        default:
        		removeFocus();
	    		mHasTouch = false;
	    		return false;
        }
    	
    	return false;
    }
    
    private void init() {

		mMainPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		mMainPaint.setColor(mMainColor);
		
		mSidePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		mSidePaint.setColor(mSideColor);
		
		mPaddingPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		mPaddingPaint.setStyle(Paint.Style.FILL_AND_STROKE);
		mPaddingPaint.setColor(mPaddingColor);
		
		mMainTextPaint = new TextPaint(Paint.ANTI_ALIAS_FLAG);
		mMainTextPaint.setColor(mMainTextColor);
		mMainTextPaint.setTextSize(30);
		mMainTextPaint.setTextAlign(Paint.Align.LEFT);

		mSideTextPaint = new TextPaint(Paint.ANTI_ALIAS_FLAG);
		mSideTextPaint.setColor(mSideTextColor);
		mSideTextPaint.setTextSize(16);
		mSideTextPaint.setTextAlign(Paint.Align.LEFT);
		
		setClickable(true);
		setLongClickable(false);
		setFocusable(true);
		setFocusableInTouchMode(false);
		
		mState = CirclesControlState.NO_FOCUS;
    }
    
    /** 
     * Resize the font so the specified text fits in the text box
     * assuming the text box is the specified width.
     */
    private void refitText(String text, int textWidth, TextPaint originalPaint) 
    { 
        if (textWidth <= 0)
            return;
        int targetWidth = textWidth - this.getPaddingLeft() - this.getPaddingRight();
        float hi = 100;
        float lo = 2;
        final float threshold = 0.5f; // How close we have to be

        while((hi - lo) > threshold) {
            float size = (hi+lo)/2;
            originalPaint.setTextSize(size);
            if(originalPaint.measureText(text) >= targetWidth) 
                hi = size; // too big
            else
                lo = size; // too small
        }
        // Use lo so that we undershoot rather than overshoot
        //this.setTextSize(TypedValue.COMPLEX_UNIT_PX, lo);
        originalPaint.setTextSize(lo);
    }
    
    private void layoutText() {
    	int mainMultiplier = 1 + (int) Math.max(1, Math.min(Math.floor(mMainText.length()/6), 4));
    	refitText(mMainText, (int)((mMainRadius - mPadding*2) * mainMultiplier), mMainTextPaint);
    	
    	int sideMultiplier = 1 + (int) Math.max(1, Math.min(Math.floor(mSideText.length()/6), 4));
    	refitText(mSideText, (int)((mSideRadius - mPadding) * sideMultiplier), mSideTextPaint);
        
        mMainTextLayout = new StaticLayout(mMainText, (TextPaint) mMainTextPaint, (int)(mMainRadius-mPadding)*2, Layout.Alignment.ALIGN_CENTER, 1.0f, 0.0f, false);
        mSideTextLayout = new StaticLayout(mSideText, (TextPaint) mSideTextPaint, (int)(mSideRadius-mPadding)*2, Layout.Alignment.ALIGN_CENTER, 1.0f, 0.0f, false);
        
        if(mMainCenter != null && mSideCenter != null) {
        	mMainTextBounds = new RectF(mMainCenter.x - mMainRadius + mPadding, mMainCenter.y - mMainTextLayout.getHeight()/2.0f, mMainCenter.x + mMainRadius - mPadding, mMainCenter.y + mMainTextLayout.getHeight()/2.0f);
            mSideTextBounds = new RectF(mSideCenter.x - mSideRadius + mPadding, mSideCenter.y - mSideTextLayout.getHeight()/2.0f, mSideCenter.x + mSideRadius - mPadding, mSideCenter.y + mSideTextLayout.getHeight()/2.0f);
        }
    }
    
    
    
    @Override
    protected void onFocusChanged(boolean gainFocus, int direction,
    		Rect previouslyFocusedRect) {
    	super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
    	
    	if(gainFocus) {
    		if(mState == CirclesControlState.NO_FOCUS) {
    			if(direction == FOCUS_DOWN || direction == FOCUS_RIGHT 
    					|| direction == FOCUS_FORWARD) {
    				focusMainBtn();
    			}
    			
    			if(direction == FOCUS_UP || direction == FOCUS_LEFT 
    					|| direction == FOCUS_BACKWARD) {
    				focusSideBtn();
    			}
    		}
    		
    		if(mState == CirclesControlState.MAIN_FOCUS 
    				&& (direction == FOCUS_DOWN || direction == FOCUS_RIGHT 
							|| direction == FOCUS_FORWARD)) {
    			focusSideBtn();
    		}
    		
    		if(mState == CirclesControlState.SIDE_FOCUS 
    				&& (direction == FOCUS_UP || direction == FOCUS_LEFT 
        					|| direction == FOCUS_BACKWARD)) {
    			focusMainBtn();
    		}
    	} else {
    		removeFocus();
    	}
    }
    
    private void focusMainBtn() {
    	mMainPaint.setColor(mMainActiveColor);
    	mSidePaint.setColor(mSideColor);
    	mState = CirclesControlState.MAIN_FOCUS;
    	invalidate();
    }
    
    private void focusSideBtn() {
    	mMainPaint.setColor(mMainColor);
    	mSidePaint.setColor(mSideActiveColor);
    	mState = CirclesControlState.SIDE_FOCUS;
    	invalidate();
    }
    
    private void removeFocus() {
    	mMainPaint.setColor(mMainColor);
    	mSidePaint.setColor(mSideColor);
    	mState = CirclesControlState.NO_FOCUS;
    	invalidate();
    }
    
    
    @Override
    public boolean performClick() {
    	if(mState == CirclesControlState.MAIN_FOCUS) {
    		if(mMainTouchHandler != null) {
    			mMainTouchHandler.run();
    		}
    		super.performClick();
    		return true;
    	} else
    	if(mState == CirclesControlState.SIDE_FOCUS) {
    		if(mSideTouchHandler0 != null) {
    			mSideTouchHandler0.run();
    		}
    		super.performClick();
    		return true;
    	}
    	return false;
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	if(mState == CirclesControlState.MAIN_FOCUS) {
        	if(keyCode == KeyEvent.KEYCODE_DPAD_DOWN || keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
        		return true;
        	}
    	}
    	
    	if(mState == CirclesControlState.SIDE_FOCUS) {
        	if(keyCode == KeyEvent.KEYCODE_DPAD_UP || keyCode == KeyEvent.KEYCODE_DPAD_LEFT) {
        		return true;
        	}
    	}
    	
    	return super.onKeyDown(keyCode, event);
    }
    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
    	if(mState == CirclesControlState.MAIN_FOCUS) {
        	if(keyCode == KeyEvent.KEYCODE_DPAD_DOWN || keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
        		onFocusChanged(true, FOCUS_DOWN, new Rect());
        		playSoundEffect(SoundEffectConstants.NAVIGATION_DOWN);
        		return true;
        	}
    	}
    	
    	if(mState == CirclesControlState.SIDE_FOCUS) {
        	if(keyCode == KeyEvent.KEYCODE_DPAD_UP || keyCode == KeyEvent.KEYCODE_DPAD_LEFT) {
        		onFocusChanged(true, FOCUS_UP, new Rect());
        		playSoundEffect(SoundEffectConstants.NAVIGATION_UP);
        		return true;
        	}
    	}
    	
    	return super.onKeyUp(keyCode, event);
    }
}
