/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef PLATFORM_ANDROID_H_
#define PLATFORM_ANDROID_H_

#include <jni.h>
#include <stdint.h>
#include <sys/types.h>
#include <android/asset_manager_jni.h>

#define JNI_SETAPPLICATIONCONTEXT Java_net_kishonti_platform_Platform_setApplicationContext(JNIEnv *env, jobject clazz, jobject context, jobject assetMgr)
extern "C" void JNI_SETAPPLICATIONCONTEXT;

#define APICALL __attribute__((visibility("default")))

namespace kishonti {
namespace android {

namespace detail
{
	struct Battery
	{
		float level;
		int status;
		float temperature;
	};
}

class APICALL Platform {
    friend void ::JNI_SETAPPLICATIONCONTEXT;
public:
    ~Platform();
    static Platform *instance();

    jobject context();
    AAssetManager *assetManager();
    JNIEnv *env();
    void detachThread();

    static float batteryTemperature();
    static float batteryLevel();
    static bool batteryCharging();
    static bool isJavaInitialized();
private:
    Platform();
    static void release();
    static Platform *platform_;
    jobject context_;
    jobject assetMgr_;
};

}
}

#endif  // PLATFORM_ANDROID_H_

