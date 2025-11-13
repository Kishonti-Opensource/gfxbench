/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_STREAMINF_INCLUDED
#define NG_STREAMINF_INCLUDED


#include <stdint.h>
#include <stddef.h>
#include "ng/macros.h"
#include "ng/ngrtl_core_export.h"


namespace ng
{

class NGRTL_EXPORT OStream
{
public:
	virtual void flush() = 0;
	virtual void write(const void *data, size_t size) = 0;
	virtual ~OStream() {}
};

class NGRTL_EXPORT IStream
{
public:
	//return number of bytes read in nBytesRead, throws exception if nBytesRead == 0 and end-of-stream occured before 'size' bytes
	//make sure to specify 0 as default value for nBytesRead when implementing IStream
	virtual void read(OUT void *data, size_t size, OUT size_t* nBytesRead = 0) = 0;
	virtual ~IStream() {}
};

class NGRTL_EXPORT SeekStreamBase
{
public:
	//use same origin constants as fseek
	void seek(ptrdiff_t offset, int origin)
	{
		sizeof(ptrdiff_t) == 4
			? seek32((int32_t)offset, origin)
			: seek64((int64_t)offset, origin);
	}
	virtual void seek32(int32_t offset, int origin) = 0; //use same origin constants as fseek
	virtual void seek64(int64_t offset, int origin) = 0; //use same origin constants as fseek
	ptrdiff_t tell()
	{
		return sizeof(size_t) == 4
			? tell32()
			: (ptrdiff_t)tell64();
	}
	virtual int32_t tell32() = 0;
	virtual int64_t tell64() = 0;

	virtual ~SeekStreamBase() {}
};

class NGRTL_EXPORT ISeekStream
	: public IStream
	, public virtual SeekStreamBase
{
};

class NGRTL_EXPORT OSeekStream
	: public OStream
	, public virtual SeekStreamBase
{
};

}

#endif

