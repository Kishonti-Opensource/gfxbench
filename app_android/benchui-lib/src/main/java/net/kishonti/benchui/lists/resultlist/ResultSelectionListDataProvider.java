/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import net.kishonti.swig.ResultItem;
import net.kishonti.swig.ResultItemVector;

public interface ResultSelectionListDataProvider {
	
	public ResultItemVector getResultSelectorList(boolean isHeadered);
	public int getResultSelectorItemPosition(String resultId);
	public ResultItem getResultSelectorItemForPosition(int position);
	public long getResultSelectorCount();
	
}
