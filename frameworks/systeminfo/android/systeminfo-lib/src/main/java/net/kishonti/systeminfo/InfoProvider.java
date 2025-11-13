/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import android.content.Context;

//TODO remove this class
public abstract class InfoProvider {

	protected Context context ;

	public InfoProvider(Context context) {
		this.context = context;
	}

}
