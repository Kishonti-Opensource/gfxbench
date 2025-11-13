/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.widget.TextView;

public class AutoSizeTextView extends TextView {
	public AutoSizeTextView(Context context) {
		super(context);
	}

	public AutoSizeTextView(Context context, AttributeSet attrs) {
		super(context, attrs);
		super.setText(getText(), BufferType.SPANNABLE);
	}

	@Override
	public void setText(CharSequence text, BufferType type) {
		super.setText(text, BufferType.SPANNABLE);
		doSize();
	}

	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);
		doSize();
	}

	void doSize() {
		if (getWidth() > 0) {
			Paint p = this.getPaint();

			Rect bounds = new Rect();
			float scale = 1;
			p.getTextBounds(getText().toString(), 0, getText().length(), bounds);

			float cw = getWidth();
			float ch = getHeight();
			float ww = bounds.width();
			float hh = bounds.height();

			if (hh / ch > ww / cw) {
				scale = ch / hh;
			} else {
				scale = cw / ww;
			}

			p.setTextSize(p.getTextSize() * scale);
			setTextSize(TypedValue.COMPLEX_UNIT_PX, p.getTextSize() * 0.8f);
		}
	}
}
