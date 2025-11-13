/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.deviceinfo;

import java.util.ArrayList;
import java.util.List;

public class AndroidInfo {

    public AndroidInfo() {
        features = new ArrayList<String>();
        sensors = new ArrayList<SensorInfo>();
        supported_abis = new ArrayList<String>();
        supported_32_bit_abis = new ArrayList<String>();
        supported_64_bit_abis = new ArrayList<String>();
        tags = new ArrayList<String>();
        cpuinfo_features = new ArrayList<String>();
        displays = new ArrayList<DisplayInfo>();
    }

    public String bootloader;
    public String device;
    public String brand;
    public String manufacturer;
    public String model;
    public String hardware;
    public String product;
    public String type;
    public String fingerprint;
    public String version_release;
    public String version_sdk;
    public String version_codename;
    public String version_incremental;
    public List<String> features;
    public List<SensorInfo> sensors;
    public List<String> supported_abis;
    public List<String> supported_64_bit_abis;
    public List<String> supported_32_bit_abis;
    public List<String> tags;
    public int version_sdk_int;

    public long internal_storage_size;
    public long external_storage_size;
    public long total_memory;

    public String default_locale;
    public String phone_type;
    public String sim_country_code;

    public List<String> cpuinfo_features;
    public String cpuinfo_processor;
    public String cpuinfo;
    public boolean cpuinfo_arm_emulated;
    public int cpu_count;
    public double cpu_max_frequency;
    public double cpu_min_frequency;
    public List<KeyValuePair<String>> env;
    public List<KeyValuePair<String>> getprop;
    public List<DisplayInfo> displays;
}
