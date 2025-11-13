/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_tokenizer.h"

#include <sstream>
#include <assert.h>


std::string TokenTypeToString(KSLTokenType type)
{
	switch (type)
	{
	// symbols
	case KSL_TOKEN_GREATEREQUAL:     return "SYGE";
	case KSL_TOKEN_LESSEQUAL:        return "SYLE";
	case KSL_TOKEN_EQUALEQUAL:       return "SYEE";
	case KSL_TOKEN_SEMICOLON:        return "SYSE";
	case KSL_TOKEN_LEFT_PARENT:      return "SYLP";
	case KSL_TOKEN_RIGHT_PARENT:     return "SYRP";
	case KSL_TOKEN_COMMA:            return "SYCO";
	case KSL_TOKEN_LEFT_BRACE:       return "SY{LB";
	case KSL_TOKEN_RIGHT_BRACE:      return "SY}RB";
	case KSL_TOKEN_LEFT_BRACKET:     return "SY[LB";
	case KSL_TOKEN_RIGHT_BRACKET:    return "SY]RB";
	case KSL_TOKEN_LESS:             return "SYL";
	case KSL_TOKEN_GREATER:          return "SYG";
	case KSL_TOKEN_EQUAL:            return "SYEQ";
	case KSL_TOKEN_POINT:            return "SYPO";
	case KSL_TOKEN_PLUS:             return "SYPLUS";
	case KSL_TOKEN_MINUS:            return "SYMIN";
	case KSL_TOKEN_MUL:              return "SYMUL";
	case KSL_TOKEN_DIV:              return "SYDIV";
	case KSL_TOKEN_MOD:              return "SYMOD";
	case KSL_TOKEN_LAZYAND:          return "SYLAND";
	case KSL_TOKEN_LAZYOR:           return "SYLOR";
	case KSL_TOKEN_PLUS_EQUAL:       return "SYPLEQ";
	case KSL_TOKEN_MINUS_EQUAL:      return "SYMIEQ";
	case KSL_TOKEN_MUL_EQUAL:        return "SYMUEQ";
	case KSL_TOKEN_DIV_EQUAL:        return "SYDIEQ";
	case KSL_TOKEN_PLUSPLUS:         return "SYPLPL";
	case KSL_TOKEN_MINUSMINUS:       return "SYMNMN";
	case KSL_TOKEN_LESSLESS:         return "SYLSLS";
	case KSL_TOKEN_GREATGREAT:       return "SYGRGR";
	case KSL_TOKEN_NOTEQUAL:         return "SYNE";
	case KSL_TOKEN_BITWISE_AND:      return "SYBAND";
	case KSL_TOKEN_BITWISE_OR:       return "SYBOR";
	case KSL_TOKEN_LESSLESSEQUAL:    return "SYLLE";
	case KSL_TOKEN_GREATGREATEQUAL:  return "SYGGE";
	case KSL_TOKEN_COLON:            return "SYCO";
	case KSL_TOKEN_QUESTION_MARK:    return "SYQM";
	case KSL_TOKEN_EXCLAMATION_MARK: return "SYEM";

	// Reserved words
	case KSL_TOKEN_UNIFORM:    return "RWUN";
	case KSL_TOKEN_IN:         return "RWIN";
	case KSL_TOKEN_OUT:        return "RWOUT";
	case KSL_TOKEN_INOUT:      return "RWINOUT";
	case KSL_TOKEN_CONST:      return "RWCONST";
	case KSL_TOKEN_TRUE:       return "RWTRUE";
	case KSL_TOKEN_FALSE:      return "RWFALSE";
	case KSL_TOKEN_IF:         return "RWIF";
	case KSL_TOKEN_ELSE:       return "RWELSE";
	case KSL_TOKEN_FOR:        return "RWFOR";
	case KSL_TOKEN_BREAK:      return "RWBREAK";
	case KSL_TOKEN_CONTINUE:   return "RWCONT";
	case KSL_TOKEN_RETURN:     return "RWRET";
	case KSL_TOKEN_DISCARD:    return "RWDISCARD";
	case KSL_TOKEN_BUFFER:     return "RWBUF";
	case KSL_TOKEN_SHARED:     return "RWSHRD";
	case KSL_TOKEN_IMAGE2D:    return "RWIMG2D";
	case KSL_TOKEN_WRITEONLY:  return "RWWO";
	case KSL_TOKEN_READONLY:   return "RWRO";
	case KSL_TOKEN_NUMTHREADS: return "RWNT";
	case KSL_TOKEN_STRUCT:     return "RWSTRUCT";
	case KSL_TOKEN_FORCE_HIGHP:return "FHP";

	// General
	case KSL_TOKEN_IDENTIFIER:       return "ID";
	case KSL_TOKEN_FLOAT:            return "FL";
	case KSL_TOKEN_HALF:             return "HL";
	case KSL_TOKEN_SIGNED_INTEGER:   return "INT";
	case KSL_TOKEN_UNSIGNED_INTEGER: return "UINT";
	case KSL_TOKEN_END_OF_TOKENS:    return "END";
	default:
		assert(0);
		return "INVALID!!!";
	}
}


std::string KSLTokenizer::DebugDump()
{
	std::stringstream sstream;

	uint32_t last_line = 1;

	for (size_t i = 0; i < m_tokens.size(); i++)
	{
		KSLToken &t = m_tokens[i];
		
		while (last_line < t.line)
		{
			sstream << std::endl;
			last_line++;
		}

		sstream << t.str_value << "[" << TokenTypeToString(t.type) << "] ";
	}

	return sstream.str();
}

