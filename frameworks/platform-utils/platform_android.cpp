/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "platform/platform_android.h"
#include <android/log.h>
#include <stdexcept>
#include <cstdlib>  // atexit
#include <android/log.h>

static const int BATTERY_STATUS_CHARGING = 2;
static const int BATTERY_STATUS_FULL = 5;

#define  LOG_TAG    "platform-utils"

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


JavaVM *cached_jvm = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
	cached_jvm = jvm;
	return JNI_VERSION_1_2;
}

JNIEnv *JNU_GetEnv()
{
	JNIEnv *env = 0;
	jint rc = cached_jvm->GetEnv((void **)&env, JNI_VERSION_1_2);
	if (rc == JNI_EDETACHED)
	{
		JavaVMAttachArgs attach_args = { JNI_VERSION_1_2, NULL, NULL };
		rc = cached_jvm->AttachCurrentThread(&env, &attach_args);
		if (rc != 0)
		{
			throw std::runtime_error("jni attach thread failed");
		}
	}
	if (rc == JNI_EVERSION)
	{
		throw std::runtime_error("jni version not supported");
	}
	return env;
}

#if 0
void doTest()
{
	using namespace kishonti::android;
	Platform *p = Platform::instance();
	LOGD("battery: %f", Platform::batteryLevel());
	AAssetManager *mgr = p->assetManager();
	AAssetDir* dir = AAssetManager_openDir(mgr, "test");
	const char *name = AAssetDir_getNextFileName(dir);
	while (name != NULL)
	{
		LOGD("asset: %s", name);
		name = AAssetDir_getNextFileName(dir);
	}
	AAssetDir_close(dir);
}
#endif

JNIEXPORT void JNI_SETAPPLICATIONCONTEXT
{
	kishonti::android::Platform *p = kishonti::android::Platform::instance();
	if (p->context_ == NULL)
	{
		p->context_ = env->NewGlobalRef(context);
	}
	p->assetMgr_ = env->NewGlobalRef(assetMgr);
//	doTest();
}


namespace kishonti {
namespace android {

namespace detail
{
	Battery batteryInfo();
}

Platform *Platform::platform_ = 0;


Platform::Platform()
	: context_(0)
	, assetMgr_(0)
{
}

Platform::~Platform()
{
	if (context_ != 0)
	{
		env()->DeleteGlobalRef(assetMgr_);
		assetMgr_ = 0;
		env()->DeleteGlobalRef(context_);
		context_ = 0;
	}
}

Platform *Platform::instance()
{
	if (platform_ == 0)
	{
		platform_ = new Platform();
		atexit(Platform::release);
	}
	return platform_;
}

bool Platform::isJavaInitialized()
{
	return Platform::instance()->context()!=NULL;
}

void Platform::release()
{
	delete platform_;
	platform_ = 0;
}

JNIEnv *Platform::env()
{
	return JNU_GetEnv();
}

void Platform::detachThread()
{
	cached_jvm->DetachCurrentThread();
}

jobject Platform::context()
{
	return context_;
}

AAssetManager *Platform::assetManager()
{
	if(assetMgr_ != NULL)
	{
		return AAssetManager_fromJava(env(), assetMgr_);
	}
	return NULL;
}

float Platform::batteryTemperature()
{
	detail::Battery battery = detail::batteryInfo();
	return battery.temperature;
}

float Platform::batteryLevel()
{
	detail::Battery battery = detail::batteryInfo();
	return battery.level;
}

bool Platform::batteryCharging()
{
	detail::Battery battery = detail::batteryInfo();
	return battery.status == BATTERY_STATUS_CHARGING || battery.status == BATTERY_STATUS_FULL;
}
}
}
