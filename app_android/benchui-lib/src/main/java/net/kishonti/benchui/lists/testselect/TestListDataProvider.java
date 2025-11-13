/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.lists.testselect;

import net.kishonti.swig.TestItem;
import net.kishonti.swig.TestItemVector;

public interface TestListDataProvider {
	
	public long getTestCount();
	public TestItem getTestForPosition(int position);
	public TestItemVector getTestList();
	public TestItemVector getSelectedTests(Boolean all);
	public void saveTestSelection(String prefs);
	public void loadTestSelection(String prefs);
	public void addTestIncompReason(String testId, String reason);
	public void enableLoginDependentTests(boolean enable);
	public void setSelectionForGroup(String groupId, Boolean selected);
	public void setSelectionForTest(String testId, Boolean selected);
	public TestItem getTest(String testId);
	
}
