/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.widget.LinearLayout;

public class FixedAspectView extends LinearLayout {
	double aspect;

	public FixedAspectView(Context context, AttributeSet attrs) {
		super(context, attrs);
		TypedArray a = context.getTheme().obtainStyledAttributes(attrs, R.styleable.CustomParams, 0, 0);
		aspect = (double) a.getInt(R.styleable.CustomParams_aspectX, 1) / (double) a.getInt(R.styleable.CustomParams_aspectY, 1);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int widthMode = MeasureSpec.getMode(widthMeasureSpec);
		int widthSize = MeasureSpec.getSize(widthMeasureSpec);
		int heightMode = MeasureSpec.getMode(heightMeasureSpec);
		int heightSize = MeasureSpec.getSize(heightMeasureSpec);

		int maxWidth = 0;
		int maxHeight = 0;
		int targetWidth;
		int targetHeight;

		if (widthMode != MeasureSpec.UNSPECIFIED) {
			maxWidth = widthSize;
		}
		if (heightMode != MeasureSpec.UNSPECIFIED) {
			maxHeight = heightSize;
		}

		if ((maxWidth > 0) && (maxHeight > 0)) {
			// set exact size with min(maxWidth,maxHeight
			if (maxWidth < maxHeight * aspect) {
				targetWidth = maxWidth;
				targetHeight = (int) (maxWidth / aspect);
			} else {
				targetWidth = (int) (maxHeight * aspect);
				targetHeight = maxHeight;
			}
		} else if (maxHeight > 0) {
			//set exact size with maxHeight,maxHeight
			targetWidth = (int) (maxHeight * aspect);
			targetHeight = maxHeight;
		} else if (maxWidth > 0) {
			//set exact size with maxWidth,maxWidth
			targetWidth = maxWidth;
			targetHeight = (int) (maxWidth / aspect);
		} else {
			// measure with unspecified, set exact size with max(width,height)
			super.onMeasure(MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED), MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED));
			targetWidth = getMeasuredWidth();
			targetHeight = getMeasuredHeight();
			targetWidth = Math.max(targetWidth, (int) (targetHeight * aspect));
			targetHeight = Math.max((int) (targetWidth / aspect), targetHeight);
		}

		// final measurement set
		super.onMeasure(MeasureSpec.makeMeasureSpec(targetWidth, MeasureSpec.EXACTLY), MeasureSpec.makeMeasureSpec(targetHeight, MeasureSpec.EXACTLY));
	}
}
