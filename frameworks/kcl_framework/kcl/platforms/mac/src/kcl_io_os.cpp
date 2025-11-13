/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_io.h"

#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

void KCL::File::OS_PreOpen()
{
    //std::string datadir = OSImpl::GetDataDirectory();
    
    //KCL::uint32 imageKey = adler32( 0, (const unsigned char *)fn, strlen( fn));
    
    //sprintf( fn2, "%sraw/z%x", datadir.c_str(), imageKey);
}

int KCL::File::MkDir(char const* a)
{
    return 0;
}

int KCL::File::ListDir(std::vector<std::string> & result, int types, const char * path, const char * pattern)
{
	return 0;
}
