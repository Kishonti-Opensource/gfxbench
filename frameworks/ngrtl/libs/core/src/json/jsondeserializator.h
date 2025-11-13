/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_JSONDESERIALIZATOR_INCLUDED
#define NG_JSONDESERIALIZATOR_INCLUDED

#include "ng/json.h"

namespace ng {

namespace JsonDetail { class Deserializator; }

class JsonDeserializator
{
public:
	JsonDeserializator();
	~JsonDeserializator();

	void deserialize(const char* s, OUT JsonValue& jv, OUT ng::Result& result) const;
	void deserializeFromFile(const char* filename, OUT JsonValue& jv, OUT ng::Result& result) const;
private:
	JsonDetail::Deserializator* p;
};

}

#endif


