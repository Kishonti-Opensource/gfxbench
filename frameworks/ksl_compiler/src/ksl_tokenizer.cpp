/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_tokenizer.h"

#include <assert.h>
#include <sstream>
#include <cctype>
#include <string>
#include <cstdlib>


uint64_t ksl_atoi(const char* str, bool is_hexadecimal);


KSLTokenizer::KSLTokenizer()
{
	Clear();

	// Build Symbol table
	// The symbols must be registered in descending order by size!!!
	{
		RegisterSymbol(KSL_TOKEN_LESSLESSEQUAL, "<<=");
		RegisterSymbol(KSL_TOKEN_GREATGREATEQUAL, ">>=");
		RegisterSymbol(KSL_TOKEN_LESSEQUAL, "<=");
		RegisterSymbol(KSL_TOKEN_GREATEREQUAL, ">=");
		RegisterSymbol(KSL_TOKEN_EQUALEQUAL, "==");
		RegisterSymbol(KSL_TOKEN_NOTEQUAL, "!=");
		RegisterSymbol(KSL_TOKEN_LESSLESS, "<<");
		RegisterSymbol(KSL_TOKEN_GREATGREAT, ">>");
		RegisterSymbol(KSL_TOKEN_LAZYOR, "||");
		RegisterSymbol(KSL_TOKEN_LAZYAND, "&&");
		RegisterSymbol(KSL_TOKEN_PLUS_EQUAL, "+=");
		RegisterSymbol(KSL_TOKEN_MINUS_EQUAL, "-=");
		RegisterSymbol(KSL_TOKEN_MUL_EQUAL, "*=");
		RegisterSymbol(KSL_TOKEN_DIV_EQUAL, "/=");
		RegisterSymbol(KSL_TOKEN_PLUSPLUS, "++");
		RegisterSymbol(KSL_TOKEN_MINUSMINUS, "--");
		RegisterSymbol(KSL_TOKEN_SEMICOLON, ";");
		RegisterSymbol(KSL_TOKEN_LEFT_PARENT, "(");
		RegisterSymbol(KSL_TOKEN_RIGHT_PARENT, ")");
		RegisterSymbol(KSL_TOKEN_LEFT_BRACE, "{");
		RegisterSymbol(KSL_TOKEN_RIGHT_BRACE, "}");
		RegisterSymbol(KSL_TOKEN_LEFT_BRACKET, "[");
		RegisterSymbol(KSL_TOKEN_RIGHT_BRACKET, "]");
		RegisterSymbol(KSL_TOKEN_LESS, "<");
		RegisterSymbol(KSL_TOKEN_GREATER, ">");
		RegisterSymbol(KSL_TOKEN_EQUAL, "=");
		RegisterSymbol(KSL_TOKEN_POINT, ".");
		RegisterSymbol(KSL_TOKEN_COMMA, ",");
		RegisterSymbol(KSL_TOKEN_PLUS, "+");
		RegisterSymbol(KSL_TOKEN_MINUS, "-");
		RegisterSymbol(KSL_TOKEN_MUL, "*");
		RegisterSymbol(KSL_TOKEN_DIV, "/");
		RegisterSymbol(KSL_TOKEN_MOD, "%");
		RegisterSymbol(KSL_TOKEN_BITWISE_AND, "&");
		RegisterSymbol(KSL_TOKEN_BITWISE_OR, "|");
		RegisterSymbol(KSL_TOKEN_COLON, ":");
		RegisterSymbol(KSL_TOKEN_QUESTION_MARK, "?");
		RegisterSymbol(KSL_TOKEN_EXCLAMATION_MARK, "!");
	}

	// Build reserved words table
	{
		RegisterReservedWord(KSL_TOKEN_UNIFORM, "uniform");
		RegisterReservedWord(KSL_TOKEN_IN, "in");
		RegisterReservedWord(KSL_TOKEN_OUT, "out");
		RegisterReservedWord(KSL_TOKEN_INOUT, "inout");
		RegisterReservedWord(KSL_TOKEN_CONST, "const");
		RegisterReservedWord(KSL_TOKEN_TRUE, "true");
		RegisterReservedWord(KSL_TOKEN_FALSE, "false");
		RegisterReservedWord(KSL_TOKEN_IF, "if");
		RegisterReservedWord(KSL_TOKEN_ELSE, "else");
		RegisterReservedWord(KSL_TOKEN_FOR, "for");
		RegisterReservedWord(KSL_TOKEN_CONTINUE, "continue");
		RegisterReservedWord(KSL_TOKEN_BREAK, "break");
		RegisterReservedWord(KSL_TOKEN_RETURN, "return");
		RegisterReservedWord(KSL_TOKEN_DISCARD, "discard");
		RegisterReservedWord(KSL_TOKEN_BUFFER, "buffer");
		RegisterReservedWord(KSL_TOKEN_SHARED, "shared");
		RegisterReservedWord(KSL_TOKEN_IMAGE2D, "image2D");
		RegisterReservedWord(KSL_TOKEN_READONLY, "readonly");
		RegisterReservedWord(KSL_TOKEN_WRITEONLY, "writeonly");
		RegisterReservedWord(KSL_TOKEN_NUMTHREADS, "numthreads");
		RegisterReservedWord(KSL_TOKEN_STRUCT, "struct");
		RegisterReservedWord(KSL_TOKEN_FORCE_HIGHP, "force_highp");
	}
}


KSLTokenizer::~KSLTokenizer()
{
	Clear();
}


void KSLTokenizer::Clear()
{
	m_tokens.clear();
	m_line = 1;
	m_column = 1;
}


bool KSLTokenizer::Tokenize()
{	
	bool s = true;
	m_buffer = m_source.c_str();
	m_pointer = 0;

	// scan for tokens
	while (s)
	{
		if (ScanEnd())
		{
			break;
		}

		if (ScanWhitespace()) continue;
		if (ScanSymbol()) continue;
		if (ScanIdentifier()) continue;
		// ScanFloat must be called before ScanInteger
		// (an integer literal is a valid prefix of a float literal)
		if (ScanFloat()) continue;
		if (ScanInteger()) continue;
			
		// create error log
		std::stringstream sstream;
		sstream<<"tokenization fail at "<< m_line <<":"<< m_column << std::endl;
		m_errors.push_back(KSLError(KSL_TOKENIZER, KSL_ERROR, m_line, m_column, sstream.str()));

		s = false;
		break;
	}

	// find reserved words
	for (size_t i = 0; i < m_tokens.size(); i++)
	{
		KSLToken &t = m_tokens[i];
		if (t.type != KSL_TOKEN_IDENTIFIER) continue;

		for (size_t j = 0; j < m_reserved_words.size(); j++)
		{
			if (m_reserved_words[j].second == t.str_value)
			{
				t.type = m_reserved_words[j].first;
				break;
			}
		}
	}

	// add end of tokens
	{
		KSLToken t;
		t.type = KSL_TOKEN_END_OF_TOKENS;
		t.column = m_column;
		t.line = m_line;
		m_tokens.push_back(t);
	}

	return s;
}


bool KSLTokenizer::ScanSymbol()
{
	const char* it = m_buffer + m_pointer;

	KSLToken t;
	t.column = m_column;
	t.line = m_line;

	for (size_t i = 0; i < m_symbol_table.size(); i++)
	{
		size_t l = m_symbol_table[i].second.size();

		if (memcmp(m_symbol_table[i].second.c_str(), it, l) == 0)
		{
			t.type = m_symbol_table[i].first;
			t.str_value = m_symbol_table[i].second;
			
			m_tokens.push_back(t);
			m_pointer += l;

			return true;
		}
	}

	return false;
}


bool KSLTokenizer::ScanIntStrFromPos(const size_t p, std::string &istr, bool hexadecimal)
{
	const char* it = m_buffer + p;
	const char* start_it = it;

	int (*p_is_digit)(int) = (hexadecimal) ? isxdigit : isdigit;

	if (!p_is_digit(*it))
	{
		return false;
	}

	while (p_is_digit(*it))
	{
		it++;
		m_column++;
	}

	size_t l = it - start_it;
	istr = std::string(start_it, l);

	return true;
}


bool KSLTokenizer::ScanFloat()
{
	size_t p = m_pointer;
	size_t start_p = p;

	KSLToken t;
	t.column = m_column;
	t.line = m_line;

	// scan integer part
	std::string int_part;
	bool int_part_ok = ScanIntStrFromPos(p, int_part, false);
	if (!int_part_ok)
	{
		return false;
	}
	p += int_part.size();

	// scan point
	if (m_buffer[p] != '.')
	{
		return false;
	}
	p += 1;

	// scan fraction
	std::string fract_part;
	bool fract_part_ok = ScanIntStrFromPos(p, fract_part, false);
	if (!fract_part_ok)
	{
		return false;
	}
	p += fract_part.size();

	t.str_value = int_part + "." + fract_part;
	t.float_value = atof(t.str_value.c_str());

	if (m_buffer[p] == 'h')
	{
		t.type = KSL_TOKEN_HALF;
		t.str_value += "h";
		p += 1;
	}
	else
	{
		t.type = KSL_TOKEN_FLOAT;
	}

	size_t l = p - start_p;
	m_tokens.push_back(t);
	m_pointer += l;
	m_column += (uint32_t)l;

	return true;
}


bool KSLTokenizer::ScanInteger()
{
	size_t p = m_pointer;
	size_t start_p = p;

	KSLToken t;
	t.column = m_column;
	t.line = m_line;

	bool is_hexadecimal = (m_buffer[p] == '0') && ((m_buffer[p + 1] == 'x') || (m_buffer[p + 1] == 'X' ));
	if (is_hexadecimal)
	{
		p += 2;
	}

	// scan integer part
	std::string int_part;
	bool int_part_ok = ScanIntStrFromPos(p, int_part, is_hexadecimal);
	if (!int_part_ok)
	{
		return false;
	}
	p += int_part.size();

	// scan for "u" or "U"
	if ( (m_buffer[p] == 'u') || (m_buffer[p] == 'U') )
	{
		p += 1;
		t.type = KSL_TOKEN_UNSIGNED_INTEGER;
		int_part += 'u';
	}
	else
	{
		t.type = KSL_TOKEN_SIGNED_INTEGER;
	}

	if (is_hexadecimal)
	{
		int_part = "0x" + int_part;
	}


	t.str_value = int_part;
	t.int_value = ksl_atoi(t.str_value.c_str(), is_hexadecimal);

	if ((t.int_value > KSL_INT32_MAX) && (t.type == KSL_TOKEN_SIGNED_INTEGER))
	{
		WarnTooLargeSignedIntegerLiteral(t.str_value);
	}

	size_t l = p - start_p;
	m_tokens.push_back(t);
	m_pointer += l;
	m_column += (uint32_t)l;

	return true;
}


bool KSLTokenizer::ScanIdentifier()
{
	const char* it = m_buffer + m_pointer;
	const char* start_it = it;

	KSLToken t;
	t.column = m_column;
	t.line = m_line;
	t.type = KSL_TOKEN_IDENTIFIER;

	if (!(isalpha(*it) || (*it == '_')))
	{
		return false;
	}

	while (isalnum(*it) || (*it == '_'))
	{
		it++;
	}

	size_t l = it - start_it;
	std::string v(start_it, l);
	t.str_value = v;

	m_tokens.push_back(t);
	m_pointer += l;
	m_column += (uint32_t)l;

	return true;
}


bool KSLTokenizer::ScanWhitespace()
{
	const char c = m_buffer[m_pointer];

	if (c == '\n')
	{
		m_line++;
		m_column = 1;
		m_pointer++;
		return true;
	}
	else if ((c == '\t') || (c == ' '))
	{
		m_column++;
		m_pointer++;
		return true;
	}
	else if ((c == '\r'))
	{
		m_pointer++;
		return true;
	}

	return false;
}


bool KSLTokenizer::ScanEnd()
{
	return m_pointer == m_source.size();
}


// may be replaced with C++11 function
uint64_t ksl_atoi(const char* str, bool is_hexadecimal)
{
	uint64_t res = 0;
	std::stringstream sstream;
	if (is_hexadecimal)
	{
		sstream << std::hex;
	}
	sstream << str;
	sstream >> res;
	return res;
}


void KSLTokenizer::WarnTooLargeSignedIntegerLiteral(const std::string &str)
{
	std::stringstream sstream;
	sstream << "too large signed integer literal: (" << str << ") at " << m_line << ":" << m_column
		<<". May cause compile error.";
	m_errors.push_back(KSLError(KSL_TOKENIZER, KSL_WARNING, m_line, m_column, sstream.str()));
}


