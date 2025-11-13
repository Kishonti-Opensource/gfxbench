/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "platform/platform_android.h"
#include "ScopedLocalRef.h"
#include <stdint.h>
#include <cstdio>
#include <dirent.h>
#include <cstring>

static const int BATTERY_STATUS_UNKNOWN = 1;
static const int BATTERY_STATUS_CHARGING = 2;
static const int BATTERY_STATUS_DISCHARGING = 3;
static const int BATTERY_STATUS_NOT_CHARGING = 4;
static const int BATTERY_STATUS_FULL = 5;

namespace kishonti {
namespace android {
namespace detail {

int readIntFromFile(char* fname)
{
	int i = -1;
	FILE* f = fopen(fname,"r");
	if (!f) return i;
	fscanf(f,"%d",&i);
	fclose(f);
	return i;
}

bool readStringFromFile(char* fname, char* str, int size)
{
	FILE* f = fopen(fname,"r");
	if (!f) return false;
	char* out=fgets(str,size,f);
	fclose(f);
	return out!=NULL;
}

char battery_filename_status[250];
char battery_filename_temp[250];
char battery_filename_capacity[250];

bool battery_files_initialized = false;

const char power_root[] =  "/sys/class/power_supply/";

void InitializeBatteryFiles()
{
	battery_filename_status[0]=0;
	battery_filename_temp[0]=0;
	battery_filename_capacity[0]=0;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (power_root)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			char status_filename[250];
			char temp_filename[250];
			char capacity_filename[250];
			strcpy(status_filename, power_root);
			strcat(status_filename, ent->d_name);
			strcpy(temp_filename,status_filename);
			strcpy(capacity_filename,status_filename);
			strcat(status_filename,"/status");
			strcat(temp_filename,"/temp");
			strcat(capacity_filename,"/capacity");
			
			FILE *f=NULL;
			f=fopen(status_filename,"r");
			if (f)
			{
				strcpy(battery_filename_status, status_filename);
				fclose(f);
			}
			f=fopen(temp_filename,"r");
			if (f)
			{
				strcpy(battery_filename_temp, temp_filename);
				fclose(f);
			}

			f=fopen(capacity_filename,"r");
			if (f)
			{
				strcpy(battery_filename_capacity, capacity_filename);
				fclose(f);
			}
		}
		closedir (dir);
	}
	battery_files_initialized = true;
}

Battery batteryInfo()
{
	Battery battery;
	battery.level = -1;
	battery.status = -1;
	battery.temperature = -1;

	if (!Platform::isJavaInitialized())
	{
		if (!battery_files_initialized) InitializeBatteryFiles();
		if (battery_filename_capacity[0]!=0) battery.level = readIntFromFile(battery_filename_capacity);
		if (battery_filename_temp[0]!=0) battery.temperature = readIntFromFile(battery_filename_temp);
		if (battery.temperature>0) battery.temperature/=10;
		char battery_status[20];
		if (battery_filename_status[0]!=0 && readStringFromFile(battery_filename_status,battery_status,20))
		{
			switch (battery_status[0])
			{
				case 'C': // Charging
					battery.status = BATTERY_STATUS_CHARGING;
					break;
				case 'D': // Discharging
					battery.status = BATTERY_STATUS_DISCHARGING;
					break;
				case 'N': // Not charging
					battery.status = BATTERY_STATUS_NOT_CHARGING;
					break;
				case 'F': // Full
					battery.status = BATTERY_STATUS_FULL;
					break;
				case 'U': // Unknown
				default:
					battery.status = BATTERY_STATUS_UNKNOWN;
					break;
			}
		}
		return battery;
	}

	Platform *p = Platform::instance();
	JNIEnv *env = p->env();
	jobject jcontext = p->context();
	ScopedLocalRef<jstring> jaction (env, env->NewStringUTF("android.intent.action.BATTERY_CHANGED"));

	ScopedLocalRef<jclass> clsIntentFilter(env, env->FindClass("android/content/IntentFilter"));
	jmethodID jintentFilterCstr = env->GetMethodID(clsIntentFilter.get(), "<init>", "(Ljava/lang/String;)V");
	ScopedLocalRef<jobject> jintentFilter(env, env->NewObject(clsIntentFilter.get(), jintentFilterCstr, jaction.get()));

	ScopedLocalRef<jclass> clsContext(env, env->FindClass("android/content/Context"));
	jmethodID registerReceiver = env->GetMethodID(clsContext.get(), "registerReceiver",
		"(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;");
	ScopedLocalRef<jobject> jintent(env, env->CallObjectMethod(jcontext, registerReceiver, 0, jintentFilter.get()));

	ScopedLocalRef<jclass> clsIntent(env, env->FindClass("android/content/Intent"));
	jmethodID getIntExtra = env->GetMethodID(clsIntent.get(), "getIntExtra", "(Ljava/lang/String;I)I");

	ScopedLocalRef<jstring> jlevel(env, env->NewStringUTF("level"));
	ScopedLocalRef<jstring> jscale(env, env->NewStringUTF("scale"));
	ScopedLocalRef<jstring> jstatus(env, env->NewStringUTF("status"));
	ScopedLocalRef<jstring> jtemperature(env, env->NewStringUTF("temperature"));

	float level = (float) env->CallIntMethod(jintent.get(), getIntExtra, jlevel.get(), -1);
	float scale = (float) env->CallIntMethod(jintent.get(), getIntExtra, jscale.get(), -1);
	battery.temperature = env->CallIntMethod(jintent.get(), getIntExtra, jtemperature.get(), -1);
	if (battery.temperature>0) battery.temperature/=10;
	battery.status = env->CallIntMethod(jintent.get(), getIntExtra, jstatus.get(), -1);
	battery.level = level / scale;
	return battery;
}

}}}
