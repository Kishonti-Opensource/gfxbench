/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.initialization;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;

import org.apache.commons.io.FileUtils;

import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;
import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.R;
import net.kishonti.benchui.model.MinimalProps;
import net.kishonti.swig.ApiDefinition;
import net.kishonti.swig.EGLGraphicsContext;
import net.kishonti.swig.StringVector;
import net.kishonti.systeminfo.swig.Properties;
import net.kishonti.theme.Localizator;

public class CollectDeviceInfoTask extends InitTask {

	public static final String KEY_SYSTEMINFO = "systeminfo";

	public CollectDeviceInfoTask(Context context) {
		super(context, Localizator.getString(context, "LoadingDeviceInfo"), 1);
	}
	
	public Result run(HashMap<String, Object> params) {
		MinimalProps minimalProps = BenchmarkApplication.getMinimalProps(mContext);
		ApiDefinition gles = new ApiDefinition();
		gles.setType(ApiDefinition.Type.ES);
		gles.setMajor(minimalProps.appinfo_gles_major);
		gles.setMinor(minimalProps.appinfo_gles_minor);
        if (minimalProps.has_EXT_color_buffer_float
            || minimalProps.appinfo_gles_aep_compatible)
        {
            StringVector ext = new StringVector();
            if (minimalProps.has_EXT_color_buffer_float) {
                ext.add("GL_EXT_color_buffer_float");
            }
            if (minimalProps.appinfo_gles_aep_compatible) {
                ext.add("GL_ANDROID_extension_pack_es31a");
            }
            gles.setExtensions(ext);
        }
		
		BenchmarkApplication.getApiDefinitions().add(gles);
		
		List<String> openclConfigs = net.kishonti.systeminfo.DataCollector.getCBJNIOpenCLDeviceList();
        
		if(minimalProps.sysinf.getHasCl()) {
			ApiDefinition cl = new ApiDefinition();
			cl.setType(ApiDefinition.Type.CL);
			cl.setMajor(minimalProps.appinfo_opencl_major);
			cl.setMinor(minimalProps.appinfo_opencl_minor);
			
			BenchmarkApplication.getApiDefinitions().add(cl);
		}
		
		if(minimalProps.sysinf.getHasCuda()) {
	        ApiDefinition cuda = new ApiDefinition();
	        cuda.setType(ApiDefinition.Type.CUDA);
	        cuda.setMajor(3);
	        cuda.setMinor(0);
	        BenchmarkApplication.getApiDefinitions().add(cuda);
		}
		
		if(minimalProps.sysinf.getHasVulkan()) {
	        ApiDefinition vulkan = new ApiDefinition();
	        vulkan.setType(ApiDefinition.Type.VULKAN);
	        vulkan.setMajor(minimalProps.appinfo_vulkan_major);
	        vulkan.setMinor(minimalProps.appinfo_vulkan_minor);
	        BenchmarkApplication.getApiDefinitions().add(vulkan);
		}

		//TODO CL info
//		if(props.has("api/cl/platform_count")) {
//			int cl_platform_count = props.get("api/cl/platform_count").getInt();
//			String cl_platform_version = props.get("api/cl/platforms/0/version").getString();
//			String cl_platform_name = props.get("api/cl/platforms/0/name").getString();
//			
//		}
		
		Properties props = minimalProps.props;

		try {
			String propsString = props.toJsonString();
			Log.i("Device Info", propsString);
			params.put(KEY_SYSTEMINFO, props);
			FileUtils.write(new File(mContext.getCacheDir().getPath() + "/props.json"), propsString);
			Log.d("Device Info", "Properties succesfully saved");
		} catch (Exception e) {
			e.printStackTrace();
			return new Result(false, "Device info collection fail: " + e.getMessage());
		}

		PackageManager pm = mContext.getPackageManager();
		props.setString("appinfo/packagename", mContext.getPackageName());
		props.setString("appinfo/installername", String.format("%s", pm.getInstallerPackageName(mContext.getPackageName())));
		props.setString("appinfo/benchmark_id", mContext.getString(R.string.app_product_id));
        props.setString("appinfo/storename", mContext.getString(R.string.app_store_name));
		props.setString("appinfo/locale", Locale.getDefault().toString());
		props.setString("appinfo/platform", "android");
		try {
			props.setString("appinfo/version", pm.getPackageInfo(mContext.getPackageName(), 0).versionName);
		} catch (NameNotFoundException e) {
			throw new RuntimeException(e.getLocalizedMessage());
		}
		
		BenchmarkApplication.getModel(mContext).setupConfigurations();

		return new Result(true, Localizator.getString(mContext, "LoadingDeviceInfo") + " " + Localizator.getString(mContext, "OK"));
	}

}
