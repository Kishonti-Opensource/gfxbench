/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;

public class MenuView extends LinearLayout {

	private boolean inflating;
	private LinearLayout mButtonDrawer;
	private Boolean isFill;

	private final ArrayList<View> pageHeaders = new ArrayList<View>();

	public interface MenuItemClickListener {
		void onMenuItemClick(int newItem);
	}

	private MenuItemClickListener itemClickListener;

	public void setOnMenuItemClickListener(MenuItemClickListener listener) {
		itemClickListener = listener;
	}

	private void Setup(Context context) {
		inflating = true;
		LayoutParams buttonDrawerParams = null;

		if (isFill) {
			buttonDrawerParams = new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
			buttonDrawerParams.setMargins(-1, 0, -1, 0);
			LayoutInflater.from(context).inflate(R.layout.layout_menu_bottom_fill, this, true);
		} else {
			buttonDrawerParams = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT, 0);
			buttonDrawerParams.setMargins(-1, 0, -1, 0);
			LayoutInflater.from(context).inflate(R.layout.layout_menu_bottom, this, true);
		}

		inflating = false;
		mButtonDrawer = (LinearLayout) findViewById(R.id.buttonDrawer);
	}

	public MenuView(Context context, AttributeSet attrs) {
		super(context, attrs);
		TypedArray a = context.getTheme().obtainStyledAttributes(attrs, R.styleable.CustomParams, 0, 0);
		isFill = a.getBoolean(R.styleable.CustomParams_fill, true);
		Setup(context);
	}

	private class InternalMenuItemClickListener implements OnClickListener {

		@Override
		public void onClick(View view) {
			select(pageHeaders.indexOf(view));
			if (itemClickListener != null) {
				itemClickListener.onMenuItemClick(pageHeaders.indexOf(view));
			}
		}
	}

	public int getMenuItemCount() {
		return pageHeaders.size();
	}

	public void setText(int index, String newString) {
		ViewIDs viewid = (ViewIDs) pageHeaders.get(index).getTag();
		viewid.btnText.setText(newString);
	}

	class ViewIDs {
		public TextView btnText;
		public ImageView btnImage;
	}

	@Override
	public void addView(View child, int index, ViewGroup.LayoutParams params) {
		if (!inflating) {
			if (mButtonDrawer != null) {
				if (child.getClass() == MenuViewItem.class) {
					MenuViewItem mvi = (MenuViewItem) child;
					View v = LayoutInflater.from(getContext()).inflate(R.layout.layout_menu_item, null);
					ViewIDs viewid = new ViewIDs();
					TextView tv = (TextView) v.findViewById(R.id.btnText);

					if (tv != null) {
						tv.setText(mvi.getText());
					}
					ImageView iv = (ImageView) v.findViewById(R.id.btnImage);
					if (iv != null) {
						iv.setImageDrawable(mvi.getImage());
					}
					viewid.btnText = tv;
					viewid.btnImage = iv;
					v.setTag(viewid);
					LayoutParams lp = null;

					if (isFill) {
						lp = new LayoutParams(0, ViewGroup.LayoutParams.MATCH_PARENT, 1);
					} else {
						lp = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT);
					}

					mButtonDrawer.addView(v, index, lp);
					if (index == -1) {
						pageHeaders.add(v);
					} else {
						pageHeaders.add(index, v);
					}

					v.setOnClickListener(new InternalMenuItemClickListener());
					if (isFill) {
						mButtonDrawer.setWeightSum(pageHeaders.size());
					}
				}
			}
		} else {
			if (child.getId() == R.id.buttonDrawer) {
				mButtonDrawer = (LinearLayout) child;
			}
			super.addView(child, index, params);
		}
	}

	public void select(int position) {
		for (int j = 0; j < pageHeaders.size(); j++) {
			View v = pageHeaders.get(j);
			v.setActivated(position == j);
		}

	}
}
