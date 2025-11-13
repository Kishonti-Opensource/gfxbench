/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.util.HashMap;

import net.kishonti.theme.Localizator;

import android.content.Context;

public class InternetNeededAlertTask extends InitTask {
	
	public static final String KEY_FIRSTRUN = "firstrun";
	
	boolean mFirstRun;
	
	public InternetNeededAlertTask(Context context) {
		super(context, "", 1);
	}

	@Override
	public Result run(HashMap<String, Object> params) {
		mFirstRun = mContext.getSharedPreferences(InitializerApplication.KEY_PREFSNAME, Context.MODE_PRIVATE).getBoolean(KEY_FIRSTRUN, true);
		mContext.getSharedPreferences(InitializerApplication.KEY_PREFSNAME, Context.MODE_PRIVATE).edit().putBoolean(EULATask.KEY_EULAACCEPTED, true).commit();
		return new Result(true, "");
	}
	
	@Override
	public UserInteraction getUserInteraction() {
		if(mFirstRun) {
			return new UserInteraction(
					Localizator.getString(mContext, "NetRequirementDialogTitle"), 
					Localizator.getString(mContext, "NetRequirementDialogBody"),
					Localizator.getString(mContext, "OK"), 
					null);
		} else {
			return super.getUserInteraction();
		}
	}

}
