/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/stream/textstream.h"
#include <cstring>
#include <algorithm>

#include "ng/require.h"
#include "ng/format.h"

#include "ng/stream/stream.h"

#include <sstream>

namespace ng
{
	const size_t c_tempReadBufferSize = 256;

	ITextStream::
		ITextStream(IStream* sb, Flags flags)
		: _streamBuf(sb)
		, _flags(flags)
		, _encoding((Flags)(flags & encoding_mask))
		, _tempReadBuffer(c_tempReadBufferSize)
	{
		if ( !(_flags & flag_dontAutodetectEncoding) )
		{
			size_t bomsize(0);
			Flags encoding = lookAheadForBom(OUT bomsize);
			if ( encoding != encoding_none ) //was succesful
			{
				_encoding = encoding;
				if ( !(flags & flag_dontIgnoreAnyBoms) )
					inputBufferPopFront(bomsize); //skip first bom
			}
		}
	}

	namespace Newlines
	{
		const uint32_t LF = 0x0a;
		const uint32_t VT = 0x0b;
		const uint32_t FF = 0x0c;
		const uint32_t CR = 0x0d;
		const uint32_t NEL = 0x85;
		const uint32_t LS = 0x2028;
		const uint32_t PS = 0x2029;
	}

	bool ITextStream::
		readLineOrFalse(OUT std::string& s)
	{
		s.clear();

		bool bSomethingRead = false;
		for(;;)
		{
			fillOutputBuffer(2); //2 for detecting CR LF
			if ( _outputBuffer.empty() ) break;
			bSomethingRead = true;
			uint32_t c = _outputBuffer.front();
			_outputBuffer.pop_front();
			if ( isNewline(c) )
			{
				if ( c == Newlines::CR && !_outputBuffer.empty() && _outputBuffer.front() == Newlines::LF )
					_outputBuffer.pop_front();
				break;
			} else
				appendUtf8(c, OUT s);
		}

		return bSomethingRead;
	}

	void ITextStream::
		readFullFile(OUT std::string& s, const std::string& lineSeparator)
	{
		s.clear();
		std::string s2;
		uint32_t count = 0;
		while( readLineOrFalse(s2) )
		{
			if ( count++ > 0 )
				s += lineSeparator;
			s += s2;
		}
	}

	ITextStream::Flags ITextStream::
		lookAheadForBom(OUT size_t& bomSize)
	{
		const size_t c_maxBomSize = 4;

		fillInputBuffer(c_maxBomSize);

		bomSize = 0;

		static const uint8_t bomUTF16BE[2] = { 0xfe, 0xff };
		static const uint8_t bomUTF16LE[2] = { 0xff, 0xfe };
		static const uint8_t bomUTF8[3] = { 0xef, 0xbb, 0xbf };
		static const uint8_t bomUTF32BE[4] = { 0x00, 0x00, 0xfe, 0xff };
		static const uint8_t bomUTF32LE[4] = { 0xff, 0xfe, 0x00, 0x00 };

		size_t charsRead = _inputBuffer.size();
		const uint8_t* c = charsRead > 0 ? &_inputBuffer.front() : 0;
		Flags enc(encoding_none); //set to default mode
		if ( charsRead >= 4 && memcmp(c, bomUTF32LE, 4) == 0 )
		{
			enc = encoding_utf32le;
			bomSize = 4;
		} else if ( charsRead >= 4 && memcmp(c, bomUTF32BE, 4) == 0 )
		{
			enc = encoding_utf32be;
			bomSize = 4;
		} else if ( charsRead >= 3 && memcmp(c, bomUTF8, 3) == 0 )
		{
			enc = encoding_utf8;
			bomSize = 3;
		} else if ( charsRead >= 2 && memcmp(c, bomUTF16LE, 2) == 0 )
		{
			enc = encoding_utf16le;
			bomSize = 2;
		} else if ( charsRead >= 2 && memcmp(c, bomUTF16BE, 2) == 0 )
		{
			enc = encoding_utf16be;
			bomSize = 2;
		}
		return enc;
	}

	inline bool isMsb10(uint8_t c)
	{
		return (c & 0xc0) == 0x80;
	}

	inline int nOnesInMsb(uint8_t c)
	{
		int n = 0;
		while(c & 0x80)
		{
			++n;
			c <<= 1;
		}
		return n;
	}

bool ITextStream::
	parseUtf8(OUT size_t& bytesParsed, OUT uint32_t& u) const
{
	bytesParsed = 0;
	size_t ibs = _inputBuffer.size();
	if ( ibs == 0 )
		return false;

	uint8_t d0 = _inputBuffer.front();
	int n0 = nOnesInMsb(d0);
	if ( n0 == 1 || n0 > 4 )
		return false;
	bytesParsed = 1;

	if ( n0 == 0 )
	{
		u = d0;
		return true;
	}

	int nToParse = std::min((int)ibs, n0);
	u = d0 & ((1 << (7 - n0)) - 1);
	for(int i = 1; i < (int)nToParse; ++i)
	{
		uint8_t d = _inputBuffer[i];
		if ( isMsb10(d) )
		{
			++bytesParsed;
			u = (u << 6) | (d & 0x3f);
		} else
			break;
	}
	return bytesParsed == n0;
}

bool ITextStream::
	parseUtf16(OUT size_t& bytesParsed, OUT uint32_t& u, bool bBE) const
{
	bytesParsed = 0;
	size_t ibs = _inputBuffer.size();
	if ( ibs < 2 )
		return false;

	uint16_t d0 = bBE
		? ((uint16_t)_inputBuffer[0] << 8) + (uint16_t)_inputBuffer[1]
		: ((uint16_t)_inputBuffer[1] << 8) + (uint16_t)_inputBuffer[0];

	if ( d0 < 0xd800 || 0xdbff < d0 )
	{
		bytesParsed = 2;
		u = d0;
		return true;
	}

	if ( 0xdc00 <= d0 && d0 <= 0xdfff )
		return false;

	assert(0xd800 <= d0 && d0 <= 0xdbff);
	bytesParsed = 2;

	if ( ibs < 4 )
		return false;

	uint16_t d1 = bBE
		? ((uint16_t)_inputBuffer[2] << 8) + (uint16_t)_inputBuffer[3]
		: ((uint16_t)_inputBuffer[3] << 8) + (uint16_t)_inputBuffer[2];

	if ( d1 < 0xdc00 || 0xdfff < d1)
		return false;

	assert(0xdc00 <= d1 && d1 <= 0xdfff);

	bytesParsed = 4;

	static const uint32_t offset = 0x10000 - (((uint32_t)0xd800 << 10) + (uint32_t)0xdc00);
	u = ((uint32_t)d0 << 10) + (uint32_t)d1 + offset;

	return true;
}

bool ITextStream::
	ditob_utf816()
{
	assert(_encoding == encoding_utf8 ||
		_encoding == encoding_utf16be ||
		_encoding == encoding_utf16le);

	bool bUtf8 = _encoding == encoding_utf8;
	bool bBE = _encoding == encoding_utf16be;

	const size_t c_maxUtf8CharLength = 4;
	const size_t c_maxUtf16CharLength = 4;

	size_t bytesParsed;
	uint32_t u;
	fillInputBuffer(bUtf8 ? c_maxUtf8CharLength : c_maxUtf16CharLength);
	bool b = bUtf8
		? parseUtf8(OUT bytesParsed, OUT u)
		: parseUtf16(OUT bytesParsed, OUT u, bBE);
	if ( b )
	{
		inputBufferPopFront(bytesParsed);
		_outputBuffer.push_back(u);
		return true;
	} else if ( _inputBuffer.empty() ) //eof
		return false;
	else
	{
		size_t nBytes = std::min( (size_t)(bUtf8 ? 1u : 2u), _inputBuffer.size());
		//not eof, decoding failed
		if ( _errorHandling == utf8error_TransmitInvalidBytesAsUDCXX )
		{
			for(size_t i = 0; i < nBytes; ++i)
				_outputBuffer.push_back((uint32_t)0xdc00 + _inputBuffer[i]);
			inputBufferPopFront(nBytes);
		}
		else if ( _errorHandling == utf8error_DropInvalidBytes )
		{
			inputBufferPopFront(nBytes);
		} else if ( _errorHandling == utf8error_ThrowException )
		{
			ng::format f = ng::format("Invalid %s byte sequence: ") % (bUtf8 ? "UTF8" : "UTF16" );
			for(size_t i = 0; i < std::min(_inputBuffer.size(), c_maxUtf8CharLength); ++i)
				f % _inputBuffer[i] % " ";
			require(false, ng::cstr(f));
		} else
			require(false);
		return true;
	}
}

bool ITextStream::
	ditob_utf32(bool bBE)
{
	const size_t c_utf32CharLength = 4;
	uint32_t u;
	fillInputBuffer(c_utf32CharLength);
	if ( _inputBuffer.empty() )
		return false; //eof
	if ( _inputBuffer.size() == c_utf32CharLength )
	{
		if ( bBE )
			u = ((uint32_t)_inputBuffer[0] << 24) |
				((uint32_t)_inputBuffer[1] << 16) |
				((uint32_t)_inputBuffer[2] << 8) |
				(uint32_t)_inputBuffer[3];
		else
			u = ((uint32_t)_inputBuffer[3] << 24) |
				((uint32_t)_inputBuffer[2] << 16) |
				((uint32_t)_inputBuffer[1] << 8) |
				(uint32_t)_inputBuffer[0];
		if ( u <= 0x10ffff )
		{
			inputBufferPopFront(c_utf32CharLength);
			_outputBuffer.push_back(u);
		}
	} else
	{
		size_t nBytes = std::min(_inputBuffer.size(), c_utf32CharLength);
		//not eof, decoding failed
		if ( _errorHandling == utf8error_TransmitInvalidBytesAsUDCXX )
		{
			for(size_t i = 0; i < nBytes; ++i)
				_outputBuffer.push_back((uint32_t)0xdc00 + _inputBuffer[i]);
			inputBufferPopFront(nBytes);
		} else if ( _errorHandling == utf8error_DropInvalidBytes )
		{
			inputBufferPopFront(nBytes);
		} else if ( _errorHandling == utf8error_ThrowException )
		{
			ng::format f = ng::format("Invalid UTF32 byte sequence: ");
			for(size_t i = 0; i < nBytes; ++i)
				f % _inputBuffer[i] % " ";
			require(false, ng::cstr(f));
		} else
			require(false);
	}
	return true; //non-eof
}

bool ITextStream::
	decodeInputToOutputBuffer()
{
	switch(_encoding)
	{
	case encoding_none:
		fillInputBuffer(1);
		if ( !_inputBuffer.empty() )
		{
			_outputBuffer.push_back(_inputBuffer.front());
			_inputBuffer.pop_front();
			return true;
		} else
			return false;
	case encoding_utf8:
	case encoding_utf16be:
	case encoding_utf16le:
		return ditob_utf816();
	case encoding_utf32be:
		return ditob_utf32(true);
	case encoding_utf32le:
		return ditob_utf32(false);
	default:
		require(false);
	}
	return false;
}

bool ITextStream::
	isNewline(uint32_t c) const
{
	return c == Newlines::LF
		|| c == Newlines::VT
		|| c == Newlines::FF
		|| c == Newlines::CR
		|| c == Newlines::NEL
		|| c == Newlines::LS
		|| c == Newlines::PS;
}

void ITextStream::
	appendUtf8(uint32_t c, INOUT std::string& s)
{
	if ( c <= 0x7f )
		s.push_back(c);
	else if ( c < 0x7ff )
	{
		s.push_back((c >> 6) | 0xc0);
		s.push_back((c & 0x3f) | 0x80);
	} else if ( c <= 0xffff )
	{
		s.push_back((c >> 12) | 0xe0 );
		s.push_back(((c >> 6) & 0x3f) | 0x80);
		s.push_back((c & 0x3f) | 0x80);
	} else if ( c <= 0x10ffff )
	{
		s.push_back((c >> 18) | 0xf0);
		s.push_back(((c >> 12) & 0x3f) | 0x80);
		s.push_back(((c >> 6) & 0x3f) | 0x80);
		s.push_back((c & 0x3f) | 0x80);
	} else
	{
		require(false, cstr(format("Invalid Unicode character: %s") % c));
	}
}


void ITextStream::
	fillInputBuffer(size_t nCharsAtLeast)
{
	require(nCharsAtLeast <= c_tempReadBufferSize);
	size_t sizeBefore = _inputBuffer.size();
	if ( sizeBefore <= nCharsAtLeast )
	{
		size_t nToRead = nCharsAtLeast - sizeBefore;
		size_t nRead;
		_streamBuf->read(&_tempReadBuffer.front(), nToRead, OUT &nRead);
		_inputBuffer.insert(_inputBuffer.end(), _tempReadBuffer.begin(), _tempReadBuffer.begin() + nRead);
	}
}

void ITextStream::
	inputBufferPopFront(size_t n)
{
	require(_inputBuffer.size() >= n);
	if ( n == 1 )
		_inputBuffer.pop_front();
	else
	{
		InputBuffer::iterator it = _inputBuffer.begin();
		_inputBuffer.erase(it, it + n);
	}
}

void ITextStream::
	fillOutputBuffer(size_t nCharsAtLeast)
{
	while(_outputBuffer.size() < nCharsAtLeast)
	{
		if ( !decodeInputToOutputBuffer() )
			break;
	}
}

void OTextStream::
	writeLine(const char* s)
{
	require(s != NULL);
	size_t len = strlen(s);
	_streamBuf->write((const uint8_t*)s, len);
	_streamBuf->write((const uint8_t*)_newLine, _newLineSize);
}

void OTextStream::
	writeNewLine()
{
	_streamBuf->write((const uint8_t*)_newLine, _newLineSize);
}

}

