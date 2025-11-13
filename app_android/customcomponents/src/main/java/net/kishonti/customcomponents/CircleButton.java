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
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.ViewGroup;

public class CircleButton extends SquareView {

	private final Paint buttonPaint = new Paint();
	private final Paint buttonBorderPaint = new Paint();
	private final Paint backgroundPaint = new Paint();
	private final RectF clientRect = new RectF();
	AutoSizeTextView textView = null;

	private OnClickListener clickListener;

	@Override
	public void setOnClickListener(OnClickListener l) {
		clickListener = l;
	}

	private void Setup() {
		setWillNotDraw(false);
		buttonBorderPaint.setStyle(Paint.Style.STROKE);
		backgroundPaint.setColor(0x00000000);
		//textPaint.setTypeface(getTypeface());
	}

	public void setText(String text) {
		textView.setText(text);
	}

	public CircleButton(Context context, AttributeSet attrs) {
		super(context, attrs);
		Setup();
		TypedArray a = context.getTheme().obtainStyledAttributes(attrs, R.styleable.CustomParams, 0, 0);
		buttonPaint.setColor(a.getColor(R.styleable.CustomParams_fillColor, 0));
		buttonPaint.setAntiAlias(true);
		buttonBorderPaint.setColor(a.getColor(R.styleable.CustomParams_borderColor, 0));
		buttonBorderPaint.setStrokeWidth(a.getDimensionPixelSize(R.styleable.CustomParams_borderWidth, 0));
		buttonBorderPaint.setAntiAlias(true);
		textView = new AutoSizeTextView(context);
		textView.setTextColor(a.getColor(R.styleable.CustomParams_textColor, 0));
		textView.setGravity(Gravity.CENTER);
		textView.setScaleX(0.8f);
		textView.setScaleY(0.8f);
		textView.setLayoutParams(new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
		textView.setText(a.getText(R.styleable.CustomParams_text));
		this.addView(textView);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (clickListener == null) return false;
		if (!isEnabled()) return false;
		int cx = getWidth() / 2;
		int cy = getHeight() / 2;
		int diffX = (int) (event.getX() - cx);
		int diffY = (int) (event.getY() - cy);
		if (diffX * diffX + diffY * diffY < cx * cx) {
			if (event.getAction() == MotionEvent.ACTION_DOWN) return true;
			if (event.getAction() == MotionEvent.ACTION_UP) {
				clickListener.onClick(this);
				return true;
			}
			return true;
		}
		return false;
	}

	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
		super.onSizeChanged(w, h, oldw, oldh);
		invalidate();
	}

	@Override
	protected void onDraw(Canvas canvas) {
		int w = getWidth();
		int h = getHeight();
		clientRect.set(0 + buttonBorderPaint.getStrokeWidth() / 2, 0 + buttonBorderPaint.getStrokeWidth() / 2, w - buttonBorderPaint.getStrokeWidth() / 2, h - buttonBorderPaint.getStrokeWidth() / 2);
		canvas.drawRect(0, 0, w, h, backgroundPaint);
		canvas.drawOval(clientRect, buttonBorderPaint);
		canvas.drawOval(clientRect, buttonPaint);
		//super.draw(canvas);
	}
}
