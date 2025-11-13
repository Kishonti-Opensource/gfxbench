/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.io.File;
import java.util.HashMap;

import net.kishonti.systeminfo.swig.Properties;
import net.kishonti.systeminfo.swig.PropertyIter;
import net.kishonti.systeminfo.swig.systeminfolib;
import net.kishonti.theme.Localizator;

import android.content.Context;

public class DetermineBigDataDirTask extends InitTask {
	
	public static final String KEY_BIGDATADIR = "bigDataDir";
	public static final String KEY_MINSIZE = "bigDataMinSize";
	public static final String KEY_MANUAL_UNINSTALL = "bigDataManualUninstall";
	
	private String mFromPreferences;

	public DetermineBigDataDirTask(Context context) {
		super(context, Localizator.getString(context, "StorageCheck"), 1);
	}

	@Override
	public Result run(HashMap<String, Object> params) {
		mFromPreferences = mContext.getSharedPreferences(InitializerApplication.KEY_PREFSNAME, Context.MODE_PRIVATE).getString(KEY_BIGDATADIR, null);
		Integer minSize = 0;
		if(params.get(KEY_MINSIZE) != null) {
			minSize = (Integer)params.get(KEY_MINSIZE);
		}
		Properties systeminfo = (Properties)params.get(CollectDeviceInfoTask.KEY_SYSTEMINFO);
		
		if(mFromPreferences == null) {
			String path;
			
			if (mContext.getExternalFilesDir("")!=null) {
				path = mContext.getExternalFilesDir("").getAbsolutePath();
				if(hasEnoughFreeSpace(path, minSize)) {
					params.put(KEY_BIGDATADIR, path);
					return new Result(true, "Huge dataset storage found.");
				}
			}

			path = mContext.getDir("sync", Context.MODE_PRIVATE).getAbsolutePath();
			if(hasEnoughFreeSpace(path, minSize)) {
				params.put(KEY_BIGDATADIR, path);
				return new Result(true, "Huge dataset storage found.");
			}
			
			PropertyIter iter = systeminfo.groupIterator(systeminfolib.getSTORAGE());
			while (!iter.done()) {
				String[] parts = iter.name().split("/");
				if (parts[parts.length-1].contentEquals("mnt"))
				{
					path = iter.value().getString()+"/Android/data/"+mContext.getPackageName()+"/files";
					if(hasEnoughFreeSpace(path, minSize)) {
						params.put(KEY_BIGDATADIR, path);
						params.put(KEY_MANUAL_UNINSTALL, true);
						return new Result(true, "Huge dataset storage found.");
					}
				}
				iter.next();
			}
			
			return new Result(false, Localizator.getString(mContext, "StorageCheck") + " " + Localizator.getString(mContext, "Error"));
		} else {
			params.put(KEY_BIGDATADIR, mFromPreferences);
			return new Result(true,  Localizator.getString(mContext, "StorageCheck") + " " + Localizator.getString(mContext, "OK"));
		}
	}
	
	private boolean hasEnoughFreeSpace(String path, int required) {
		String[] pathSegments = path.split("//");
		if(pathSegments.length < 1) return false;
		String destinationPath = pathSegments[pathSegments.length - 1];
		File destinationFile = new File(destinationPath);
		destinationFile.mkdirs();
		Long freeSpaceAtDestination = destinationFile.getFreeSpace();
		return freeSpaceAtDestination > required;
	}

}
