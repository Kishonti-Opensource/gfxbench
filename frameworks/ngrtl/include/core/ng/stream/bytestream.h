/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_BYTESTREAM_INCLUDED
#define NG_BYTESTREAM_INCLUDED

#include <string>

#include "ng/cstring.h"
#include "ng/substring.h"

#include "stream.h"

//Input/Output byte and bit streams, allow io of simple types to streambuf

namespace ng
{

class IByteStream
	: public IStream
{
public:
	IByteStream(IStream* streamBuf)
		: _streamBuf(streamBuf)
	{}

	virtual void read(void* data, size_t size, OUT size_t* nBytesRead = 0) { _streamBuf->read(data, size, OUT nBytesRead); }

	//read var-len unsigned, see writeUX

	uint32_t readUX32(); 
	uint64_t readUX64();

	template<typename T> T readUX(); //not defined

	//convenience method to read UX directly from memory
	static uint32_t readUX32_mem(const void* source, OUT size_t* nBytesRead = 0);
	static uint64_t readUX64_mem(const void* source, OUT size_t* nBytesRead = 0);
	template<typename T> static T readUX_mem(const void* source, OUT size_t* nBytesRead = 0); //not defined

#define DEF(T, U) \
	void read##U(T& x) { _streamBuf->read((uint8_t*)&x, sizeof(T)); } \
	T read##U() { T x; read##U(OUT x); return x; } \

	DEF(uint8_t, U8)
	DEF(uint16_t, U16)
	DEF(uint32_t, U32)
	DEF(uint64_t, U64)
	DEF(int8_t, I8)
	DEF(int16_t, I16)
	DEF(int32_t, I32)
	DEF(int64_t, I64)
	DEF(float, Float)
	DEF(double, Double)
	DEF(char, Char)

#undef DEF

	std::string readStringUntilZero(); //read until zero
private:
	IStream* _streamBuf;
};

class IBitStream
	: public IStream
{
public:
	IBitStream(IStream* streamBuf)
		: _streamBuf(streamBuf)
		, _bitsInBuffer(0)
		, _buffer(0)
	{}

	virtual void read(void* data, size_t size, OUT size_t* nBytesRead = 0);
	inline int readOneBits(bool bReadNextZeroBit); //read until next '0' bit, return count of ones, throws if end-of-stream before 0
	inline int readZeroBits(bool bReadNextOneBit); //read until next '1' bit, return count of zeros, throws if end-of-stream before 1
	uint32_t readBits(size_t bits);

#define DEF(T, U) \
	void read##U(T& x) { read(&x, sizeof(T)); } \
	T read##U() { T x; read##U(OUT x); return x; } \

	DEF(uint8_t, U8)
	DEF(uint16_t, U16)
	DEF(uint32_t, U32)
	DEF(uint64_t, U64)
	DEF(int8_t, I8)
	DEF(int16_t, I16)
	DEF(int32_t, I32)
	DEF(int64_t, I64)
	DEF(float, Float)
	DEF(double, Double)
	DEF(char, Char)
#undef DEF

	void alignToByte();

private:
	IStream* _streamBuf;
	uint8_t _bitsInBuffer;
	uint8_t _buffer;

	int readOnesZerosCore(uint8_t zeroOrOne, bool bReadNextBit);
	void reloadBuffer(); //exception on end-of-stream
	bool tryReloadBuffer(); //false on end-of-stream. true if no need to reload (_bitsInBuffer > 0)

};

inline int IBitStream::
	readOneBits(bool bReadNextZeroBit)
{
	return readOnesZerosCore(1, bReadNextZeroBit);
}

inline int IBitStream::
	readZeroBits(bool bReadNextOneBit)
{
	return readOnesZerosCore(0, bReadNextOneBit);
}

class OByteStream
	: public OStream
{
public:
	OByteStream(OStream* streamBuf)
		: _streamBuf(streamBuf)
	{}

	virtual void write(const void* data, size_t size) { _streamBuf->write(data, size); }
	virtual void flush() { _streamBuf->flush(); }

#define DEF(T, U) \
	void write##U(const T& x) { _streamBuf->write(&x, sizeof(T)); }

	DEF(uint8_t, U8)
	DEF(uint16_t, U16)
	DEF(uint32_t, U32)
	DEF(uint64_t, U64)
	DEF(int8_t, I8)
	DEF(int16_t, I16)
	DEF(int32_t, I32)
	DEF(int64_t, I64)
	DEF(float, Float)
	DEF(double, Double)
	DEF(char, Char)

#undef DEF

	//write variable length number (little endian, 7 bits per byte, bit#7=1 means next byte follows)
	void writeUX32(uint32_t x); 
	void writeUX64(uint64_t x);
	void writeUX(unsigned int x)
    {
        writeUX32(x);
    }
	void writeUX(unsigned long x)
    {
        if (sizeof(unsigned long) == 4)
            writeUX32(x);
        else
            writeUX64(x);
    }
	void writeUX(unsigned long long x)
    {
        writeUX64(x);
    }

	void writeStringAndZero(const ng::substring& s);
	void writeStringWithoutZero(const ng::substring& s);
private:
	OStream* _streamBuf;
};

class OBitStream
	: public OStream
{
public:
	OBitStream(OStream* streamBuf)
		: _streamBuf(streamBuf)
		, _buffer(0)
        , _bitsInBuffer(0)		
	{}

	virtual void write(const void* data, size_t size);
	virtual void flush(); //fills last partial byte with zero bits

	void writeBits(uint32_t x, size_t bits);
	void writeOneBits(size_t pcs);
	void writeZeroBits(size_t pcs);

	void alignToByte();

#define DEF(T, U) \
	void write##U(const T& x) { write(&x, sizeof(T)); }

	DEF(uint8_t, U8)
	DEF(uint16_t, U16)
	DEF(uint32_t, U32)
	DEF(uint64_t, U64)
	DEF(int8_t, I8)
	DEF(int16_t, I16)
	DEF(int32_t, I32)
	DEF(int64_t, I64)
	DEF(float, Float)
	DEF(double, Double)
	DEF(char, Char)

#undef DEF
private:
	OStream* _streamBuf;

	uint8_t _buffer;
	uint8_t _bitsInBuffer;

	void pushFullByte();
};

}

#endif

