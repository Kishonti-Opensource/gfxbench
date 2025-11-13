/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import net.kishonti.swig.CompareItemVector;
import net.kishonti.swig.CompareItem;

public interface DuelListDataProvider {
	
	public CompareItemVector getDuelList();
	public int getDuelItemPosition(String resultId);
	public CompareItem getDuelItemForPosition(int position);
	public long getDuelCount();
	
}
