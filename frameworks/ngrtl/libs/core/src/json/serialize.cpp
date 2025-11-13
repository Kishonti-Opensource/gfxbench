/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "jsonserializator.h"

#include <stdio.h>  // sprintf
#include <stdlib.h> // atof

#include <string>

#include "ng/math.h"
#include "ng/require.h"
#include "ng/stream/filestreambuf.h"
#include "ng/format.h"
#include "ng/stringutil.h"

//see RFC4627 (JSON standard)

namespace ng
{

namespace JsonDetail
{
	class Serializator
	{
	public:
		enum EolMode
		{
			eolLF,
			eolCRLF
		};
		Serializator()
			: _bCompactMode(false)
			, _currentLineBeginPos(0)
			, _eolMode(eolLF)
			, _s(NULL)
		{}
		void serialize(const JsonValue& jv, OUT std::string& s)
		{
			_s = &s;
			_s->clear();
			addValue(jv, 0);
			if ( !_bCompactMode ) addEol();
		}

		bool _bCompactMode;
		size_t _currentLineBeginPos;
	private:
		EolMode _eolMode;
		std::string* _s;

		bool isArrayOfSimpleValues(const JsonValue& jv)
		{
			if ( !jv.isArray() ) return false;
			//check if all simple values
			for(size_t i = 0; i < jv.size(); ++i)
			{
				if ( jv[i].isArray() || jv[i].isObject() )
					return false;
			}
			return true;
		}
		void addIndent(int indent)
		{
			if ( !_bCompactMode )
				_s->append(indent, ' ');
		}
		void addValue(const JsonValue& jv, int indent)
		{
			addIndent(indent);
			if ( jv.isNull() )
			{
				_s->append("null");
			} else if ( jv.isNumber() )
			{
				double d = jv.number();
				char gbuf[100];
				bool bOk = false;
				if ( d == floor(d) && fabs(d) < 2e9 ) //if small integer printf with .0f
				{
					sprintf(gbuf, "%.0f", d);
                    bool bAtofOk;
                    bOk = ng::atof(gbuf, &bAtofOk) == d;
                    bOk &= bAtofOk;
				}
				const int c_maxFracDigits = 18;
				for(int n = 0; !bOk && n <= c_maxFracDigits; ++n)
				{
					sprintf(gbuf, "%.*g", n, d);
                    bool bAtofOk;
                    bOk = (ng::atof(gbuf, &bAtofOk) == d);
                    bOk &= bAtofOk;
                    if (bOk)
						break;
				}
				*_s += gbuf;
			} else if ( jv.isBoolean() )
			{
				*_s += jv.boolean() ? "true" : "false";
			} else if ( jv.isArray() )
			{
				bool bNotCompactButFewEols = !_bCompactMode && isArrayOfSimpleValues(jv);

				if ( bNotCompactButFewEols )
				{
					*_s += "[ ";
				} else
				{
				*_s += '[';
				addEol();
				}
				const size_t c_maxPosInLine = 80;
				size_t firstCharPos = _s->size();
				size_t indent2 = _s->size() - _currentLineBeginPos;
				bool bFirst = true;
				for(size_t i = 0; i < jv.size(); ++i)
				{
					if ( !bFirst )
					{
						if ( bNotCompactButFewEols )
						{
							if ( _s->size() - firstCharPos > c_maxPosInLine )
							{
						*_s += ',';
						addEol();
								addIndent((int)indent2);
								firstCharPos = _s->size();
					} else
								*_s += ", ";
						} else
						{
							*_s += ',';
							addEol();
						}
					} else
						bFirst = false;
					addValue(jv[i], bNotCompactButFewEols ? 0 : indent+1);
				}
				if ( !bNotCompactButFewEols )
				{
				addEol();
				addIndent(indent);
				*_s += ']';
				} else
				{
					*_s += " ]";
				}
			} else if ( jv.isObject() )
			{
				*_s += '{';
				addEol();
				JsonValue::const_object_iterator itEnd = jv.endObject();
				bool bFirst = true;
				for(JsonValue::const_object_iterator it = jv.beginObject(); it != itEnd; ++it)
				{
					if ( !bFirst )
					{
						*_s += ',';
						addEol();
					} else
						bFirst = false;
					addIndent(indent + 1);
					addString(it->first.c_str());
					*_s += ':';
					if ( _bCompactMode )
					{
						addValue(*it->second, 0);
					} else
					{
						const JsonValue& v = *it->second;
						if ( (v.isArray() && !isArrayOfSimpleValues(v)) || v.isObject() )
						{
							addEol();
							addValue(v, indent + 1);
						} else
						{
							*_s += ' ';
							addValue(v, 0);
						}
					}
				}
				addEol();
				addIndent(indent);
				*_s += '}';
			} else if ( jv.isString() )
			{
				addString(jv.string());
			} else
				require(false);
		}
		void addEol()
		{
			if ( !_bCompactMode )
			{
				switch(_eolMode)
				{
				case eolLF:
					*_s += (char)10;
					break;
				case eolCRLF:
					*_s += (char)13;
					*_s += (char)10;
					break;
				}
				_currentLineBeginPos = _s->size();
			}
		}
		void addString(const char* s)
		{
			char buf[10];
			*_s += '"';
			const char* p0 = s;
			const char* p1 = s;
			while(*p1 != 0)
			{
				const char* escape = NULL;
				if ( *p1 == '"' )
					escape = "\\\"";
				else if ( *p1 == '\\' )
					escape = "\\\\";
				else if ( 0 <= *p1 && *p1 <= 0x1f )
				{
					sprintf(buf, "\\u%04x", (int)*p1);
					escape = buf;
				}
				if ( escape != NULL )
				{
					_s->append(p0, p1);
					*_s += escape;
					p0 = ++p1;
				} else
					++p1;
			}
			_s->append(p0, p1);
			*_s += '"';
		}
	};

	class Deserializator
	{
	public:
		void deserialize(const char* s, OUT JsonValue& jv)
		{
		}
	};
}

using namespace JsonDetail;

JsonSerializator::
	JsonSerializator()
	: p(new Serializator)
{
}

JsonSerializator::
	~JsonSerializator()
{
	delete p;
}

void JsonSerializator::
	serialize(const JsonValue& jv, OUT std::string& s) const
{
	p->serialize(jv, s);
}

void JsonSerializator::
	serializeToFile(const JsonValue& jv, const char* filename, OUT ng::Result& result) const
{
	result.clear();
	try {
		std::string serialized;
		p->serialize(jv, serialized);

		FileStreamBuf f(filename, "wb", result);
		if ( result.error() ) return;
		f.write(serialized.c_str(), serialized.size());
		f.close(result); //we're interested in errors during close. Omitting this would call destructor which ignores errors
	} catch(std::exception& e)
	{
		if ( result.throws() )
			throw;
		else
		{
			NG_SET_RESULT_MSG(result, FORMATCSTR("JsonSerializator exception: %s", e.what()));
			return;
		}
	} catch(...)
	{
		if ( result.throws() )
			throw;
		else
		{
			NG_SET_RESULT_MSG(result, "JsonSerializator unknown exception");
			return;
		}
	}
}

void JsonSerializator::
	setCompactMode(bool b)
{
	p->_bCompactMode = b;
}

}
