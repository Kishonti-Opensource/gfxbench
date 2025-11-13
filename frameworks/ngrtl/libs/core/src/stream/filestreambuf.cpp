/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/stream/filestreambuf.h"

#include <limits>

#include "ng/require.h"
#include "ng/format.h"
#include "ng/log.h"
#ifdef UNDER_CE
#include "errno.h"
#else
#include <errno.h>
#endif

namespace ng
{
FileStreamBuf::
	FileStreamBuf()
	: _stream(NULL)
	, _bOwner(false)
{}

FileStreamBuf::
	FileStreamBuf(FILE* stream)
	: _stream(stream)
	, _bOwner(false)
{
	require(stream != NULL, "Constructed with FILE* stream = NULL");
}

FileStreamBuf::
	FileStreamBuf(const char* filename, const char* mode, OUT Result& result)
	: _stream(NULL)
	, _bOwner(false)
{
	open(filename, mode, OUT result);
}

FileStreamBuf::
	~FileStreamBuf()
{
	if ( _bOwner && _stream != NULL )
	{
		try {
			Result result;
			close(result);
			if (result.error())
				NGLOG_ERROR("FileStreamBuf::close error: %s", result.what());
		} catch(std::exception& e)
		{
			NGLOG_ERROR("FileStreamBuf::close exception: %s", e.what());
		} catch(...)
		{
			NGLOG_ERROR("FilestreamBuf::close unknown exception");
		}
	}
}

void FileStreamBuf::
	open(const char* filename, const char* mode, OUT Result& res)
{
	res.clear();
	if ( _bOwner )
	{
		NG_SET_RESULT_MSG(res, "FileStreamBuf::open called without first closing current file");
		return;
	}
	_stream = fopen(filename, mode);
	if ( !_stream )
	{
		NG_SET_RESULT_MSG_ERRNO(res, FORMATCSTR("failed: fopen(\"%s\", \"%s\")", filename, mode), errno);
		return;
	}
	_bOwner = true;
}

void FileStreamBuf::
	close(OUT ng::Result& res)
{
	res.clear();
	if ( !_bOwner )
	{
		NG_SET_RESULT_MSG(res, "FileStreamBuf::close(), FILE* not owned by this object");
		return;
	}
	int r = fclose(_stream);
	_stream = nullptr;
	_bOwner = false;
	if ( r != 0 )
		NG_SET_RESULT_MSG_ERRNO(res, "failed: fclose()", errno);
}

void FileStreamBuf::
	read(OUT void *data, size_t size, OUT size_t* nBytesRead)
{
	size_t s = fread(data, 1, size, _stream); //fread's size = 1, fread's count = 'size'!!
	if ( nBytesRead != NULL )
		*nBytesRead = s;
	else
		require(s == size, cstr(format("Failed to read %s bytes") % size));
}

void FileStreamBuf::
	write(const void *data, size_t size)
{
	size_t s = fwrite(data, 1, size, _stream);
	require(s == size, cstr(format("Failed to write %s bytes") % size));
}

void FileStreamBuf::
	seek32(int32_t offset, int origin)
{
	int r = fseek(_stream, offset, origin);
	require(r == 0, cstr(format("fseek returned error code: %s") % r ));
}

void FileStreamBuf::
	seek64(int64_t offset, int origin)
{
	int r;
	if ( sizeof(long) == 4 )
	{
#if defined UNDER_CE && defined _MSC_VER 
		r = fseek(_stream, offset, origin);
#elif defined _MSC_VER //also defined for win64
		r = _fseeki64(_stream, offset, origin);
#else
		require(false, "64 bit fseek not supported when long is 4 bytes");
#endif
	} else
		r = fseek(_stream, (long)offset, origin);
	require(r == 0, cstr(format("fseek returned error code: %s") % r ));
}

int32_t FileStreamBuf::
	tell32()
{
	long pos = ftell(_stream);
	require(pos >= 0, cstr(format("ftell returned error value: %s") % pos));
	if ( sizeof(long) > 4 )
		require(pos <= 0x7fffffff, cstr(format("can't return 64 bit value in 32 bits, position: %s") % pos));
	return (int32_t)pos;
}

int64_t FileStreamBuf::
	tell64()
{
	int64_t pos;
	if ( sizeof(long) == 4 )
	{
#if defined UNDER_CE && _MSC_VER
		pos = ftell(_stream);
#elif defined _MSC_VER //also defined for win64
		pos = _ftelli64(_stream);
#else
		require(false, "64 bit ftell not supported when long is 4 bytes");
#endif
	} else
		pos = ftell(_stream);
	require(pos >= 0, cstr(format("ftell returned error value: %s") % pos));
	return pos;
}

void FileStreamBuf::
	flush()
{
	int result = ::fflush(_stream);
	require(result == 0, cstr(format("fflush returned error value; %s") % result));
}

bool FileStreamBuf::isAtEof()
{
	char dummy = 0;
	size_t bytes_read = 0;
	read( &dummy, 1, &bytes_read);
	if (bytes_read != 1)
	{
		require(feof(_stream), "Read past end but feof() still 0");
		return true;
	}
	seek64(-1, SEEK_CUR);
	return false;
}

}
