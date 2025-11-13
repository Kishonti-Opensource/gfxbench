/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <ppl.h>
#include <ppltasks.h>
#include <string>
#include <DirectXMath.h>
#include "kcl_io.h"

namespace IO
{
	struct ByteArray { std::vector<byte> data; };
	
	inline ByteArray ReadDataSync(std::string filename, std::string path)
	{
		/*if (path.size()==0)
			path = platformStringToStdString(Windows::ApplicationModel::Package::Current->InstalledLocation->Path);*/

		std::string fullpath = path + filename;

		KCL::AssetFile file(fullpath);
		if (file.GetLastError()) throw std::exception("File not found.");
		ByteArray ba;
		ba.data.resize(file.GetLength());
		memcpy(&ba.data[0], file.GetBuffer(), file.GetLength());
		
		return ba;
	}

	inline bool ExistsFile(std::string filename, std::string path)
	{
        std::string fullpath;
		if (path.size()==0)
		{
            // path = platformStringToStdString(Windows::ApplicationModel::Package::Current->InstalledLocation->Path);
            fullpath = path + "\\" + filename;
        }
        else
        {
            fullpath = path + filename;
        }
		
		FILE *fp;
		fp=fopen(fullpath.c_str(),"rb");
		if (fp!=NULL)
			fclose(fp);
		
		return (fp!=NULL);
	}
}