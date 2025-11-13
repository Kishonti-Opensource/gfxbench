/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.resultlist;

import net.kishonti.benchui.db.CompareSQLiteHelper;
import net.kishonti.swig.CompareResult;
import net.kishonti.swig.CompareResultVector;

public interface CompareListDataProvider {
	
	public CompareResultVector getCompareList(String result_id, String filter, CompareSQLiteHelper.FormFactorFilter formFilter);
	public CompareResult getCompareItemForPosition(int position);
	public long getCompareListCount();
	
}
