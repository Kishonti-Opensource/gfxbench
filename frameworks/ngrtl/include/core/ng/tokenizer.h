/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <string>
#include <vector>

#include "ng/macros.h"
#include "ng/cstring.h"

namespace ng {

namespace TokenizerFlags {
	static const int AllowEmptyTokens = 1<<0;
	static const int StripLeadingSpaces = 1<<1;
	static const int StripEndSpaces = 1<<2;
	static const int HonourStrings = 1<<3;
	static const int PreserveQuotes = 1<<4;
	static const int PreserveEscapes = 1<<5;
}

void tokenize(
	OUT std::vector<std::string> &tokens,
	const ng::cstring &string,
	const ng::cstring &delimiters,
	int flags);

}


#endif