/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;

public class Battery extends BroadcastReceiver {
	private static double mLevel = -1.0;
	private static boolean mPlugged;
	private static boolean mCharging;
	private boolean mIsRegistered;
	private Context mContext;
	private volatile static Battery mInstance;

	@Override
	public void onReceive(Context arg0, Intent intent) {
		/*
		 * public static final int BATTERY_PLUGGED_AC = 1;
		 * public static final int BATTERY_PLUGGED_USB = 2;
		 * public static final int BATTERY_PLUGGED_WIRELESS = 4;
		 * public static final int BATTERY_PLUGGED_ANY = BATTERY_PLUGGED_AC | BATTERY_PLUGGED_USB | BATTERY_PLUGGED_WIRELESS;
		 */

		mLevel = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0) / (float) intent.getIntExtra(BatteryManager.EXTRA_SCALE, 1);
		mPlugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0) > 0;
		mCharging = intent.getIntExtra(BatteryManager.EXTRA_STATUS, 0) == BatteryManager.BATTERY_STATUS_CHARGING;
	}

	public void release() {
		if (mIsRegistered) {
			mContext.unregisterReceiver(this);
			mIsRegistered = false;
		}
	}

	public void register(Context context) {
		if (mIsRegistered) {
			return;
		}
		mContext = context;
		Intent intent = mContext.registerReceiver(this, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
		if(intent != null) {
			mLevel = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0) / (float) intent.getIntExtra(BatteryManager.EXTRA_SCALE, 1);
			mPlugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0) > 0;
			mCharging = intent.getIntExtra(BatteryManager.EXTRA_STATUS, 0) == BatteryManager.BATTERY_STATUS_CHARGING;
			mIsRegistered = true;
		}
	}

	public boolean getCharging() {
		return mCharging;
	}

	public boolean getConnected() {
		return mPlugged;
	}

	public double getBatteryLevel() {
		return mLevel;
	}

	public static Battery getInstance() {
		if (mInstance == null) {
			synchronized (Battery.class) {
				if (mInstance == null) {
					mInstance = new Battery();
				}
			}
		}
		return mInstance;
	}

}
