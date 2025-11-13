/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/stream/bytestream.h"

#include <assert.h>
#include <algorithm>

#include "ng/require.h"

namespace ng
{

std::string IByteStream::
	readStringUntilZero()
{
	std::string s;
	for(;;)
	{
		char c = readChar();
		if ( c == 0 ) break;
		s += c;
	}
	return s;
}

void OByteStream::
	writeStringAndZero(const ng::substring& s)
{
 	write(s.begin(), s.size());
	writeU8(0);
}

void OByteStream::
	writeStringWithoutZero(const ng::substring& s)
{
 	write(s.begin(), s.size());
}


void OByteStream::
	writeUX32(uint32_t x)
{
	do {
		uint8_t u = x & 127;
		x >>= 7;
		if ( x != 0 )
			u |= 128;
		writeU8(u);
	} while(x != 0);
}

void OByteStream::
	writeUX64(uint64_t x)
{
	do {
		uint8_t u = x & 127;
		x >>= 7;
		if ( x != 0 )
			u |= 128;
		writeU8(u);
	} while(x != 0);
}

template<>
uint32_t IByteStream::
	readUX<uint32_t>()
{
	return readUX32();
}

template<>
uint64_t IByteStream::
	readUX<uint64_t>()
{
	return readUX64();
}

template<>
uint32_t IByteStream::
	readUX_mem<uint32_t>(const void* source, OUT size_t* nBytesRead)
{
	return readUX32_mem(source, nBytesRead);
}

template<>
uint64_t IByteStream::
	readUX_mem<uint64_t>(const void* source, OUT size_t* nBytesRead)
{
	return readUX64_mem(source, nBytesRead);
}

uint32_t IByteStream::
	readUX32()
{
	uint8_t u;
	uint32_t x(0);
	for(int c = 0; c < 5; ++c)
	{
		u = readU8();
		x |= (uint32_t)(u & 127) << (c * 7);
		if ( (u & 128) == 0 )
			return x;
	}
	require(false);
	return 0;
}

uint64_t IByteStream::
	readUX64()
{
	uint8_t u;
	uint64_t x(0);
	for(int c = 0; c < 10; ++c)
	{
		u = readU8();
		x |= (uint64_t)(u & 127) << (c * 7);
		if ( (u & 128) == 0 )
			return x;
	}
	require(false);
	return 0;
}

uint32_t IByteStream::
	readUX32_mem(const void* source, OUT size_t* nBytesRead)
{
	const uint8_t* p((const uint8_t*)source);
	uint8_t u;
	uint32_t x(0);
	for(int c = 0; c < 5; ++c)
	{
		u = *(p++);
		x |= (uint32_t)(u & 127) << (c * 7);
		if ( (u & 128) == 0 )
		{
			if ( !!nBytesRead )
				*nBytesRead = p - (const uint8_t*)source;
			return x;
		}
	}
	require(false);
	if ( !!nBytesRead )
		*nBytesRead = 0;
	return 0;
}

uint64_t IByteStream::
	readUX64_mem(const void* source, OUT size_t* nBytesRead)
{
	const uint8_t* p((const uint8_t*)source);
	uint8_t u;
	uint32_t x(0);
	for(int c = 0; c < 10; ++c)
	{
		u = *(p++);
		x |= (uint64_t)(u & 127) << (c * 7);
		if ( (u & 128) == 0 )
		{
			if ( !!nBytesRead )
				*nBytesRead = p - (const uint8_t*)source;
			return x;
		}
	}
	require(false);
	if ( !!nBytesRead )
		*nBytesRead = 0;
	return 0;
}


int IBitStream::
	readOnesZerosCore(uint8_t zeroOrOne, bool bReadNextBit)
{
	assert(zeroOrOne == 0 || zeroOrOne == 1);
	const uint8_t fullByte = (zeroOrOne == 0) ? 0 : 0xff;
	int onesRead(0);
	for(;;)
	{
		reloadBuffer();
		if ( _bitsInBuffer == 8 && _buffer == fullByte )
		{
			onesRead += 8;
			_bitsInBuffer = 0;
			continue;
		}
		while ( _bitsInBuffer > 0 && ((_buffer >> 7) == zeroOrOne) )
		{
			++onesRead;
			--_bitsInBuffer;
			_buffer <<= 1;
		}
		if ( _bitsInBuffer > 0 )
		{
			assert((_buffer >> 7) != zeroOrOne);
			if ( bReadNextBit )
			{
				--_bitsInBuffer;
				_buffer <<= 1;
			}
			break;
		}
	}
	return onesRead;
}

uint32_t IBitStream::
	readBits(size_t bits)
{
	uint32_t result = 0;

	while (bits > 0)
	{
		reloadBuffer();

		size_t bitsToRead = std::min(bits, (size_t)_bitsInBuffer);
		assert(bitsToRead <= 8);

		uint32_t t = (uint32_t)_buffer >> (8 - bitsToRead);
		result = (result << bitsToRead) | t;
		_buffer <<= bitsToRead;
		_bitsInBuffer -= (uint8_t)bitsToRead;
		bits -= bitsToRead;
	}

	return result;
}

void IBitStream::
	reloadBuffer()
{
	if (_bitsInBuffer > 0)
		return;
	_streamBuf->read(&_buffer, 1);
	_bitsInBuffer = 8;
}

bool IBitStream::
	tryReloadBuffer()
{
	if (_bitsInBuffer > 0)
		return true;
	size_t s;
	_streamBuf->read(&_buffer, 1, OUT& s);
	if ( s == 0 )
		return false;
	else
	{
		_bitsInBuffer = 8;
		return true;
	}
}

void IBitStream::
	alignToByte()
{
	if ( (_bitsInBuffer & 7) != 0 )
		_bitsInBuffer = 0;
}

void OBitStream::
	writeBits(uint32_t x, size_t xbits)
{
	if ( x == 0 && xbits == 0 )
		return;

	require(0 < xbits && xbits <= 32 && (xbits == 32 || x < (1u << xbits)));
	assert(_bitsInBuffer != 8); //all methods must pushFullByte before return
	while(xbits > 0)
	{
		int bitsToWrite = std::min(8 - (int)_bitsInBuffer, (int)xbits);
		uint8_t mask = (uint8_t)(((uint32_t)1 << bitsToWrite) - 1);
		int maskShift = 8 - bitsToWrite - _bitsInBuffer;
		assert(0 <= maskShift && maskShift < 32);
		int bitsLeftToWrite = (int)xbits - bitsToWrite;
		_buffer = (_buffer & ~(mask << maskShift)) | (((x >> bitsLeftToWrite) & mask) << maskShift);
		_bitsInBuffer += bitsToWrite;
		pushFullByte();
		xbits -= bitsToWrite;
	}
}

void OBitStream::
	alignToByte()
{
	if ( _bitsInBuffer != 0 )
	{
		writeBits(0, 8 - _bitsInBuffer);
		require(_bitsInBuffer == 0);
	}
}

void OBitStream::
	writeOneBits(size_t pcs)
{
	while(pcs > 0)
	{
		size_t q32 = std::min(pcs, (size_t)32);
		writeBits(~(uint32_t)0 >> (32-q32), q32);
		pcs -= q32;
	}
}

void OBitStream::
	writeZeroBits(size_t pcs)
{
	while(pcs > 0)
	{
		size_t q32 = std::min(pcs, (size_t)32);
		writeBits(0, q32);
		pcs -= q32;
	}
}

void OBitStream::
	pushFullByte()
{
	assert(_bitsInBuffer <= 8);
	if ( _bitsInBuffer == 8 )
	{
		_streamBuf->write(&_buffer, 1);
		_bitsInBuffer = 0;
	}
}

void IBitStream::
	read(void* vdata, size_t size, OUT size_t* nBytesRead)
{
	uint8_t* data = (uint8_t*)vdata;

	if ( _bitsInBuffer == 0)
		_streamBuf->read(data, size, nBytesRead);
	else
	{
		if ( nBytesRead == 0 )
			for(size_t i = 0; i < size; ++i)
				data[i] = readBits(8);
		else
		{
			for(*nBytesRead = 0; *nBytesRead < size; ++*nBytesRead)
			{
				uint8_t result = 0;
				int bits = 8;

				while (bits > 0)
				{
					if ( !tryReloadBuffer() )
					{
						//put back bits read
						//we needed 8 bits, must have read 8 - bits in previous iteration
						assert(_bitsInBuffer == 0); //otherwise tryReloadBuffer returned true
						_buffer = result << bits; //put lower 8-bits of result into upper end of _buffer
						_bitsInBuffer = (uint8_t)(8 - bits);
						return;
					}

					int bitsToRead = std::min(bits, (int)_bitsInBuffer);
					assert(bitsToRead <= 8);

					result = (result << bitsToRead) | (_buffer >> (8 - bitsToRead));
					_buffer <<= bitsToRead;
					_bitsInBuffer -= (uint8_t)bitsToRead;
					bits -= bitsToRead;
				}

				data[*nBytesRead] = result;
			}
		}
	}
}

void OBitStream::
	write(const void* data, size_t size)
{
	if ( _bitsInBuffer == 0 )
		_streamBuf->write(data, size);
	else
	{
		for(size_t i = 0; i < size; ++i)
			writeBits(((const uint8_t*)data)[i], 8);
	}
}

void OBitStream::
	flush()
{
	alignToByte();
}

}
