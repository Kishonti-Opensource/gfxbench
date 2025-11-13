/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ScrollView;


public class NoInterceptScrollView extends ScrollView {

	public NoInterceptScrollView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	private boolean isSwitchAtCoords(int x, int y) {
		ViewGroup v = this;
		int sx = 0;
		int sy = 0;
		while (v.getChildCount() > 0) {
			sx -= v.getLeft();
			sy -= v.getTop();
			sx += v.getScrollX();
			sy += v.getScrollY();
			int i;
			for (i = 0; i < v.getChildCount(); i++) {
				if (isPointInsideView(x + sx, y + sy, v.getChildAt(i))) break;
			}

			if (i < v.getChildCount()) {
				if (FlipSwitch.class.isInstance(v.getChildAt(i))) {
					return true;
				} else if (ViewGroup.class.isInstance(v.getChildAt(i))) {
					v = (ViewGroup) v.getChildAt(i);
				} else {
					break;
				}
			} else {
				break;
			}
		}
		return false;
	}

	private boolean isPointInsideView(int x, int y, View v) {
		Rect rect = new Rect();
		v.getHitRect(rect);
		return rect.contains(x, y);
	}

	private boolean switchDragging = false;

	@Override
	public boolean onInterceptTouchEvent(MotionEvent ev) {
		if (ev.getAction() == MotionEvent.ACTION_DOWN) {
			if (isSwitchAtCoords((int) ev.getX(), (int) ev.getY())) {
				switchDragging = true;
			}
		}
		if ((ev.getAction() == MotionEvent.ACTION_UP) || (ev.getAction() == MotionEvent.ACTION_CANCEL)) {
			switchDragging = false;
		}
		return super.onInterceptTouchEvent(ev) && !switchDragging;
	}
}
