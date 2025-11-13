/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.ColorStateList;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public class NavigationBar extends LinearLayout implements View.OnClickListener {

	public interface OnNavBarClickListener {
		void onLeftClick(View view);

		void onRightClick(View view);
	}

	private OnNavBarClickListener listener;
	private Button left;
	private Button right;
	private TextView title;

	public NavigationBar(Context context, AttributeSet attrs) {
		super(context, attrs);
		View v = LayoutInflater.from(context).inflate(R.layout.layout_navbar, this);
		listener = null;
		left = (Button) v.findViewById(R.id.headerButtonLeft);
		right = (Button) v.findViewById(R.id.headerButtonRight);
		title = (TextView) v.findViewById(R.id.headerTitle);

		TypedArray a = context.getTheme().obtainStyledAttributes(attrs, R.styleable.CustomParams, 0, 0);
		ColorStateList leftButtonColor = a.getColorStateList(R.styleable.CustomParams_leftButtonColor);
		ColorStateList rightButtonColor = a.getColorStateList(R.styleable.CustomParams_rightButtonColor);
		ColorStateList titleColor = a.getColorStateList(R.styleable.CustomParams_titleColor);

		if (leftButtonColor != null) {
			left.setTextColor(leftButtonColor);
		}
		if (rightButtonColor != null) {
			right.setTextColor(rightButtonColor);
		}
		if (titleColor != null) {
			title.setTextColor(titleColor);
		}

		left.setEnabled(a.getBoolean(R.styleable.CustomParams_leftButtonEnabled, true));
		left.setVisibility(a.getBoolean(R.styleable.CustomParams_leftButtonVisible, false) ? View.VISIBLE : View.INVISIBLE);
		left.setText(a.getText(R.styleable.CustomParams_leftButtonText));
		left.setOnClickListener(this);

		title.setText(a.getText(R.styleable.CustomParams_titleText));

		right.setEnabled(a.getBoolean(R.styleable.CustomParams_rightButtonEnabled, true));
		right.setVisibility(a.getBoolean(R.styleable.CustomParams_rightButtonVisible, false) ? View.VISIBLE : View.INVISIBLE);
		right.setText(a.getText(R.styleable.CustomParams_rightButtonText));
		right.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		if (listener != null) {
			if (v == left) {
				listener.onLeftClick(this);
			}
			if (v == right) {
				listener.onRightClick(this);
			}
		}
	}

	public void setOnNavBarClickListener(OnNavBarClickListener listener) {
		this.listener = listener;
	}

	public void removeOnNavBarClickListener(OnNavBarClickListener listener) {
		if (this.listener == listener) {
			this.listener = null;
		}
	}

	public void setLeftButtonColor(ColorStateList buttonColor) {
		left.setTextColor(buttonColor);
	}

	public ColorStateList getLeftButtonColor() {
		return left.getTextColors();
	}

	public void setRightButtonColor(ColorStateList buttonColor) {
		right.setTextColor(buttonColor);
	}

	public ColorStateList getRightButtonColor() {
		return right.getTextColors();
	}

	public void setTitleColor(ColorStateList titleColor) {
		title.setTextColor(titleColor);
	}

	public ColorStateList getTitleColor() {
		return title.getTextColors();
	}

	public void setLeftText(String leftText) {
		left.setText(leftText);
	}

	public void setRightText(String rightText) {
		right.setText(rightText);
	}

	public void setTitleText(String titleText) {
		title.setText(titleText);
	}

	public String getLeftText() {
		return left.getText().toString();
	}

	public String getRightText() {
		return right.getText().toString();
	}

	public String getTitleText() {
		return title.getText().toString();
	}

	public void setLeftEnabled(boolean enabled) {
		left.setEnabled(enabled);
	}

	public boolean isLeftEnabled() {
		return left.isEnabled();
	}

	public void setLeftVisible(boolean visible) {
		left.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
	}

	public boolean isLeftVisible() {
		return left.getVisibility() == View.VISIBLE;
	}

	public void setRightEnabled(boolean enabled) {
		right.setEnabled(enabled);
	}

	public boolean isRightEnabled() {
		return right.isEnabled();
	}

	public void setRightVisible(boolean visible) {
		right.setVisibility(visible ? View.VISIBLE : View.INVISIBLE);
	}

	public boolean isRightVisible() {
		return right.getVisibility() == View.VISIBLE;
	}
}
