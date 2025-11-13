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
    char dirname[1024] = {0};
    int savepos = 0;
    if(m_mode==KCL::Write)
    {
	    for(unsigned int i = 0; i < m_filename.length(); i++)
	    {
		    if(m_filename[i]=='\\' || m_filename[i]=='/')
		    {
			    savepos = i;
		    }
	    }
	    if(savepos != 0)
	    {
		strncpy(dirname, m_filename.c_str(), savepos + 1);
		DIR *dir = opendir (dirname);

		  if (dir != NULL)
		  {
		      closedir (dir);
		  }
		  else
		  {
		     mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		  }
	    }  
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
