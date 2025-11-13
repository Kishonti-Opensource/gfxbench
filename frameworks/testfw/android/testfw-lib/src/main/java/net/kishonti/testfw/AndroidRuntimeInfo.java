/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import net.kishonti.swig.RuntimeInfo;
import net.kishonti.swig.LinuxRuntimeInfo;



public class AndroidRuntimeInfo extends LinuxRuntimeInfo
{
    private final Context mContext;

    public AndroidRuntimeInfo(Context context) {
        mContext = context;
    }

    public RuntimeInfo.BatteryStatus batteryStatus() {
        switch (getBatteryData().getIntExtra(
                BatteryManager.EXTRA_STATUS, BatteryManager.BATTERY_STATUS_UNKNOWN))
        {
        case BatteryManager.BATTERY_STATUS_CHARGING:
            return BatteryStatus.BATTERY_CHARGING;
        case BatteryManager.BATTERY_STATUS_DISCHARGING:
            return BatteryStatus.BATTERY_DISCHARGING;
        case BatteryManager.BATTERY_STATUS_FULL:
            return BatteryStatus.BATTERY_FULL;
        case BatteryManager.BATTERY_STATUS_NOT_CHARGING:
            return BatteryStatus.BATTERY_NOT_CHARGING;
        default:
            return BatteryStatus.BATTERY_UNKNOWN;
        }
    }

    public double batteryLevelPercent() {
        Intent data = getBatteryData();
        return 100.0 * data.getIntExtra(BatteryManager.EXTRA_LEVEL, 0) /
                data.getIntExtra(BatteryManager.EXTRA_SCALE, 1);
    }

    public double batteryTemperatureCelsius() {
        return getBatteryData().getIntExtra(BatteryManager.EXTRA_TEMPERATURE, 0);
    }

    private Intent getBatteryData() {
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        return mContext.registerReceiver(null, intentFilter);
    }
}
