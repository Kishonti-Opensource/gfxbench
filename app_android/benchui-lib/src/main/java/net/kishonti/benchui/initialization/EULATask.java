/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.util.HashMap;

import net.kishonti.theme.Localizator;
import android.content.Context;

public class EULATask extends InitTask {

	public static final String KEY_EULAACCEPTED = "eulaAccept";
	
	private boolean mAccepted;
	
	public EULATask(Context context) {
		super(context, "", 1);
	}

	@Override
	public Result run(HashMap<String, Object> params) {
		mAccepted = mContext.getSharedPreferences(InitializerApplication.KEY_PREFSNAME, Context.MODE_PRIVATE).getBoolean(KEY_EULAACCEPTED, false);
		return new Result(true, "");
	}
	
	@Override
	public UserInteraction getUserInteraction() {
		if(!mAccepted) {
			return new UserInteraction(
					Localizator.getString(mContext, "LicenseDialogTitle"), 
					Localizator.getString(mContext, "LicenseDialogBody"),  
					Localizator.getString(mContext, "Accept"),
					Localizator.getString(mContext, "Decline"));
		} else {
			return super.getUserInteraction();
		}
	}
}
