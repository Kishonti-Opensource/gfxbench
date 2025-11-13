/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.MotionEvent;
import android.view.View;

public class TabbarBtn extends View {
	public static final int BTNPOS_ONLY = 0;
	public static final int BTNPOS_LEFTEND = 1;
	public static final int BTNPOS_RIGHTEND = 2;
	public static final int BTNPOS_CENTER = 3;
	
	private String mName = "Home";
	private int mPictureId = -1;
	private int mSelectedPictureId = -1;
	private int mBtnPos = BTNPOS_ONLY;
	private boolean mSelected = false;
	private int mIndex = 0;
	
	private RectF mPictureBounds = new RectF();
	private RectF mBtnFillBounds = new RectF();
	private RectF mBtnForeBounds = new RectF();

    private Paint mTextPaint;
    private Paint mTextSelectedPaint;
    private Paint mBackPaint;
    private Paint mForePaint;
    private Paint mForeSelectedPaint;
    
    private Bitmap mPicture;
    private Bitmap mSelectedPicture;
    private float mPicSize = 0.0f;
    
    private float mTextX = 0.0f;
    private float mTextY = 0.0f;
    private float mTextHeight = 0.0f;
    private float mMargin = 0.0f;
    
    private OnSelectionChangedListener mSelectionChangedListener = null;
    
    /**
     * Interface definition for a callback to be invoked when selection changes.
     */
    public interface OnSelectionChangedListener {
        void OnSelectionChanged(TabbarBtn source, boolean selection);
    }
    
	
	public TabbarBtn(Context context) {
		super(context);
		init();
	}
	
	public TabbarBtn(Context context, String name, int pictureId, int selectedPictureId, int btnPos, boolean selected, int index) {
		super(context);

        // Retrieve the values from the TypedArray and store into
        // fields of this class.
        mName = name;
        mPictureId = pictureId;
        mSelectedPictureId = selectedPictureId;
        mBtnPos = btnPos;
        mSelected = selected;
        mIndex = index;
        
		init();
	}

	public TabbarBtn(Context context, AttributeSet attrs) {
		super(context, attrs);
		
		TypedArray a = context.getTheme().obtainStyledAttributes(
                attrs,
                R.styleable.TabbarBtn,
                0, 0
        );

        try {
            // Retrieve the values from the TypedArray and store into
            // fields of this class.
            mName = a.getString(R.styleable.TabbarBtn_TB_text);
            mPictureId = a.getResourceId(R.styleable.TabbarBtn_TB_picture, -1);
            mSelectedPictureId = a.getResourceId(R.styleable.TabbarBtn_TB_selectedPicture, -1);
            mBtnPos = a.getInteger(R.styleable.TabbarBtn_TB_btnPosition, BTNPOS_ONLY);
            mSelected = a.getBoolean(R.styleable.TabbarBtn_TB_selected, false);
            mIndex = a.getInteger(R.styleable.TabbarBtn_TB_index, 0);
        } finally {
            // release the TypedArray so that it can be reused.
            a.recycle();
        }

        init();
	}
	
    /**
     * Returns a value that specifies the index of the button.
     *
     * @return The index of the button.
     */
    public int getIndex() {
        return mIndex;
    }
	
    /**
     * Returns a value that specifies the position of the button in the
     * tabbar.
     *
     * @return One of BTNPOS_ONLY, BTNPOS_CENTER, BTNPOS_LEFTEND or BTNPOS_RIGHTEND.
     */
    public int getBtnPos() {
        return mBtnPos;
    }

    /**
     * Set a value that specifies the position of the button in the
     * tabbar.
     *
     * @param btnPos BTNPOS_ONLY if its the only btn in the tabbar,
     * 				BTNPOS_CENTER if its a middle element,
     * 				BTNPOS_LEFTEND or BTNPOS_RIGHTEND if it's positioned on the left
     * 				or right.
     */
    public void setBtnPos(int btnPos) {
        if (btnPos != BTNPOS_CENTER && btnPos != BTNPOS_LEFTEND && btnPos != BTNPOS_ONLY && btnPos != BTNPOS_RIGHTEND) {
            throw new IllegalArgumentException(
                    "TextPos must be one of {BTNPOS_CENTER, BTNPOS_LEFTEND, BTNPOS_ONLY, BTNPOS_RIGHTEND");
        }
        mBtnPos = btnPos;
        //TODO this should be on size changed or something similar....
        invalidate();
    }
    
    /**
     * Returns a value that specifies whether the button is in the 
     * selected state.
     *
     * @return True or false.
     */
    public boolean getSelected() {
        return mSelected;
    }

    /**
     * Set a value that specifies whether the button is in the 
     * selected state.
     *
     * @param sel True to select the button else false.
     */
    public void setSelected(boolean sel) {
        mSelected = sel;
        onSizeChanged(getWidth(), getHeight(), getWidth(), getHeight());
        invalidate();
        if (mSelectionChangedListener != null) {
        	mSelectionChangedListener.OnSelectionChanged(this, mSelected);
        }
    }
    
    /**
     * Set a value that specifies whether the button is in the 
     * selected state.
     *
     * @param sel True to select the button else false.
     * @param invokeSelectionChanged Set true to invoke the OnSelectionChanged methods
     * 			false otherwise.
     */
    public void setSelected(boolean sel, boolean invokeSelectionChanged) {
        mSelected = sel;
        onSizeChanged(getWidth(), getHeight(), getWidth(), getHeight());
        invalidate();
        if (mSelectionChangedListener != null && invokeSelectionChanged) {
        	mSelectionChangedListener.OnSelectionChanged(this, mSelected);
        }
    }
    
    /**
     * Register a callback to be invoked when the button selection changes.
     *
     * @param listener Can be null.
     *                 The current selection changed listener to attach to this view.
     */
    public void setOnSelectionChangedListener(OnSelectionChangedListener listener) {
    	mSelectionChangedListener = listener;
    }


    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        // Do nothing. Do not call the superclass method--that would start a layout pass
        // on this view's children. TabbarBtn lays out its children in onSizeChanged().
    }
    
    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        // Draw the background
        canvas.drawRect(mBtnFillBounds, mBackPaint);
        
        // Draw the foreground
        if(mSelected) {
            canvas.drawRect(mBtnForeBounds, mForeSelectedPaint);
        } else {
            canvas.drawRect(mBtnForeBounds, mForePaint);
        }
        
        // Draw the picture
        if(mSelected) {
            canvas.drawBitmap(mSelectedPicture, null, mPictureBounds, null);
        } else {
            canvas.drawBitmap(mPicture, null, mPictureBounds, null);
        }
        
        // Draw the text
        if(mSelected) {
        	canvas.drawText(mName, mTextX, mTextY, mTextSelectedPaint);
        } else {
        	canvas.drawText(mName, mTextX, mTextY, mTextPaint);
        }
        
    }
    
    //
    // Measurement functions. This example uses a simple heuristic: it assumes that
    // the pie chart should be at least as wide as its label.
    //
    @Override
    protected int getSuggestedMinimumWidth() {
        return (int) 1;
    }

    @Override
    protected int getSuggestedMinimumHeight() {
        return (int) (mPicSize + 4*mMargin + mTextHeight);
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
        
        // Account for padding
        float xpad = (float) (getPaddingLeft() + getPaddingRight());
        float ypad = (float) (getPaddingTop() + getPaddingBottom());
        
        
        float leftInnerMargin = 0;
        float rightInnerMargin = 0;
        if(mBtnPos == BTNPOS_ONLY) {
        	leftInnerMargin = 0;
            rightInnerMargin = 0;
        } else if(mBtnPos == BTNPOS_CENTER) {
        	leftInnerMargin = mMargin;
            rightInnerMargin = mMargin;
        } else if(mBtnPos == BTNPOS_LEFTEND) {
        	leftInnerMargin = 0;
            rightInnerMargin = mMargin;
        } else if(mBtnPos == BTNPOS_RIGHTEND) {
        	leftInnerMargin = mMargin;
            rightInnerMargin = 0;
        }
        
        float topInnerMargin = 2*mMargin;
        if(mSelected) {
        	topInnerMargin = 0;
        }
        
        float fullHeight = mPicSize + 2*mMargin + mTextHeight;
        
        mBtnFillBounds = new RectF(getPaddingLeft(), getPaddingTop(), w - xpad, h - ypad);
        mBtnForeBounds = new RectF(mBtnFillBounds.left + leftInnerMargin, mBtnFillBounds.top + topInnerMargin, 
        		mBtnFillBounds.right - rightInnerMargin, mBtnFillBounds.bottom);
        mPictureBounds = new RectF(mBtnForeBounds.centerX() - mPicSize/2, mBtnForeBounds.centerY() - fullHeight/2.0f, 
        		mBtnForeBounds.centerX() + mPicSize/2, mBtnForeBounds.centerY() - fullHeight/2.0f + mPicSize);
        
        mTextX = mPictureBounds.centerX();
        mTextY = mPictureBounds.bottom + 4*mMargin + mTextHeight;
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) {
    	boolean result = false;
    	if (event.getAction() == MotionEvent.ACTION_DOWN) {
            setSelected(true);
            result = true;
        }
        return result;
    }
    
    private void init() {

		mMargin = getContext().getResources().getDimension(R.dimen.tabbar_margin);
    	
    	// Set up the paint for the label text
    	mTextPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    	int mTextColor = getContext().getResources().getColor(R.color.tabbar_font_normal);
    	mTextPaint.setColor(mTextColor);
		mTextHeight = getContext().getResources().getDimension(R.dimen.tabbar_fontsize);
		mTextPaint.setTextSize(mTextHeight);
        mTextPaint.setTextAlign(Paint.Align.CENTER);
        
    	mTextSelectedPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    	int mTextSelectedColor = getContext().getResources().getColor(R.color.tabbar_font_selected);
    	mTextSelectedPaint.setColor(mTextSelectedColor);
    	mTextSelectedPaint.setTextSize(mTextHeight);
    	mTextSelectedPaint.setTextAlign(Paint.Align.CENTER);
    	
    	// Set up the paint for the background
    	mBackPaint = new Paint(0);
    	int mBackColor = getContext().getResources().getColor(R.color.tabbar_back);
    	mBackPaint.setColor(mBackColor);
    	
    	// Set up the paint for the background
    	mForePaint = new Paint(0);
    	int mForeColor = getContext().getResources().getColor(R.color.tabbar_btn_back);
    	mForePaint.setColor(mForeColor);
    	mForeSelectedPaint = new Paint(0);
    	int mForeSelectedColor = getContext().getResources().getColor(R.color.tabbar_btn_selected);
    	mForeSelectedPaint.setColor(mForeSelectedColor);
    	
    	if(mPictureId > 0) {
        	mPicture = BitmapFactory.decodeResource(getResources(), mPictureId);
    	} else {
    		mPicture = BitmapFactory.decodeResource(getResources(), R.drawable.app_home_icon);
    	}
    	
    	if(mSelectedPictureId > 0) {
        	mSelectedPicture = BitmapFactory.decodeResource(getResources(), mSelectedPictureId);
    	} else {
    		mSelectedPicture = BitmapFactory.decodeResource(getResources(), R.drawable.app_home_icon_sel);
    	}
    	
    	//TODO get normal size somehow... (48x48 --> 36x36)
    	//mPicSize = mPicture.getHeight()*3/4;
    	//mPicSize = mPicture.getHeight();
    	mPicSize = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 40, getResources().getDisplayMetrics());
    }
}
