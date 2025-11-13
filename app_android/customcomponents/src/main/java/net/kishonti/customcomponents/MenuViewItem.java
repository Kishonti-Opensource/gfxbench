/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.View;

public class MenuViewItem extends View {

	private Drawable image;
	private String text;

	public MenuViewItem(Context context, AttributeSet attrs) {
		super(context, attrs);
		TypedArray a = context.getTheme().obtainStyledAttributes(attrs, R.styleable.CustomParams, 0, 0);
		text = a.getString(R.styleable.CustomParams_text);
		int imageId = a.getResourceId(R.styleable.CustomParams_image, 0);
		if (imageId != 0) {
			image = getResources().getDrawable(imageId);
		}
	}

	public Drawable getImage() {
		return image;
	}

	public String getText() {
		return text;
	}

	public void setText(String newText) {
		text = newText;
	}
}
