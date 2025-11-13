/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.support.v4.view.GestureDetectorCompat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.SoundEffectConstants;
import android.view.View;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.Scroller;
import android.widget.TextView;

public class FlipSwitch extends LinearLayout {
	public interface OnStateChangeListener {
		public void onStateChanged(View view, boolean state);
	}

	private GestureDetectorCompat mGestureDetector = null;
	private HorizontalScrollView hsv = null;
	private Scroller scr = null;
	private boolean isInDragMode = false;
	private int knobColor = -1;
	private int textColor = -1;
	private boolean isenabled = false;
	private boolean state = false;
	private int width = 0;
	private int childWidth = 0;
	private OnStateChangeListener stateChangeListener = null;

	public void setOnStateChangeListener(OnStateChangeListener listener) {
		stateChangeListener = listener;
	}

	public boolean getState() {
		return state;
	}

	@Override
	public void setEnabled(boolean newstate) {
		super.setEnabled(newstate);
		isenabled = newstate;
	}

	public void changeState(boolean newState) {
		isInDragMode = false;
		if (!newState) {
			scr.startScroll(hsv.getScrollX(), 0, hsv.getChildAt(0).getWidth() - hsv.getWidth() - hsv.getScrollX(), 0);
		} else {
			scr.startScroll(hsv.getScrollX(), 0, -hsv.getScrollX(), 0);
		}
		postInvalidate();
		if (newState == state) {
			return;
		}
		state = newState;
		if (stateChangeListener != null) {
			stateChangeListener.onStateChanged(this, state);
		}
	}

	@Override
	public boolean onInterceptTouchEvent(MotionEvent ev) {
		return true;
	}

	@Override
	public boolean onTouchEvent(MotionEvent motionEvent) {
		if (!getEnabled()) {
			return false;
		}
		if ((motionEvent.getAction() == MotionEvent.ACTION_UP) || (motionEvent.getAction() == MotionEvent.ACTION_CANCEL)) {
			if (isInDragMode) {
				changeState(scr.getCurrX() < (hsv.getChildAt(0).getWidth() - hsv.getWidth()) / 2);
			}
		}
		mGestureDetector.onTouchEvent(motionEvent);
		return true;
	}

	private void Setup(Context context) {
		final View v = LayoutInflater.from(context).inflate(R.layout.layout_switch, this, true);
		hsv = (HorizontalScrollView) v.findViewById(R.id.switchScroller);
		mGestureDetector = new GestureDetectorCompat(context, new SwitchGestureListener());
		scr = new Scroller(context);

//		View knob = v.findViewById(R.id.switchOn);
//		((TextView) knob).setTextColor(textColor);
//		knob.setBackgroundColor(knobColor);
		setWillNotDraw(false);
		changeState(state);
	}

	public FlipSwitch(Context context) {
		super(context);
		isenabled = true;
		textColor = context.getResources().getColor(R.color.flipswitch_on_text);
		knobColor = context.getResources().getColor(R.color.flipswitch_on_back);
		state = false;
		
		setFocusable(true);
		setClickable(true);
		
		Setup(context);
	}

	public FlipSwitch(Context context, AttributeSet attrs) {
		super(context, attrs);
		TypedArray a = context.getTheme().obtainStyledAttributes(attrs, R.styleable.CustomParams, 0, 0);
		isenabled = a.getBoolean(R.styleable.CustomParams_enabled, true);
		textColor = a.getColor(R.styleable.CustomParams_textColor, R.color.flipswitch_on_text);
		knobColor = a.getColor(R.styleable.CustomParams_knobColor, R.color.flipswitch_on_back);
		state = a.getBoolean(R.styleable.CustomParams_state, false);
		
		setFocusable(true);
		setClickable(true);
		
		Setup(context);
	}

	private void CheckSizeChange() {
		if ((hsv.getWidth() != width) || (hsv.getChildAt(0).getWidth() != childWidth)) {
			width = hsv.getWidth();
			childWidth = hsv.getChildAt(0).getWidth();
			changeState(state);
		}
	}

	public void setCustomEnabled(Boolean newValue) {
		isenabled = newValue;
	}

	public boolean getEnabled() {
		return isenabled;
	}

	private class SwitchGestureListener extends GestureDetector.SimpleOnGestureListener {
		@Override
		public boolean onDown(MotionEvent motionEvent) {
			if (!getEnabled()) {
				return false;
			}
			return true;
		}

		@Override
		public void onShowPress(MotionEvent motionEvent) {
		}

		@Override
		public boolean onSingleTapUp(MotionEvent motionEvent) {
			isInDragMode = false;
			if (!getEnabled()) {
				return false;
			}
			if (scr.isFinished()) {
				changeState(!state);
			}
			return true;
		}

		@Override
		public boolean onScroll(MotionEvent motionEvent, MotionEvent motionEvent2, float v, float v2) {
			if (!getEnabled()) {
				return false;
			}
			if (!isInDragMode) {
				isInDragMode = true;
				scr.abortAnimation();
			}
			scr.setFinalX((int) (scr.getFinalX() + v));
			postInvalidate();
			return true;
		}

		@Override
		public void onLongPress(MotionEvent motionEvent) {

		}

		@Override
		public boolean onFling(MotionEvent motionEvent, MotionEvent motionEvent2, float v, float v2) {
			if (!getEnabled()) {
				return false;
			}
			changeState(v > 0);
			return true;
		}
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		CheckSizeChange();
		boolean isStillScrolling = scr.computeScrollOffset();
		hsv.scrollTo(scr.getCurrX(), scr.getCurrY());
		if (isStillScrolling) {
			postInvalidate();
		}
	}
	
	@Override
	public boolean performClick() {
		super.performClick();
		if(isEnabled()) {
			changeState(!getState());
    		playSoundEffect(SoundEffectConstants.CLICK);
			return true;
		}
		return false;
	}
}
