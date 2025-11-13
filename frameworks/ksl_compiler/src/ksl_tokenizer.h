/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_TOKENIZER__
#define __KSL_TOKENIZER__

#include "ksl_common.h"

#include <cstdint>
#include <string>
#include <vector>


enum KSLTokenType
{
	// symbols
	KSL_TOKEN_LEFT_PARENT,
	KSL_TOKEN_RIGHT_PARENT,
	KSL_TOKEN_LEFT_BRACE,
	KSL_TOKEN_RIGHT_BRACE,
	KSL_TOKEN_LEFT_BRACKET,
	KSL_TOKEN_RIGHT_BRACKET,
	KSL_TOKEN_LESS,
	KSL_TOKEN_GREATER,
	KSL_TOKEN_EQUAL,
	KSL_TOKEN_LESSEQUAL,
	KSL_TOKEN_GREATEREQUAL,
	KSL_TOKEN_EQUALEQUAL,
	KSL_TOKEN_NOTEQUAL,
	KSL_TOKEN_LESSLESS,
	KSL_TOKEN_GREATGREAT,
	KSL_TOKEN_POINT,
	KSL_TOKEN_COMMA,
	KSL_TOKEN_SEMICOLON,
	KSL_TOKEN_MINUS,
	KSL_TOKEN_PLUS,
	KSL_TOKEN_MUL,
	KSL_TOKEN_DIV,
	KSL_TOKEN_MOD,
	KSL_TOKEN_MINUS_EQUAL,
	KSL_TOKEN_PLUS_EQUAL,
	KSL_TOKEN_MUL_EQUAL,
	KSL_TOKEN_DIV_EQUAL,
	KSL_TOKEN_LAZYOR,
	KSL_TOKEN_LAZYAND,
	KSL_TOKEN_BITWISE_OR,
	KSL_TOKEN_BITWISE_AND,
	KSL_TOKEN_LESSLESSEQUAL,
	KSL_TOKEN_GREATGREATEQUAL,
	KSL_TOKEN_PLUSPLUS,
	KSL_TOKEN_MINUSMINUS,
	KSL_TOKEN_COLON,
	KSL_TOKEN_QUESTION_MARK,
	KSL_TOKEN_EXCLAMATION_MARK,
	KSL_TOKEN_FORCE_HIGHP,

	// reserved words
	KSL_TOKEN_UNIFORM,
	KSL_TOKEN_IN,
	KSL_TOKEN_OUT,
	KSL_TOKEN_INOUT,
	KSL_TOKEN_CONST,
	KSL_TOKEN_TRUE,
	KSL_TOKEN_FALSE,
	KSL_TOKEN_IF,
	KSL_TOKEN_ELSE,
	KSL_TOKEN_FOR,
	KSL_TOKEN_BREAK,
	KSL_TOKEN_CONTINUE,
	KSL_TOKEN_RETURN,
	KSL_TOKEN_STRUCT,
	KSL_TOKEN_DISCARD,
	KSL_TOKEN_BUFFER,
	KSL_TOKEN_SHARED,
	KSL_TOKEN_IMAGE2D,
	KSL_TOKEN_READONLY,
	KSL_TOKEN_WRITEONLY,
	KSL_TOKEN_NUMTHREADS,

	// general tokens
	KSL_TOKEN_IDENTIFIER,
	KSL_TOKEN_FLOAT,
	KSL_TOKEN_HALF,
	KSL_TOKEN_SIGNED_INTEGER,
	KSL_TOKEN_UNSIGNED_INTEGER,

	// other
	KSL_TOKEN_END_OF_TOKENS,
	KSL_TOKEN_INVALID,

	KSL_NUM_KSL_TOKENS
};


struct KSLToken
{
	KSLTokenType type;
	std::string str_value;
	double float_value;
	uint64_t int_value;

	uint32_t line;
	uint32_t column;

	KSLToken()
		: type(KSL_TOKEN_INVALID)
		, float_value(0.0)
		, int_value(0)
		, line(0)
		, column(0)
	{ }
};


class KSLTokenizer
{
public:
	KSLTokenizer();
	~KSLTokenizer();

	bool Tokenize();
	void Clear();

	std::string m_source;
	std::vector<KSLToken> m_tokens;

	std::vector<KSLError> m_errors;

	// debug methods
	std::string DebugDump();

private:

	bool ScanWhitespace();
	bool ScanSymbol();
	bool ScanIdentifier();
	bool ScanEnd();
	bool ScanFloat();
	bool ScanInteger();
	bool ScanIntStrFromPos(const size_t p, std::string &istr, bool hexadecimal);

	void RegisterSymbol(const KSLTokenType t, const std::string &v)
	{
		m_symbol_table.push_back(std::pair<KSLTokenType, std::string>(t, v));
	}

	void RegisterReservedWord(const KSLTokenType t, const std::string &v)
	{
		m_reserved_words.push_back(std::pair<KSLTokenType, std::string>(t, v));
	}

	const char* m_buffer;
	size_t m_pointer;

	uint32_t m_line;
	uint32_t m_column;

	std::vector<std::pair<KSLTokenType, std::string>> m_symbol_table;
	std::vector<std::pair<KSLTokenType, std::string>> m_reserved_words;

	// Errors
	void WarnTooLargeSignedIntegerLiteral(const std::string &str);
};


#endif // __KSL_TOKENIZER__

