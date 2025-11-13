/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_TEXTSTREAM_INCLUDED
#define NG_TEXTSTREAM_INCLUDED

#include <stdint.h>

#include <string>
#include <deque>
#include <vector>

#include "ng/macros.h"

namespace ng
{

class IStream;
class OStream;

class ITextStream
{
public:
	enum Flags
	{
		//encoding sets mode if no autodetection or autodetection fails
		encoding_none = 0, //default, bytes transmitted as they are (to U+00XX)
		encoding_utf8 = 1,
		encoding_utf16be = 2,
		encoding_utf16le = 3,
		encoding_utf32be = 4,
		encoding_utf32le = 5,

		encoding_mask = 0x07,

		//control autodetection (first bytes of stream interpreted as BOM)
		flag_autodetectEncoding = 0, //default
		flag_dontAutodetectEncoding = 8,
		
		//control BOM handling, default to ignore autodetected bom, preserve others
		flag_ignoreAutodetectedBom = 0, //default
		flag_dontIgnoreAnyBoms = 16,
		flag_ignoreAllBoms = 32,
		
		//utf8 invalid seq handling
		utf8error_ThrowException = 0, //default
		utf8error_TransmitInvalidBytesAsUDCXX = 64,
		utf8error_DropInvalidBytes = 128,
		
		utf8error_mask = 64 + 128
	};
	ITextStream(IStream* sb, Flags flags = (Flags)0);
	bool readLineOrFalse(OUT std::string& s);
	void readFullFile(OUT std::string& s, const std::string& lineSeparator = "\n");
private:
	IStream* _streamBuf;
	Flags _flags;
	Flags _encoding;
	typedef std::deque<uint8_t> InputBuffer;
	InputBuffer _inputBuffer;
	typedef std::deque<uint32_t> OutputBuffer;
	OutputBuffer _outputBuffer;
	std::vector<uint8_t> _tempReadBuffer;
	Flags _errorHandling;

	//fills input buffer from _streamBuf then
	//tries to parse the first bytes in the input buffer as bom
	//return encoding
	//return ascii if autodetection failed
	//resets streambuf to previous position
	//return size of detected bom in bytes
	//the bytes of the bom remain in the input buffer (that's why it's called lookahead not read)
	Flags lookAheadForBom(OUT size_t& bomSize);

	bool decodeInputToOutputBuffer(); //the next character, return false on EOF
	bool ditob_utf816(); //utf8-16 part of decodeInputToOutputBuffer, private to that function
	bool ditob_utf32(bool bBE); //utf32 part of decodeInputToOutputBuffer, private to that function, bBE true if big-endian

	bool isNewline(uint32_t c) const;
	void appendUtf8(uint32_t c, INOUT std::string& s);

	void fillInputBuffer(size_t nCharsAtLeast); //reads from streamBuf until _inputBuffer.size() >= nCharsAtLeast or EOF
	void inputBufferPopFront(size_t n);
	void fillOutputBuffer(size_t nCharsAtLeast); //decodes chars from inputBuffer to outputBuffer until outputBuffer.size() >= nCharsAtLeast or EOF
	
	//Parse the beginning of the _inputBuffer as utf8 character
	//Make sure to fill the _inputBuffer to at least 4 bytes (max length of utf8 char) before calling this
	//Return true on successful conversion, result in u, n of characters consumed in bytesParsed
	//bytesParsed also set correctly on failed conversions
	bool parseUtf8(OUT size_t& bytesParsed, OUT uint32_t& u) const;
	bool parseUtf16(OUT size_t& bytesParsed, OUT uint32_t& u, bool bBE) const; //bBE if big-endian

};

class OTextStream
{
public:
	OTextStream(OStream* sb)
		: _streamBuf(sb)
	{
		static const char* defaultNewLine = "\n";
		_newLine = defaultNewLine;
		_newLineSize = 1;
	}
	void writeNewLine();
	void writeLine(const char* s);
private:
	OStream* _streamBuf;
	const char* _newLine;
	size_t _newLineSize;
};

}
#endif

