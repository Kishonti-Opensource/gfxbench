/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.deviceinfo;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Scanner;

import net.kishonti.testfw.AndroidRuntimeInfo;

import org.apache.commons.io.FileUtils;

import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.content.Context;
import android.content.pm.FeatureInfo;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.WindowManager;

@SuppressLint("NewApi")
public class AndroidInfoCollector {

    @SuppressWarnings("deprecation")
    public AndroidInfo collect(Context context) {
        AndroidInfo a = new AndroidInfo();
        a.device = Build.BOARD;
        a.manufacturer = Build.MANUFACTURER;
        a.brand = Build.BRAND;
        if (Build.VERSION.SDK_INT >= 21) {
            a.supported_abis = Arrays.asList(Build.SUPPORTED_ABIS);
            a.supported_32_bit_abis = Arrays.asList(Build.SUPPORTED_32_BIT_ABIS);
            a.supported_64_bit_abis = Arrays.asList(Build.SUPPORTED_64_BIT_ABIS);
        } else {
            a.supported_abis = Arrays.asList(Build.CPU_ABI, Build.CPU_ABI2);
            for (String abi : a.supported_abis) {
                if(abi.contains("64")) {
                    a.supported_64_bit_abis.add(abi);
                } else {
                    a.supported_32_bit_abis.add(abi);
                }
            }
        }
        a.bootloader = Build.BOOTLOADER;
        a.hardware = Build.HARDWARE;
        a.model = Build.MODEL;
        a.product = Build.PRODUCT;
        a.fingerprint = Build.FINGERPRINT;
        a.version_release = Build.VERSION.RELEASE;
        a.version_sdk_int = Build.VERSION.SDK_INT;
        a.version_codename = Build.VERSION.CODENAME;
        a.version_incremental = Build.VERSION.INCREMENTAL;
        a.version_sdk = Build.VERSION.SDK;
        a.type = Build.TYPE;
        a.external_storage_size = getTotalExternalStorageSize();
        a.internal_storage_size = getTotalInternalStorageSize();
        a.total_memory = getTotalMemory(context);
        a.default_locale = Locale.getDefault().toString();
        if (Build.TAGS != null) {
            a.tags = Arrays.asList(Build.TAGS.split(","));
        }
        CpuInfoParser cpuinfo = new CpuInfoParser();
        a.cpuinfo_arm_emulated = cpuinfo.isArmEmulated();
        a.cpuinfo_features = cpuinfo.getCpuFeatures();
        a.cpuinfo_processor = cpuinfo.getProcessor();
        a.cpuinfo = cpuinfo.getCpuInfoContent();

        AndroidRuntimeInfo runtime = new AndroidRuntimeInfo(context);
        a.cpu_count = runtime.cpuCount();
        a.cpu_max_frequency = runtime.maxCpuFrequencyMHz(0);
        a.cpu_min_frequency = runtime.minCpuFrequencyMHz(0);

        Map<String, String> env = System.getenv();
        a.env = new ArrayList<KeyValuePair<String>>();
        for (Map.Entry<String, String> entry : env.entrySet()) {
            a.env.add(new KeyValuePair<String>(entry.getKey(), entry.getValue()));
        }

        collectDisplays(context, a);
        collectGetprop(context, a);
        collectSensors(context, a);
        collectFeatures(context, a);
        return a;
    }

    private void collectGetprop(Context context, AndroidInfo a) {
        List<KeyValuePair<String>> props = new ArrayList<KeyValuePair<String>>();
        try {
            Process getprop = new ProcessBuilder("/system/bin/getprop").redirectErrorStream(true).start();
            InputStream stdout = getprop.getInputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(stdout));
            String line = null;
            while ((line = reader.readLine()) != null) {
                String[] v = line.split(":");
                if (v.length == 2) {
                    String key = cleanPropertyString(v[0]);
                    String value = cleanPropertyString(v[1]);
                    props.add(new KeyValuePair<String>(key, value));
                }
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        a.getprop = props;
    }

    private String cleanPropertyString(String s) {
        s = s.trim();
        return s.substring(1, s.length() - 1);
    }

    @SuppressWarnings("deprecation")
    private void collectSensors(Context context, AndroidInfo a) {
        SensorManager mgr = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
        List<Sensor> sensors = mgr.getSensorList(Sensor.TYPE_ALL);
        if (sensors != null) {
            for (Sensor s : sensors) {
                SensorInfo si = new SensorInfo();
                si.name = s.getName();
                si.vendor = s.getVendor();
                si.version = s.getVersion();
                si.maximum_range = s.getMaximumRange();
                si.resolution = s.getResolution();

                switch(s.getType()) {
                case Sensor.TYPE_ACCELEROMETER:
                    si.type = Sensor.STRING_TYPE_ACCELEROMETER;
                    break;
                case Sensor.TYPE_MAGNETIC_FIELD:
                    si.type = Sensor.STRING_TYPE_MAGNETIC_FIELD;
                    break;
                case Sensor.TYPE_ORIENTATION:
                    si.type = Sensor.STRING_TYPE_ORIENTATION;
                    break;
                case Sensor.TYPE_GYROSCOPE:
                    si.type = Sensor.STRING_TYPE_GYROSCOPE;
                    break;
                case Sensor.TYPE_LIGHT:
                    si.type = Sensor.STRING_TYPE_LIGHT;
                    break;
                case Sensor.TYPE_PRESSURE:
                    si.type = Sensor.STRING_TYPE_PRESSURE;
                    break;
                case Sensor.TYPE_TEMPERATURE:
                    si.type = Sensor.STRING_TYPE_TEMPERATURE;
                    break;
                case Sensor.TYPE_PROXIMITY:
                    si.type = Sensor.STRING_TYPE_PROXIMITY;
                    break;
                case Sensor.TYPE_GRAVITY:
                    si.type = Sensor.STRING_TYPE_GRAVITY;
                    break;
                case Sensor.TYPE_LINEAR_ACCELERATION:
                    si.type = Sensor.STRING_TYPE_LINEAR_ACCELERATION;
                    break;
                case Sensor.TYPE_ROTATION_VECTOR:
                    si.type = Sensor.STRING_TYPE_ROTATION_VECTOR;
                    break;
                case Sensor.TYPE_RELATIVE_HUMIDITY:
                    si.type = Sensor.STRING_TYPE_RELATIVE_HUMIDITY;
                    break;
                case Sensor.TYPE_AMBIENT_TEMPERATURE:
                    si.type = Sensor.STRING_TYPE_AMBIENT_TEMPERATURE;
                    break;
                case Sensor.TYPE_MAGNETIC_FIELD_UNCALIBRATED:
                    si.type = Sensor.STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
                    break;
                case Sensor.TYPE_GAME_ROTATION_VECTOR:
                    si.type = Sensor.STRING_TYPE_GAME_ROTATION_VECTOR;
                    break;
                case Sensor.TYPE_GYROSCOPE_UNCALIBRATED:
                    si.type = Sensor.STRING_TYPE_GYROSCOPE_UNCALIBRATED;
                    break;
                case Sensor.TYPE_SIGNIFICANT_MOTION:
                    si.type = Sensor.STRING_TYPE_SIGNIFICANT_MOTION;
                    break;
                case Sensor.TYPE_STEP_DETECTOR:
                    si.type = Sensor.STRING_TYPE_STEP_DETECTOR;
                    break;
                case Sensor.TYPE_STEP_COUNTER:
                    si.type = Sensor.STRING_TYPE_STEP_COUNTER;
                    break;
                case Sensor.TYPE_GEOMAGNETIC_ROTATION_VECTOR:
                    si.type = Sensor.STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
                    break;
                case Sensor.TYPE_HEART_RATE:
                    si.type = Sensor.STRING_TYPE_HEART_RATE;
                    break;
/*
                These values are in Sensor.class, but not documented. Eclipse can't compile.
                case Sensor.TYPE_TILT_DETECTOR:
                    si.type = Sensor.STRING_TYPE_TILT_DETECTOR;
                    break;
                case Sensor.TYPE_WAKE_GESTURE:
                    si.type = Sensor.STRING_TYPE_WAKE_GESTURE;
                    break;
                case Sensor.TYPE_GLANCE_GESTURE:
                    si.type = Sensor.STRING_TYPE_GLANCE_GESTURE;
                    break;
                case Sensor.TYPE_PICK_UP_GESTURE:
                    si.type = Sensor.STRING_TYPE_PICK_UP_GESTURE;
                    break;
 */
                default:
                    si.type = Integer.toString(s.getType());
                }
                if (Build.VERSION.SDK_INT >= 21) {
                    switch(s.getReportingMode()) {
                    case Sensor.REPORTING_MODE_CONTINUOUS:
                        si.reporting_mode = "REPORTING_MODE_CONTINUOUS";
                        break;
                    case Sensor.REPORTING_MODE_ON_CHANGE:
                        si.reporting_mode = "REPORTING_MODE_ON_CHANGE";
                        break;
                    case Sensor.REPORTING_MODE_ONE_SHOT:
                        si.reporting_mode = "REPORTING_MODE_ONE_SHOT";
                        break;
                    case Sensor.REPORTING_MODE_SPECIAL_TRIGGER:
                        si.reporting_mode = "REPORTING_MODE_SPECIAL_TRIGGER";
                        break;
                    default:
                        si.reporting_mode = Integer.toString(s.getReportingMode());
                    }
                }
                a.sensors.add(si);
            }
        }

    }

    public void collectDisplays(Context context, AndroidInfo a) {

        android.view.Display[] displays = null;
        if (Build.VERSION.SDK_INT < 17) {
            WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            android.view.Display display = windowManager.getDefaultDisplay();
            displays = new android.view.Display[] { display };
        } else {
            DisplayManager displayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
            displays = displayManager.getDisplays();
        }

        a.displays = new ArrayList<DisplayInfo>();
        for (android.view.Display display : displays) {
            DisplayMetrics displayMetrics = new DisplayMetrics();
            display.getMetrics(displayMetrics);
            Size size = getDisplaySizeInPixels(display);
            double x = Math.pow(size.width/displayMetrics.xdpi, 2);
            double y = Math.pow(size.height/displayMetrics.ydpi, 2);

            DisplayInfo d = new DisplayInfo();
            d.display_id = display.getDisplayId();
            if (Build.VERSION.SDK_INT > 17) {
                d.name = display.getName();
            }
            d.width = Math.max(size.width, size.height);
            d.height = Math.min(size.width, size.height);
            d.xdpi = displayMetrics.xdpi;
            d.ydpi = displayMetrics.ydpi;
            d.physical_size_inches = Math.sqrt(x+y);
            if (Build.VERSION.SDK_INT >= 21) {
                d.supported_refresh_rates = display.getSupportedRefreshRates();
            }
            a.displays.add(d);
        }
    }

    private Size getDisplaySizeInPixels(android.view.Display display)
    {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        display.getMetrics(displayMetrics);

        // since SDK_INT = 1;
        int width = displayMetrics.widthPixels;
        int height = displayMetrics.heightPixels;

        // includes window decorations (statusbar bar/menu bar)
        if (Build.VERSION.SDK_INT >= 14 && Build.VERSION.SDK_INT < 17) {
            try {
                width = (Integer) android.view.Display.class.getMethod("getRawWidth").invoke(display);
                height = (Integer) android.view.Display.class.getMethod("getRawHeight").invoke(display);
            } catch (Exception ignored) {
            }
        }

        // includes window decorations (statusbar bar/menu bar)
        if (Build.VERSION.SDK_INT >= 17) {
            try {
                Point realSize = new Point();
                android.view.Display.class.getMethod("getRealSize", Point.class).invoke(display, realSize);
                width = realSize.x;
                height = realSize.y;
            } catch (Exception ignored) {
            }
        }
        return new Size(width, height);
    }

    public void collectFeatures(Context context, AndroidInfo a) {
        PackageManager pkg = context.getPackageManager();
        FeatureInfo[] fis = pkg.getSystemAvailableFeatures();
        if (fis != null) {
            for (FeatureInfo f : fis) {
                if (f.name != null) {
                    a.features.add(f.name);
                }
            }
        }
    }


    public static long getTotalMemory(Context context) {
        ActivityManager mgr = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        MemoryInfo memInfo = new ActivityManager.MemoryInfo();
        mgr.getMemoryInfo(memInfo);
        return memInfo.totalMem;
    }


    public static boolean externalMemoryAvailable() {
        return android.os.Environment.getExternalStorageState().equals(
                android.os.Environment.MEDIA_MOUNTED);
    }

    public static long getTotalInternalStorageSize() {
        File path = Environment.getDataDirectory();
        StatFs stat = new StatFs(path.getPath());
        return getTotalSize(stat);
    }

    public static long getTotalExternalStorageSize() {
        if (externalMemoryAvailable()) {
            File path = Environment.getExternalStorageDirectory();
            StatFs stat = new StatFs(path.getPath());
            return getTotalSize(stat);
        }
        return 0;
    }

    @SuppressWarnings("deprecation")
    private static long getTotalSize(StatFs stat) {
        long blockSize = stat.getBlockSize();
        long totalBlocks = stat.getBlockCount();
        return totalBlocks * blockSize;
    }

    private static class CpuInfoParser {
        private static final String TAG = "CpuInfoParser";
        private List<String[]> mLines;
        private boolean mIsArm;
        private boolean mIsArmEmulated;
        private String mContent;

        public CpuInfoParser() {
            mIsArmEmulated = false;
            mLines = new ArrayList<String[]>();
            mContent = "";
            try {
                mContent = FileUtils.readFileToString(new File("/proc/cpuinfo"));
                Scanner scanner = new Scanner(mContent);
                while (scanner.hasNextLine()) {
                    String line = scanner.nextLine();
                    String e[] = line.split(":");
                    if (e.length == 2) {
                        e[0] = e[0].trim();
                        e[1] = e[1].trim();
                        mLines.add(e);
                    }
                }
                scanner.close();
                if (getValueForKey("CPU implementer") != null) {
                    mIsArm = true;
                }
                if (mIsArm) {
                    String implementer = getValueForKey("CPU implementer");
                    mIsArmEmulated = "0x69".equals(implementer);
                }
            } catch (IOException e) {
                Log.i(TAG, "Failed to read /proc/cpuinfo: " + e.getLocalizedMessage());
            }
        }

        public boolean isArmEmulated() {
            return mIsArmEmulated;
        }

        public List<String> getCpuFeatures() {
            List<String> features = new ArrayList<String>();
            try {
                features = Arrays.asList(getValueForKeys("Features", "flags").split(" "));
            } catch(Exception e) {
            }
            return features;
        }

        public String getProcessor() {
            String processor = "";
            try {
                processor = getValueForKeys("Processor", "model name");
            } catch(Exception e) {
            }
            return processor;
        }

        public String getCpuInfoContent() {
            return mContent;
        }

        private String getValueForKeys(String arm_key, String x86_key) {
            String key = mIsArm ? arm_key : x86_key;
            return getValueForKey(key);
        }

        private String getValueForKey(String key) {
            for (String[] e : mLines) {
                if (e[0].equals(key)) {
                    return e[1];
                }
            }
            return null;
        }

    }
}
