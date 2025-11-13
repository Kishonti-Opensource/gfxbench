/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.util.HashMap;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.resultsdb.ResultsDAODatabase;
import android.content.Context;

public class ReadonlyDatabaseTask extends InitTask {
	
	public ReadonlyDatabaseTask(Context context) {
		super(context, "", 1);
	}

	@Override
	public Result run(HashMap<String, Object> params) {
		String workingDir = (String)params.get(DetermineBigDataDirTask.KEY_BIGDATADIR);
		String databaseName = "top-results.sqlite";
		
//		if (ResultsDAODatabase.openSQLDB(workingDir + "/compare/" + databaseName) == null) {
//		if (ResultsDAODatabase.openSQLDB(workingDir + "/data/" + databaseName) == null) {
		
		String path = ((BenchmarkApplication)mContext).getCompareDatabasePath(workingDir); 
		if(ResultsDAODatabase.openSQLDB(path) == null) {
			ResultsDAODatabase.createDAODB(mContext, databaseName);
		}
		return new Result(true, "");
	}
	
	@Override
	public UserInteraction getUserInteraction() {
		return super.getUserInteraction();
	}
}
