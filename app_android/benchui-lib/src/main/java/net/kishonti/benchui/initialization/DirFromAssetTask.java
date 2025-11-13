/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;

import org.apache.commons.io.IOUtils;

import net.kishonti.theme.Localizator;
import android.content.Context;

public class DirFromAssetTask extends InitTask {

	String mDest;
	String mSource;

	public DirFromAssetTask(Context context, String assetPath, String destination) {
		super(context, Localizator.getString(context, "DirFromAssetTask"), 1);
		mDest = destination;
		mSource = assetPath;
	}

	public DirFromAssetTask(Context context, String assetPath) {
		super(context, Localizator.getString(context, "DirFromAssetTask"), 1);
		mDest = null;
		mSource = assetPath;
	}

	@Override
	public Result run(HashMap<String, Object> params) {
		if (mDest == null) {
			mDest = (String)params.get(DetermineBigDataDirTask.KEY_BIGDATADIR);
		}
		if((Boolean)params.get(CheckResourceExistsTask.KEY_PUSHED_DIR_FOUND)) {
			return new Result(true, Localizator.getString(mContext, "DirFromAssetTask") + " " +
						Localizator.getString(mContext, "Success"));
		} else {
			Boolean success = copyTo(mSource, mDest);
			return new Result(success,  Localizator.getString(mContext, "DirFromAssetTask") + " " +
						(success ? Localizator.getString(mContext, "Success") : Localizator.getString(mContext, "Fail")));
		}
	}

	private void dirChecker(String dir) {
		File f = new File(dir);
		if(!f.isDirectory())
			f.mkdirs();
	}

	private Boolean copyTo(String source, String dest) {
		InputStream is = null;

		try {
			is = mContext.getAssets().open(source);
		} catch (IOException e) {
			try {
				for (String sub : mContext.getAssets().list(source)) {
					if(!copyTo(source + "/" + sub, source)) {
						return false;
					}
				}
			} catch (IOException innere) {
				innere.printStackTrace();
				return false;
			}

			return true;
		}

		dirChecker(mDest + "/" + dest);
		FileOutputStream fo;
		try {
			fo = new FileOutputStream(mDest + "/" + source);
			IOUtils.copy(is, fo);
			is.close();
			fo.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return false;
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}
}
