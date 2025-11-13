/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FILE_BYTESTREAM_HELPER
#define FILE_BYTESTREAM_HELPER

#include "ng/stream/bytestream.h"

//to allow bytestream read/write as FILE* without modifying legacy code
namespace ng {

inline void fwrite(const void* p, size_t size, size_t count, ng::OByteStream* f)
{
	f->write(p, size * count);
}

inline void fread(void* p, size_t size, size_t count, ng::IByteStream* f)
{
	f->read(p, size * count);
}

}

#endif


