/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.io.File;
import java.util.HashMap;

import net.kishonti.theme.Localizator;
import android.content.Context;

public class CheckResourceExistsTask extends InitTask {

	public static final String KEY_PUSHED_DIR_FOUND = "pushedDirFound";

	String mPath = "";
	boolean data_exists = false;
	boolean mSilent = false;
	boolean mIsRelativeToBigDataDir;
	File mAbsDataPath;

	public CheckResourceExistsTask(Context context, String path, boolean isRelativeToBigDataDir, boolean silent) {
		super(context, Localizator.getString(context, "CheckPushedDir"), 1);
		mIsRelativeToBigDataDir = isRelativeToBigDataDir;
		mPath = path;
		mSilent = silent;
	}

	@Override
	public Result run(HashMap<String, Object> params) {
		File f = null;
		if (mIsRelativeToBigDataDir) {
			String bigDataDir = (String)params.get(DetermineBigDataDirTask.KEY_BIGDATADIR);
			f = new File(bigDataDir + mPath);
		} else {
			f = new File(mPath);
		}
		mAbsDataPath = f;
		if(f.exists() && f.isDirectory()) {
			params.put(KEY_PUSHED_DIR_FOUND, true);
			data_exists = true;
			return new Result(true,
					Localizator.getString(mContext, "CheckPushedDir") + ": " +
							Localizator.getString(mContext, "Success"));
		} else {
			params.put(KEY_PUSHED_DIR_FOUND, false);
			return new Result(true,
					Localizator.getString(mContext, "CheckPushedDir") + ": " +
							Localizator.getString(mContext, "Fail"));
		}
	}

	@Override
	public UserInteraction getUserInteraction() {
		if(!data_exists && !mSilent)
			return new UserInteraction(
				Localizator.getString(mContext, "FromAssetDirDialogTitle"),
				String.format(Localizator.getString(mContext, "FromAssetDirDialogBody"), mAbsDataPath.getAbsolutePath()),
				Localizator.getString(mContext, "OK"),
				Localizator.getString(mContext, "Quit"));
		else
			return super.getUserInteraction();
	}

}
