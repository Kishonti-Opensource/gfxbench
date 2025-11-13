/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.customcomponents;

import android.content.Context;
import android.util.AttributeSet;

public class SquareView extends FixedAspectView {

	public SquareView(Context context, AttributeSet attrs) {
		super(context, attrs);
		aspect = 1;
	}
}
