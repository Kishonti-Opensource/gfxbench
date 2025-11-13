/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.theme;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;

/**
 * Interface used by the ThemeListView to inform ListItemViews about data changes that
 * are not list-wide. 
 * @author balazs hajagos
 */
public interface ThemeListItemViewHolder {
	
	/**
	 * Initializes the view with inflating and refreshes its contents based on
	 * the given item.
	 * @param context The context for inflating.
	 * @param parent The parent that will contain the created view.
	 * @param o The item that corresponds the position of the current view. It may be cast
	 * to the appropriate item class by the ItemView. 
	 * @param previousObject The item that corresponds the position before the current view.
	 * May be null if no items precedes this position. Must have same class as o. 
	 * @return Returns the created and inflated view.
	 */
	public View inflateAndInit(Context context, ViewGroup parent, Object o, int index);
	
	/**
	 * Refreshes the ItemView from the given item.
	 * @param o The item that corresponds the position of the current view. It may be cast
	 * to the appropriate item class by the holder. 
	 * @param previousObject The item that corresponds the position before the current view.
	 * May be null if no items precedes this position. Must have same class as o.
	 * @throws ClassCastException If the holder decides that the given object is not a
	 * class it can represent than it should throw an exception.
	 */
	public void refreshFromItem(Object o, int index) throws ClassCastException;
	

	public interface ThemeHeaderedLIVH {
		public void layoutAsHeadered(boolean separated);
	}

	public interface ThemeDisableableLIVH {
		public void layoutAsDisabled();
	}
}
