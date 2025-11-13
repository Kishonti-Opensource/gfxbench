/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_JSONSERIALIZATOR_INCLUDED
#define NG_JSONSERIALIZATOR_INCLUDED

#include "ng/json.h"

namespace ng {

namespace JsonDetail { class Serializator; }

class JsonSerializator
{
public:
	JsonSerializator();
	~JsonSerializator();

	void setCompactMode(bool b); //default: false, set true to omit newlines, whitespaces
	void serialize(const JsonValue& jv, OUT std::string& s) const;
	void serializeToFile(const JsonValue& jv, const char* filename, OUT ng::Result& result) const;
private:
	JsonDetail::Serializator* p;
};

}

#endif
