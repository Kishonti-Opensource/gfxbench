/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_io.h"

#include <cstdlib>
#include <cstring>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "platform/platform_android.h"  // from project: platform_utils

void mkpath(const std::string& path)
{
	std::string lpath = path;
	std::string sum_path;
	std::string dirname;

	while(1)
	{
		int pos = 0;
		pos = lpath.find("/");


		if (pos == std::string::npos)
		{
			break;
		}
		else
		{
			dirname = lpath.substr(0, pos);
			lpath = lpath.substr(pos + 1, path.length());
			sum_path += dirname + "/";
			KCL::File::MkDir(sum_path.c_str());
		}
	}
}


void KCL::File::OS_PreOpen()
{
	if(m_mode==KCL::Write || m_working_dir==KCL::RWDir)
	{
		INFO("OS_PreOpen %s", m_filename.c_str());
		return;
	}

	FILE *fp = fopen(m_filename.c_str(), "rb");
	if(fp)
	{
		fclose(fp);
		return;
	}

	AAssetManager *mgr = kishonti::android::Platform::instance()->assetManager();
	if(mgr)
	{
		AAsset* asset = AAssetManager_open(mgr, (std::string("data/gfx/") + m_original_filename).c_str(), AASSET_MODE_UNKNOWN);
        if (!asset)
        {
            return;
        }
        
		m_filesize = AAsset_getLength(asset);
		m_buffer = new char[m_filesize + 1];
		m_p = m_buffer;
		m_is_filemapped = 1;
		AAsset_read(asset, m_buffer, m_filesize);
		m_buffer[m_filesize] = 0;
		AAsset_close(asset);

		mkpath(m_filename.c_str());
		FILE *f = fopen(m_filename.c_str(), "wb");
		if(f)
		{
			size_t written = fwrite(m_buffer, m_filesize, 1, f);
			if(written != 1)
			{
				INFO("Coudn't save the file to the target path (%s)!", m_filename.c_str());
				m_error = KCL_IO_FAILED_TO_WRITE;
			}
			fclose(f);
		}
	}
	else
	{
	//	INFO("Failed to get AssetManager!");
	}
}


int KCL::File::MkDir(const char* dirname)
{
	return mkdir(dirname,  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int KCL::File::ListDir(std::vector<std::string> & result, int types, const char * path, const char * pattern)
{
	return 0;
}
