/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_IO_H
#define KCL_IO_H

#include "kcl_os.h"

#include <string.h>
#include <assert.h>
#include <cstdio>
#include <string>
#include <vector>

namespace KCL
{
        struct keyStates
        {
            std::vector<bool> isPressed;
            std::vector<bool> wasPressed;
            std::vector<bool> isToggled;

            void Init(unsigned int size)
            {
                isPressed.reserve(size);
                wasPressed.reserve(size);
                isToggled.reserve(size);

                isPressed.assign(size, false);
                wasPressed.assign(size, false);
                isToggled.assign(size, false);
            }

            void SwitchKeyStates()
            {
                wasPressed = isPressed;
            }
        };


		enum WORKING_DIRERCTORY
		{
			BaseDir,
			RDir,
			RWDir
		};

		enum OPENMODE
		{
			Read,
			Write,
			Append
		};

		enum FILETYPE
		{
			FILETYPE_File = 1,
			FILETYPE_Directory = 2
		};

		enum IO_ERRORS
		{
			KCL_IO_NO_ERROR = 0,
			KCL_IO_NO_FILE_EXISTS = -1,
			KCL_IO_NO_ENOUGHT_MEMORY = -2,
			KCL_IO_NO_VALID_WOKRING_DIRECTORY = -3,
			KCL_IO_FAILED_TO_WRITE = -4
		};

		class IOException : public std::exception
		{
		protected:
			std::string m_msg;
		public:
			IOException(const std::string& msg)
			{
				m_msg = msg;
			}
			virtual ~IOException() throw() {}
			const char* what() const throw()
			{
				return m_msg.c_str();
			}
		};

		class FileNotExistsException : public IOException
		{
		public:
			FileNotExistsException(const std::string& filename) : IOException( std::string("The file does not exist=") + filename ) {}
		};

		class FileNotEnoughtMemory: public IOException
		{
		public:
			FileNotEnoughtMemory(const char* msg = "KCL_IO_NO_ENOUGHT_MEMORY") : IOException(std::string(msg)){}
		};

		class FileNotValidWorkingDirectoryException : public IOException
		{
		public:
			FileNotValidWorkingDirectoryException(const std::string& dirname) : IOException(std::string("Invalid working directory:") + dirname)
			{
			}
		};

		class File
		{
		public:
			File(const std::string& filename, const KCL::OPENMODE& mode, const KCL::WORKING_DIRERCTORY& path, const bool no_throw = false);
			virtual ~File();


			char* GetBuffer();
			void FreeBuffer();
			long GetLength();
			int GetLastError();

			//low level functions
			std::string getFileExtension();
			FILE* getFilePointer();
			const std::string getFilename();

			char* ReadString();
			std::string ReadToStdString();
			size_t Read(void* data, size_t size, size_t count);
			char* Gets(char* data, int n);
			char Getc();
			
			size_t Write(const std::string &str);
			size_t Write(const void* data, size_t size, size_t count);
			int Printf(const char* format, ...);
			int Seek(int offset, int origin);
			long Tell();
			int eof();
			void Close();
			void Remove();

			bool Opened();

			static bool Exists(const std::string &filename);
			static int MkDir(const char* dirname);
			static int ListDir(std::vector<std::string> & result, int types, const char * path, const char * pattern = NULL);
			
			static void ClearScenePath();

			static void AddScenePath(const std::string& path);
			static void SetDataPath(const std::string& path);//obsolete
			static void SetRWDataPath(const std::string& path);
			std::string GetFileLocation();

			static const std::string GetScenePath(const int path = 0);
			static KCL::uint32 GetScenePathCount();
			static const std::string& GetDataPath();
			static const std::string& GetDataRWPath();
            static FILE* Utf8fopen(const std::string& str, const std::string& mode);

			void OS_PreOpen();
		protected:
			File(){}

			void map_file();

			std::string m_filename;
			std::string m_original_filename;
			KCL::WORKING_DIRERCTORY m_working_dir;

			FILE* m_file_pointer;
			long m_filesize;
			KCL::IO_ERRORS m_error;
			char *m_buffer;
			char *m_p;
			bool m_is_filemapped;

			KCL::OPENMODE m_mode;
			void open_file_(bool no_throw = false);
			void close_file_();

			static std::vector<std::string> m_scene_path;
			static std::string m_rpath;//basepath
			static std::string m_rwpath;

			static void CheckPathValidity(std::string &path);
		};

		class AssetFile : public File
		{
		public:
			AssetFile(const std::string& filename, const bool no_throw = true);
			~AssetFile();

		protected:
		private:
			AssetFile(const AssetFile& source);
			AssetFile& operator= (const AssetFile& res);
		};
}//namespace KCL



inline size_t KCL::File::Read(void* data, size_t esize, size_t count)
{
	assert((m_file_pointer || m_is_filemapped));
	
	if(esize == 0 || count ==0)
	{
		return 0;
	}

	if(m_is_filemapped)
	{
		size_t len = m_p - m_buffer + esize * count;
		if(m_p - m_buffer  >= m_filesize)
		{
			return 0;
		}
		if(len > (unsigned)m_filesize )
		{
			count = m_filesize - (m_p - m_buffer);
			memset(data, 0, count);
			memcpy(data, m_p, count);
			m_p += count;
			return count;
		}
		memset(data, 0, esize * count);
		memcpy(data, m_p, esize * count);
		m_p += esize * count;
		return count;
	}
	else
	{
		return fread(data, esize, count, m_file_pointer);
	}
}

#endif
