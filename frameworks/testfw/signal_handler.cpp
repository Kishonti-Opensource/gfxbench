/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "signal_handler.h"
#include <cstring>
#include <stdio.h>

namespace {
	SignalHandler theHandler_;
}

SignalHandler::SignalHandler()
{
	struct sigaction handler;
	memset(&handler, 0, sizeof(handler));
	handler.sa_sigaction = my_sigaction;
	handler.sa_flags = SA_RESETHAND;
#define CATCHSIG(X) sigaction(X, &handler, &old_sa_[X]);
	CATCHSIG(SIGILL);
	CATCHSIG(SIGABRT);
	CATCHSIG(SIGBUS);
	CATCHSIG(SIGFPE);
	CATCHSIG(SIGSEGV);
	CATCHSIG(SIGPIPE);
}

#include <jni.h>

#ifdef ANDROID
#include <android/log.h>
#endif

JNIEnv *JNU_GetEnv(); // defined in the swig interface file
jclass JNU_GetNativeRecieverClass(); // defined in the swig interface file
jmethodID JNU_GetNativeRecieverMethod(); // defined in the swig interface file
void SignalHandler::my_sigaction(int sid, siginfo_t *, void *)
{
#ifdef ANDROID
	__android_log_print(ANDROID_LOG_ERROR, "NativeLog", "signal received: %d", sid);
#endif
	JNIEnv *jenv = JNU_GetEnv();
	jclass jc = jenv->FindClass("net/kishonti/NativeSignalReceiver");
	if (jc)
	{
		jmethodID mid = jenv->GetStaticMethodID(jc, "nativeSignalReceived", "(I)V");
		if (mid)
		{
			jenv->CallStaticVoidMethod(jc, mid, sid);
		}
		else {
#ifdef ANDROID
			__android_log_print(ANDROID_LOG_ERROR, "NativeLog", "nativeSignalReceived method not found");
#endif
		}
	} else {
#ifdef ANDROID
		__android_log_print(ANDROID_LOG_ERROR, "NativeLog", "NativeSignalReceiver not found");
#endif
	}
#ifdef ANDROID
	__android_log_print(ANDROID_LOG_ERROR, "NativeLog", "signal handled - java called");
	__android_log_print(ANDROID_LOG_ERROR, "NativeLog", "calling old signal handler: %p ", (void*)theHandler_.old_sa_[sid].sa_handler);
	theHandler_.old_sa_[sid].sa_handler(sid);
	__android_log_print(ANDROID_LOG_ERROR, "NativeLog", "signal handled - orig handler called");
	__android_log_print(ANDROID_LOG_ERROR, "NativeLog", "signal handled. exiting...");
#endif
}
