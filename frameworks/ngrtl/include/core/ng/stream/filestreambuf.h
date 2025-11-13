/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_FILESTREAMBUF_INCLUDED
#define NG_FILESTREAMBUF_INCLUDED

#include <stdio.h>

#include "stream.h"
#include "ng/result.h"

namespace ng
{


//streambuf interface for stdio FILE* streams
//FILE* pointer can be opened by user before using it in FileStreamBuf
//or can opened and owned internally by FileStreamBuf
class FileStreamBuf
	: public ISeekStream
	, public OSeekStream
{
	FileStreamBuf(const FileStreamBuf&);
	void operator=(const FileStreamBuf&);
public:
	FileStreamBuf();
	FileStreamBuf(FILE* stream); //set stream, not owned
	FileStreamBuf(const char* filename, const char* mode, OUT ng::Result&); //open stream, owned
	~FileStreamBuf();
	void open(const char* filename, const char* mode, OUT ng::Result&); //open stream, owned
	void close(OUT ng::Result& res);

	virtual void read(OUT void *data, size_t size, OUT size_t* nBytesRead = 0);
	virtual void write(const void *data, size_t size);

	virtual void seek32(int32_t offset, int origin);
	virtual void seek64(int64_t offset, int origin);
	virtual int32_t tell32();
	virtual int64_t tell64();

	virtual void flush();

	bool isAtEof(); //return true at the end of file, (even if not read past the end, this function does read)

	//depracated, if you need this functionality write a named method because it's not clear what this operator is intended to report
	//bool operator!() const { return !_stream; }

private:
	FILE* _stream;
	bool _bOwner;
};

}


#endif


