/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_io.h>
#include <kcl_os.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <set>
#include <cassert>


std::vector<std::string> KCL::File::m_scene_path;
std::string KCL::File::m_rpath;
std::string KCL::File::m_rwpath;

KCL::keyStates g_keyStates;


KCL::File::File(const std::string& filename, const KCL::OPENMODE& mode, const WORKING_DIRERCTORY& path, const bool no_throw)
{
	m_original_filename = filename;
	m_mode = mode;
	m_error = KCL_IO_NO_ERROR;
	m_working_dir = path;
	m_buffer = 0;
	m_file_pointer = 0;
	m_filesize = -1;
	m_is_filemapped = false;

	if ((mode == KCL::Write || mode == KCL::Append) && path == KCL::RWDir)
	{
		m_filename = GetDataRWPath();
		m_filename += filename;
	}
	else
	{
		if ((mode == KCL::Write || mode == KCL::Append) && (path == KCL::RDir || path == KCL::BaseDir))
		{
			m_filename += filename;
		}
		else
		{
			for (int i = m_scene_path.size() - 1; i >= 0; --i)
			{
				if (path == KCL::RDir)
				{
					m_filename = GetScenePath(i);
				}

				if (m_filename.length() < 1)
				{
					m_error = KCL_IO_NO_VALID_WOKRING_DIRECTORY;
					if (!no_throw)
					{
						throw FileNotValidWorkingDirectoryException(m_filename);
					}
				}

				m_filename += filename;
				if (path == KCL::RDir)
				{
					OS_PreOpen();
					m_file_pointer = Utf8fopen(m_filename, "rb");
					if (m_file_pointer)
					{
						break;
					}
				}
			}
		}
	}

	if (mode == KCL::Read && !m_file_pointer)
	{
		m_filename = GetDataPath() + filename;
		OS_PreOpen();
		m_file_pointer = Utf8fopen(m_filename, "rb");
	}

	if (mode == KCL::Read)
	{
		if (!m_file_pointer)
		{
			m_error = KCL::KCL_IO_NO_FILE_EXISTS;
		}
		else
		{
			fclose(m_file_pointer);
			m_file_pointer = 0;
		}
	}
	else
	{
		open_file_(no_throw);
	}
}


KCL::File::~File()
{
	FreeBuffer();
	close_file_();
}


void KCL::File::FreeBuffer()
{
	delete[] m_buffer;
	m_buffer = 0;
	m_is_filemapped = false;
}


char* KCL::File::GetBuffer()
{
	if(m_file_pointer==0 && m_buffer==0)
	{
		return 0;
	}

	if(m_is_filemapped)
	{
		return m_buffer;
	}
	else
	{
		m_buffer = new char[m_filesize + 1];
		if(m_buffer)
		{
			size_t read_bytes = fread(m_buffer, 1, m_filesize, m_file_pointer);
			if (m_filesize != read_bytes)
			{
				return 0;
			}
			m_buffer[m_filesize] = 0;
			return m_buffer;
		}
		else
		{
			m_error = KCL_IO_NO_ENOUGHT_MEMORY;
			throw FileNotEnoughtMemory();
		}
	}
}


long KCL::File::GetLength()
{
	if(m_file_pointer && m_filesize == -1)
	{
		long orig_position = ftell(m_file_pointer);
		fseek(m_file_pointer, 0, SEEK_END);
		m_filesize = ftell(m_file_pointer);
		fseek(m_file_pointer, orig_position, SEEK_SET);
	}
	return m_filesize;
}


int KCL::File::GetLastError()
{
	return m_error;
}


bool KCL::File::Exists(const std::string &filename)
{
	File testFile(filename, KCL::Read, KCL::RDir);

    return testFile.m_error == KCL::KCL_IO_NO_ERROR;
}


void KCL::File::open_file_(bool no_throw)
{
	m_error = KCL_IO_NO_ERROR;

	std::string mode;
	switch (m_mode)
	{
	case KCL::Read:
		mode="rb";
		break;
	case KCL::Write:
		mode="wb";
		break;
	case KCL::Append:
		mode="ab";
		break;
	}

	OS_PreOpen();
	if(GetLastError())
	{
		return;
	}
	if(!m_buffer)
	{
		if (!m_file_pointer)
		{
			m_file_pointer = Utf8fopen(m_filename, mode);
		}

		if (!m_file_pointer)
		{
			m_error = KCL_IO_NO_FILE_EXISTS;
			if (no_throw==false)
			{
				throw FileNotExistsException(m_filename.c_str());
			}
		}
	}
	GetLength();
}


std::string KCL::File::getFileExtension()
{
	char ext[4] = {0};
	ext[0] = m_filename[m_filename.length()-3];
	ext[1] = m_filename[m_filename.length()-2];
	ext[2] = m_filename[m_filename.length()-1];
	return std::string(ext);
}


FILE* KCL::File::getFilePointer()
{
	return m_file_pointer;
}


const std::string KCL::File::getFilename()
{
	return m_original_filename;
}


char KCL::File::Getc()
{
	if(m_is_filemapped)
	{
		if(m_p - m_buffer + 1 > m_filesize)
		{
			return EOF;
		}

		char ch = *m_p++;
		return ch;
	}
	else
	{
		return fgetc(m_file_pointer);
	}
}


char* KCL::File::Gets(char* data, int n)
{
	if(m_is_filemapped)
	{
		if(n < 0)
		{
			return 0;
		}
		char *pp = data;
		while (--n)
        {
			signed char ch = Getc();
			if(ch==EOF)
			{
				if(pp==data)
				{
					return 0;
				}

				break;
			}
			*pp++ = ch;
			if(ch=='\n')
			{
				break;
			}
		}

		*pp = 0;

		return data;
	}
	else
	{
		assert(m_file_pointer);
		return fgets(data, n, m_file_pointer);
	}
}


std::string KCL::File::ReadToStdString()
{
	assert((m_file_pointer || m_is_filemapped));

	std::string s;
	s.resize(m_filesize + 1);
	if (m_is_filemapped)
	{
		m_p = m_buffer;
		s[m_filesize] = 0;
		memcpy((char*)s.data(), m_p, m_filesize);
	}
	else
	{
		fseek(m_file_pointer, 0, SEEK_SET);
		fread((char*)s.data(), m_filesize, 1, m_file_pointer);
	}

	return s;
}


char* KCL::File::ReadString()
{
	abort();
	return 0;
}


size_t KCL::File::Write(const std::string &str)
{
	if (str.empty())
	{
		return 0;
	}
	return Write(str.c_str(), str.size(), 1);
}


size_t KCL::File::Write(const void* data, size_t size, size_t count)
{
	assert(m_file_pointer);
	ftell(m_file_pointer);
	return fwrite(data, size, count, m_file_pointer);
}


int KCL::File::Printf(const char* format, ...)
{
	va_list args;
	va_start( args, format );

	int len = vsnprintf( 0, 0, format, args);
	va_end (args);


	char *buffer = new char[len + 1];
	va_start( args, format );
	vsprintf (buffer, format, args);

	int r = fprintf(m_file_pointer, "%s", buffer);
	delete []buffer;
	va_end(args);
	return r;
}


//If successful, the function returns zero.
//Otherwise, it returns non - zero value.
int KCL::File::Seek(int offset, int origin)
{
	if(m_is_filemapped)
	{
		if(origin == SEEK_CUR || origin == SEEK_SET || origin == SEEK_END)
		{
			if(origin == SEEK_CUR)
			{
			}
			else if(origin == SEEK_SET)
			{
				m_p = m_buffer;
			}
			else if(origin == SEEK_END)
			{
				m_p = m_buffer;
				m_p += m_filesize;
			}
			if( m_p - m_buffer + offset >= m_filesize)
			{
				return -1;
			}
			m_p += offset;
			return 0;
		}
	}
	else
	{
		return fseek(m_file_pointer, offset, origin);
	}
	return -1;
}


bool KCL::File::Opened()
{
	return m_is_filemapped || m_file_pointer;
}


int KCL::File::eof()
{
	if(m_is_filemapped)
	{
		return (m_p + 1) - m_buffer  >= m_filesize;
	}
	assert(m_file_pointer);
	return feof(m_file_pointer);
}


long KCL::File::Tell()
{
	if(m_is_filemapped)
	{
		return m_p - m_buffer;
	}
	else
	{
		return ftell(m_file_pointer);
	}
}


void KCL::File::Close()
{
	FreeBuffer();
	close_file_();
}


void KCL::File::Remove()
{
	Close();
	::remove(m_filename.c_str());
}


void KCL::File::map_file()
{
	if(m_file_pointer)
	{
		m_error = KCL_IO_NO_ERROR;
		m_buffer = new char[m_filesize + 1];
		if(m_buffer)
		{
			fread(m_buffer, 1, m_filesize, m_file_pointer);
			m_buffer[m_filesize] = 0;
		}
		else
		{
			m_error = KCL_IO_NO_ENOUGHT_MEMORY;
			throw FileNotEnoughtMemory();
		}
		m_p = m_buffer;
		m_is_filemapped = true;
		fclose(m_file_pointer);
		m_file_pointer = 0;
	}
}


void KCL::File::close_file_()
{
	if(m_file_pointer)
	{
		fclose(m_file_pointer);
		m_file_pointer = NULL;
	}
}


FILE* KCL::File::Utf8fopen(const std::string& filename, const std::string& mode)
{
#ifndef _WIN32
    return fopen(filename.c_str(), mode.c_str());
#else
    std::wstring wide_filename(filename.size(), L' ');
    int result = MultiByteToWideChar(
            CP_UTF8,
            0,
            filename.c_str(), filename.size(),
            &wide_filename[0], wide_filename.size());
    if (result <= 0)
    {
        throw FileNotExistsException(filename);
    }
    wide_filename.resize(result);

    std::wstring wide_mode(mode.size(), L' ');
    result = MultiByteToWideChar(
        CP_UTF8,
        0,
        mode.c_str(), mode.size(),
        &wide_mode[0], wide_mode.size());
    if (result <= 0)
    {
        throw FileNotExistsException(filename);
    }
    wide_mode.resize(result);

    return _wfopen(wide_filename.c_str(), wide_mode.c_str());
#endif
}


KCL::AssetFile::AssetFile(const std::string& filename, const bool no_throw)
	:File(filename, KCL::Read, KCL::RDir)
{
	if (m_mode == KCL::Read && m_error)
	{
		return;
	}
	open_file_(no_throw);
	if(Opened())
	{
		map_file();
		close_file_();
	}
}


KCL::AssetFile::~AssetFile()
{
}


KCL::AssetFile::AssetFile(const AssetFile& source)
{
	abort();
}


KCL::AssetFile& KCL::AssetFile::operator= (const AssetFile& res)
{
	abort();
	return *this;
}


void KCL::File::CheckPathValidity(std::string &path)
{
	if (path.length() > 0 && path[path.length() - 1] != '/')
	{
		path += "/";
	}
}


void KCL::File::ClearScenePath()
{
	m_scene_path.clear();
	m_rpath = "";
	m_rwpath = "";
}


void KCL::File::AddScenePath(const std::string& path)
{
	m_scene_path.push_back(path);
	CheckPathValidity(m_scene_path.back());
}


void KCL::File::SetDataPath(const std::string& path)
{
	m_rpath = path;
	CheckPathValidity(m_rpath);
}


void KCL::File::SetRWDataPath(const std::string& path)
{
	m_rwpath = path;
	CheckPathValidity(m_rwpath);
}


const std::string& KCL::File::GetDataPath()
{
	return m_rpath;
}


std::string KCL::File::GetFileLocation()
{
	return m_filename;
}


const std::string KCL::File::GetScenePath(const int path)
{
	assert(!(m_rpath.size() < 1));
	if (path > m_rpath.size())
	{
		INFO("Error: Not existing path (max: %d req:%d)", m_rpath.size(), path);
		int* p = (int*)&path;
		*p = 0;
	}
	if (m_scene_path[path].length() < 1)
	{
		int* p = (int*)&path;
		*p = 0;
	}
	if (m_scene_path[path][0] == '/' || m_scene_path[path][0] == '\\')
	{
		INFO("Path is absolute!");
	}
	return m_rpath + m_scene_path[path];
}


KCL::uint32 KCL::File::GetScenePathCount()
{
	return (KCL::uint32)m_scene_path.size();
}


const std::string& KCL::File::GetDataRWPath()
{
	return m_rwpath;
}
