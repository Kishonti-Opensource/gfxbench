/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <cstdlib>
#include <cstring>
#include <string>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "kcl_osimpl_android.h" 
#include <sys/stat.h>
#include <sys/types.h>

extern AAssetManager* GLBAssetsManager;
extern std::string g_data_prefix;

namespace KCL {
	extern bool android_battery_charging;
	extern double android_battery_level;
}

#ifdef __cplusplus
extern "C" {
#endif

	JNIEXPORT void Java_net_kishonti_gfxbench_NativeInterface_setAssetManager(JNIEnv*  env,  jobject  thiz, jobject assetManager, jstring dataPrefix)
	{
		GLBAssetsManager = AAssetManager_fromJava(env, assetManager);

		jboolean iscopy;
		char* prefix = (char*)env->GetStringUTFChars( dataPrefix, &iscopy);
		g_data_prefix = prefix;
		env->ReleaseStringUTFChars(dataPrefix, prefix);
	}
	
	JNIEXPORT void Java_net_kishonti_gfxbench_NativeInterface_LowMemoryReceived(JNIEnv* env, jobject  thiz)
	{
		if (KCL::g_os)
		{
			((KCL::OSImpl*)KCL::g_os)->m_low_memory_flag = 1;
			INFO("Low memory received");
		} else {
			INFO("Low memory received, but no OSImpl instance available to handle it");
		}
	}

	JNIEXPORT void Java_net_kishonti_gfxbench_NativeInterface_ResetLowMemoryFlag(JNIEnv* env, jobject  thiz)
	{
		if(KCL::g_os)
		{
			INFO("Reseting low memory flag");
			((KCL::OSImpl*)KCL::g_os)->m_low_memory_flag = 0;
		} else {
			INFO("Can't reset low memory flag: No OSImpl instance");
		}
	}

	JNIEXPORT void Java_net_kishonti_gfxbench_NativeInterface_UpdateBatteryStatus(JNIEnv*  env,  jobject  thiz, jboolean isCharging, jfloat batteryPct)
	{
		KCL::android_battery_charging= isCharging;
		KCL::android_battery_level = batteryPct;
		if (KCL::g_os!=NULL)
		{
			KCL::g_os->SetBatteryCharging(isCharging);
			KCL::g_os->SetBatteryLevel(batteryPct);
		}
		INFO("Battery update called. Battery is %s and is at %.1f%% - KCL::g_os is %s",((bool)isCharging)?"charging":"not charging",((float)batteryPct)*100.0f,(KCL::g_os==NULL)?"null":"not null");
	}

#ifdef __cplusplus
}
#endif
