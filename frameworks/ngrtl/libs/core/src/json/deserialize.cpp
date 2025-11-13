/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "jsondeserializator.h"

#include <cstdio>
#include <vector>
#include <cassert>
#include <stdexcept>

#include "JSON_parser.h"
#include "ng/json.h"
#include "ng/require.h"

#include "ng/stream/filestreambuf.h"
#include "ng/stream/textstream.h"
#include "ng/scoped_ptr.h"
#include "ng/log.h"

namespace ng
{

namespace JsonDetail
{

int JsonParserCallback(void* ctx, int type, const JSON_value* value);

class Deserializator
{
public:
	void deserialize(const char* str, OUT JsonValue& jv, OUT ng::Result& result)
	{
		result.clear();

		JSON_config config;
		init_NGJSON_config(&config);

		config.depth = -1; //unlimited
		config.callback = JsonDetail::JsonParserCallback;
		config.callback_ctx = this;
		config.allow_comments = 1;
		config.handle_floats_manually = 0;

		JSON_parser_struct *jc(NULL);
		
		bool error(false);
		int nrow(0), nbyte(0);
		_root.clear();
		_stack.clear();
		_bKeySet = false;

		try
		{
			jc = new_NGJSON_parser(&config);

			const char* nextChar = str;
			nrow = 1;
			nbyte = 1;
			while (*nextChar != 0)
			{
				if (!NGJSON_parser_char(jc, (unsigned char)*nextChar)) //unsigned char is important, json parser needs positive values
				{
					error = true;
					break;
				}
				++nextChar;
				++nbyte;
				if (*nextChar == 10)
				{
					++nrow;
					nbyte = 1;
				} else if (nextChar[0] == 13 && nextChar[1] == 10)
				{
					++nextChar;
					++nrow;
					nbyte = 1;
				}
			}
			if (!error && !NGJSON_parser_done(jc))
			{
				nrow = -1;
				error = true;
			}
		} catch(std::exception& e)
		{
			if ( jc != NULL )
				delete_NGJSON_parser(jc);
			if ( result.throws() )
				throw;
			else
			{
				NG_SET_RESULT_MSG(result, FORMATCSTR("Exception caught during JSON parsing: %s", e.what()));
				return;
			}
		} catch(...)
		{
			if ( jc != NULL )
				delete_NGJSON_parser(jc);
			if ( result.throws() )
				throw;
			else
			{
				NG_SET_RESULT_MSG(result, "Unknown exception caught during JSON parsing");
				return;
			}
		}

		if ( jc != NULL )
			delete_NGJSON_parser(jc);

		if ( error )
		{
			if ( nrow < 0 )
				NG_SET_RESULT_MSG(result, "JSON parser error at end of string");
			else
				NG_SET_RESULT_MSG(result, FORMATCSTR("JSON parser error at line %s byte %s", nrow, nbyte));
			return;
		}

		jv.swap(_root);
	}

	int jsonParserCallback(int type, const JSON_value* value)
	{
		bool bBegin = false;
		if ( type == JSON_T_ARRAY_END )
		{
			assert(!_stack.empty());
			_stack.pop_back();
		} else if ( type == JSON_T_OBJECT_END )
		{
			assert(!_stack.empty());
			_stack.pop_back();
		} else if ( type == JSON_T_KEY )
		{
			_bKeySet = true;
			_key = value->vu.str.value;
		} else
		{
			JsonValue jv;
			switch(type)
			{
			case JSON_T_NULL: //jv is already null, nothing to do
				break;
			case JSON_T_INTEGER:
				//after a mod on 20120619 integers are parsed as floats
				//but still reported as integers
				//so next line (original) is commented out and replaced by the one after it
				//jv = (double)value->vu.integer_value;
				jv = value->vu.float_value;
				break;
			case JSON_T_FLOAT:
				jv = value->vu.float_value;
				break;
			case JSON_T_TRUE:
				jv = true;
				break;
			case JSON_T_FALSE:
				jv = false;
				break;
			case JSON_T_STRING:
				jv = value->vu.str.value;
				break;
			case JSON_T_ARRAY_BEGIN:
				jv.clearArray();
				bBegin = true;
				break;
			case JSON_T_OBJECT_BEGIN:
				jv.clearObject();
				bBegin = true;
				break;
			default:
				require(false);
			}
			if ( _stack.empty() )
			{
				_root.swap(jv);
				if ( bBegin )
					_stack.push_back(&_root);
			} else if (_stack.back()->isArray() )
			{
				_stack.back()->push_back(jv);
				if ( bBegin )
					_stack.push_back(&(_stack.back()->back()));
			} else if (_stack.back()->isObject() && _bKeySet )
			{
				_bKeySet = false;
				JsonValue& jv2 = (*_stack.back())[_key.c_str()];
				jv2.swap(jv);
				if ( bBegin )
					_stack.push_back(&jv2);
			} else
				require(false);
		}
		return 1;
	}
	private:
		JsonValue _root;
		std::vector<JsonValue*> _stack;
		bool _bKeySet;
		std::string _key;
};

int JsonParserCallback(void* ctx, int type, const JSON_value* value)
{
	return ((Deserializator*)ctx)->jsonParserCallback(type, value);
}	

}

using namespace JsonDetail;

JsonDeserializator::
	JsonDeserializator()
	: p(new Deserializator)
{
}

JsonDeserializator::
	~JsonDeserializator()
{
	delete p;
}

void JsonDeserializator::
	deserialize(const char* s, OUT JsonValue& jv, OUT ng::Result& result) const
{
	result.clear();
	try {
		p->deserialize(s, jv, result);
	} catch(std::exception& e)
	{
		if ( result.throws() )
			throw;
		else
			NG_SET_RESULT_MSG(result, FORMATCSTR("JsonDeserializator exception: %s", e.what()));
	} catch(...)
	{
		if ( result.throws() )
			throw;
		else
			NG_SET_RESULT_MSG(result, "JsonDeserializator unknown exception: %s");
	}
}

void JsonDeserializator::
	deserializeFromFile(const char* filename, OUT JsonValue& jv, OUT ng::Result& result) const
{
	result.clear();
	std::string content;
	try
	{
		ng::FileStreamBuf fsb(filename, "rb", result);
		if ( result.error() ) return;
		ng::ITextStream its(&fsb, ng::ITextStream::encoding_utf8);
		its.readFullFile( content, " " );
		p->deserialize( content.c_str(), jv, result);
	} catch (const std::exception& e)
	{
		if ( result.throws() )
			throw;
		else
		{
			NG_SET_RESULT_MSG(result, FORMATCSTR("JsonDeserializator::deserializeFromFile exception (filename: %s): %s", filename, e.what()));
			return;
		}
	} catch(...)
	{
		if ( result.throws() )
			throw;
		else
		{
			NG_SET_RESULT_MSG(result, FORMATCSTR("JsonDeserializator::deserializeFromFile unknown exception (filename: %s)", filename));
			return;
		}
	}
}

}
