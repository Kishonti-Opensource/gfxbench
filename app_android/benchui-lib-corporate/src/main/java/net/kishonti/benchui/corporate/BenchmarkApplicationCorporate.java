/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.corporate;

import java.io.File;
import java.util.ArrayList;

import android.content.pm.PackageManager;

import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.theme.Localizator;
import net.kishonti.benchui.initialization.CheckResourceExistsTask;
import net.kishonti.benchui.initialization.CollectDeviceInfoTask;
import net.kishonti.benchui.initialization.DetermineBigDataDirTask;
import net.kishonti.benchui.initialization.DirFromAssetTask;
import net.kishonti.benchui.initialization.InitTask;
//import net.kishonti.benchui.initialization.RWDatabaseTask;
import net.kishonti.benchui.initialization.ReadonlyDatabaseTask;

public abstract class BenchmarkApplicationCorporate extends BenchmarkApplication {

	public BenchmarkApplicationCorporate() {
		super();
		instance = this;
	}

	@Override
	public String getVersionString() {
		try {
			return String.format(Localizator.getString(getApplicationContext(), "AppVersionCorporate"), getPackageManager().getPackageInfo(getPackageName(), 0).versionName);
		} catch (PackageManager.NameNotFoundException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	@Override
	public synchronized ArrayList<InitTask> getInitTasks() {
		
		ArrayList<InitTask> initTasks = new ArrayList<InitTask>();
		
		File path = getExternalFilesDir(null);
		if (path == null) path = getFilesDir();
		
		initTasks.add(new CheckResourceExistsTask(this, path.getPath() + "/data", false, false));
		initTasks.add(new DirFromAssetTask(this, "data", path.getPath()));
		initTasks.add(new CollectDeviceInfoTask(this));
		initTasks.add(new DetermineBigDataDirTask(this));
		initTasks.add(new ReadonlyDatabaseTask(this));
//		initTasks.add(new RWDatabaseTask(this));
		
		return initTasks;
	}

	@Override
	public boolean isResultSavingEnabled() {
		return true;
	}

	@Override
	public Boolean isDetailDiagramCompatible() {
		return false;
	}
}
