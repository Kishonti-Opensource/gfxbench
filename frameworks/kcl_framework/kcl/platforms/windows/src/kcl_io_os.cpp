/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_io.h"


#define NOMINMAX
#define NOCOMM
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <direct.h>
#include <io.h>
#include "winapifamilynull.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <string>
#include <sstream>

using namespace std;

//create directory hierarchy if needed
void KCL::File::OS_PreOpen()
{
	char dirname[1024] = {0};
	int savepos = 0;
	if(m_mode==KCL::Write || m_mode==KCL::Append)
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
			dirname[savepos + 1] = 0;

			if (GetFileAttributes(dirname) == INVALID_FILE_ATTRIBUTES)
			{
				for(unsigned int i = 0; i < m_filename.length(); i++)
				{
					if(m_filename[i]=='\\' || m_filename[i]=='/')
					{
						savepos = i;
						memset(dirname, 0, sizeof(dirname));
						strncpy(dirname, m_filename.c_str(), savepos + 1);
						dirname[savepos +1] = 0;
						if(_mkdir( dirname ))
						{
							INFO("Directory created.");
						}
					}
				}
			}
		}
	}
}
#else
void KCL::File::OS_PreOpen()
{
}
#endif


int KCL::File::MkDir(const char* dirname)
{
	return _mkdir(dirname);
}

int KCL::File::ListDir(std::vector<std::string> & result, int types, const char * path, const char * pattern)
{	
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	std::string filename;
	if (pattern)
	{
		std::stringstream sstream;
		sstream << path;
		sstream << pattern;
		filename = sstream.str();
	}
	else
	{
		filename = path;
	}
	
	_finddata_t list;
	intptr_t find_result;	
	find_result = _findfirst( filename.c_str(), &list);

    if (find_result != -1L)
	{
		do
		{
			if (types & KCL::FILETYPE_Directory && list.attrib & _A_SUBDIR)
			{
				result.push_back(list.name);
			}
			else if (types & KCL::FILETYPE_File && !(list.attrib & _A_SUBDIR))
			{
				result.push_back(list.name);
			}
		}
		while(_findnext(find_result,&list)==0);
		_findclose(find_result);
	}
	return 1;
#else
	return 0;
#endif
}
