/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.model;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import net.kishonti.benchui.R;
import net.kishonti.systeminfo.AndroidDeviceInfoCollector;
import net.kishonti.systeminfo.swig.Properties;
import net.kishonti.systeminfo.swig.StringVector;
import net.kishonti.systeminfo.swig.SystemInfo;
import net.kishonti.systeminfo.swig.systeminfolib;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.util.Log;

public class MinimalProps {
	public String appinfo_packagename;
	public String appinfo_installername;
	public String appinfo_version;
	public String appinfo_storename;
	public String appinfo_benchmark_id;
	public String appinfo_locale;
	public String device_name;
	public String appinfo_testjson;
	
	public long appinfo_versioncode;
	public int appinfo_gles_major;
	public int appinfo_gles_minor;
	public int appinfo_featureset_major;
	public int appinfo_featureset_minor;
	public boolean appinfo_gles_aep_compatible = false;
	public boolean appinfo_gles_supports_aztec = false;
    public boolean has_EXT_color_buffer_float = false;
	
	public boolean gles_supports_astc = false;
	public boolean any_vulkan_device_needs_astc = false;
	public boolean any_vulkan_device_needs_etc2 = false;

	public int appinfo_opencl_major = 0;
	public int appinfo_opencl_minor = 0;
	
	public int appinfo_vulkan_major = 0;
	public int appinfo_vulkan_minor = 0;
	//public int appinfo_cuda_major;
	//public int appinfo_cuda_minor;
	
	public Properties props = new Properties();
	public SystemInfo sysinf = new SystemInfo();
	
	public List<String> syncProps = new ArrayList<String>();
	
	private void setFeatureSetBasedOnBenchmarkId(String benchmarkID) {
		if(benchmarkID.equals("gfxbench")) {
				appinfo_featureset_major = 5;
				appinfo_featureset_minor = 0;
		} else if(benchmarkID.contains("gfxbench")) {
			if(appinfo_gles_aep_compatible) {
				appinfo_featureset_major = 4;
				appinfo_featureset_minor = 0;
			} else {
				appinfo_featureset_major = appinfo_gles_major;
				appinfo_featureset_minor = Math.max(0, Math.min(1, appinfo_gles_minor));
			}
			
		} else if(benchmarkID.contains("compubench")) {
			appinfo_featureset_major = 1;
			appinfo_featureset_minor = 0;
			
		} else {
			appinfo_featureset_major = 1;
			appinfo_featureset_minor = 0;
		}
	}
	
	private void setSyncProps() {
		if (appinfo_versioncode < 40000 || (!appinfo_benchmark_id.contains("gfxbench_gl") && !appinfo_benchmark_id.equals("gfxbench"))) {
			syncProps.add("etc1");
			syncProps.add("etc2");
			syncProps.add("es2_etc1");
			
			if (appinfo_gles_major >= 3) {
				syncProps.add("es3");
				syncProps.add("es3_etc2");
				
				if(appinfo_gles_minor >= 1) {
					syncProps.add("es31");
					syncProps.add("es31_etc2");
					
					if(appinfo_gles_aep_compatible) {
						syncProps.add("es31_aep");
						syncProps.add("es31_aep_astc");
					}
				} 
			}
	
			syncProps.add("mobile");
			
		} else {
			if (appinfo_benchmark_id.equals("gfxbench")) {
				syncProps.add("mobile");

				// gfxb_5.cpp chooses ASTC over ETC2 if available
				if (any_vulkan_device_needs_astc || (appinfo_gles_supports_aztec && gles_supports_astc)) {
					syncProps.add("aztec_astc");
				}

				if (any_vulkan_device_needs_etc2 || (appinfo_gles_supports_aztec && !gles_supports_astc)) {
					syncProps.add("aztec_etc2");
				}
			}

			syncProps.add("common_etc1");
			
			if(appinfo_featureset_major <= 2) {
				syncProps.add("es2");
				syncProps.add("es2_etc1");
			}
			
			if (appinfo_gles_major >= 3) {
				syncProps.add("es3");
				syncProps.add("es3_etc2");
				
				if(appinfo_gles_minor >= 1) {
					syncProps.add("es31");
					syncProps.add("es31_etc2");
					
					if(appinfo_gles_aep_compatible) {
						syncProps.add("es31_aep");
						syncProps.add("es31_aep_astc");
					}
				} 
			}
			
			
		}
	}
	
	public MinimalProps(Context context, Boolean cmd) {
		if(!cmd) {
			props.collect(new AndroidDeviceInfoCollector(context), sysinf);
			
			appinfo_gles_major = sysinf.getGlesInfo().getMajorVersion();
			appinfo_gles_minor = sysinf.getGlesInfo().getMinorVersion();
			
			if(sysinf.getClInfo().getPlatforms().size() != 0)
			{
				appinfo_opencl_major = sysinf.getClInfo().getPlatforms().get(0).getClMajor();
				appinfo_opencl_minor = sysinf.getClInfo().getPlatforms().get(0).getClMinor();
			}
			
			if(sysinf.getVulkanInfo().getDevices().size() != 0)
			{
				appinfo_vulkan_major = sysinf.getVulkanInfo().getDevices().get(0).getMajor_vulkan_version();
				appinfo_vulkan_minor = sysinf.getVulkanInfo().getDevices().get(0).getMinor_vulkan_version();

				int num_vulkan_devices = (int)sysinf.getVulkanInfo().getDevices().size();
				for (int i = 0; i < num_vulkan_devices; i++) {
					if (sysinf.getVulkanInfo().getDevices().get(0).getSupportsASTC()) {
						any_vulkan_device_needs_astc = true;
					} else if (sysinf.getVulkanInfo().getDevices().get(0).getSupportsETC2()) {
						any_vulkan_device_needs_etc2 = true;
					}
				}
			}

			StringVector extensions = sysinf.getGlesInfo().getExtensions();
            appinfo_gles_aep_compatible = false;
			boolean has_KHR_texture_compression_astc_ldr = false;
			boolean has_EXT_draw_buffers_indexed = false;
			boolean has_EXT_tessellation_shader = false;
			boolean has_EXT_texture_cube_map_array = false;
			boolean has_EXT_geometry_shader = false;
			boolean has_EXT_gpu_shader5 = false;
			boolean has_EXT_shader_io_blocks = false;
			has_EXT_color_buffer_float = false;
			boolean has_ANDROID_extension_pack_es31a = false;
			for (int i = 0; i < extensions.size(); i++) {
				String ext = extensions.get(i);
				if(ext.contains("KHR_texture_compression_astc_ldr"))
					has_KHR_texture_compression_astc_ldr = true;
				if(ext.contains("EXT_draw_buffers_indexed"))
					has_EXT_draw_buffers_indexed = true;
				if(ext.contains("EXT_tessellation_shader"))
					has_EXT_tessellation_shader = true;
				if(ext.contains("EXT_texture_cube_map_array"))
					has_EXT_texture_cube_map_array = true;
				if(ext.contains("EXT_geometry_shader"))
					has_EXT_geometry_shader = true;
				if(ext.contains("EXT_gpu_shader5"))
					has_EXT_gpu_shader5 = true;
				if(ext.contains("EXT_shader_io_blocks"))
					has_EXT_shader_io_blocks = true;
				if(ext.contains("EXT_color_buffer_float"))
					has_EXT_color_buffer_float = true;
				if(ext.contains("ANDROID_extension_pack_es31a"))
					has_ANDROID_extension_pack_es31a = true;
				if(ext.contains("texture_compression_astc_ldr"))
					gles_supports_astc = true; // same logic as in ngl's ogl.cpp
			}
			appinfo_gles_aep_compatible = 
                    has_ANDROID_extension_pack_es31a || (
					has_KHR_texture_compression_astc_ldr &&
					has_EXT_draw_buffers_indexed &&
					has_EXT_tessellation_shader &&
					has_EXT_texture_cube_map_array &&
					has_EXT_geometry_shader &&
					has_EXT_gpu_shader5 &&
					has_EXT_shader_io_blocks);

			appinfo_gles_supports_aztec = 
                appinfo_gles_major >= 3 && 
                appinfo_gles_minor >= 1 &&
                has_EXT_color_buffer_float;

		} else {
			appinfo_gles_major = 100;
			appinfo_gles_minor = 0;
			appinfo_gles_aep_compatible = true;
		}
		
		PackageManager pm = context.getPackageManager();
		appinfo_packagename = context.getPackageName();
		appinfo_installername = String.format("%s", pm.getInstallerPackageName(context.getPackageName()));
		appinfo_locale = Locale.getDefault().toString();
		device_name = String.format("%s %s (%s)", Build.BRAND, Build.MODEL, Build.DEVICE);
		appinfo_storename = context.getString(R.string.app_store_name);
		appinfo_benchmark_id = context.getResources().getString(R.string.app_product_id);
		appinfo_version = null;
		appinfo_versioncode = 30000;
		try {
			appinfo_version = pm.getPackageInfo(context.getPackageName(), 0).versionName;
		} catch (NameNotFoundException e) {
			throw new RuntimeException(e);
		}
		
		try {
			PackageInfo pInfo = context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
			appinfo_versioncode = pInfo.versionCode;
		} catch (NameNotFoundException e) {
			Log.i("Sync warning", "App version code not found.");
		}

		this.setFeatureSetBasedOnBenchmarkId(appinfo_benchmark_id);
		this.setSyncProps();

		appinfo_testjson = "testlist_corporate.json";
		if(!appinfo_packagename.contains("corporate"))
			appinfo_testjson = String.format("testlist_%d.%d.json", appinfo_featureset_major, appinfo_featureset_minor);
		
		if(appinfo_packagename.contains("compubench") || appinfo_packagename.contains("adas"))
			appinfo_testjson = "config/benchmark_tests.json";
	}
}
