/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/json_var.h"

#include <stdio.h>  // sprintf
#include <stdlib.h> // atof

#include <string>

#include "ng/math.h"
#include "ng/require.h"
#include "ng/format.h"
#include "ng/var.h"
#include "ng/stringutil.h"

//see RFC4627 (JSON_var standard)

namespace ng {

	namespace
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
			void serialize(const var& jv, OUT std::string& s)
			{
				_s = &s;
				_s->clear();
				if (!jv.is_undefined())
					addValue(jv, 0);
				if ( !_bCompactMode ) addEol();
			}

			bool _bCompactMode;
			size_t _currentLineBeginPos;
		private:
			EolMode _eolMode;
			std::string* _s;

			bool isArrayOfSimpleValues(const var& jv)
			{
				if ( !jv.is_array() ) return false;
				//check if all simple values
				for(size_t i = 0; i < jv.array_size(); ++i)
				{
					if ( jv[i].is_array() || jv[i].is_object() )
						return false;
				}
				return true;
			}
			void addIndent(int indent)
			{
				if ( !_bCompactMode )
					_s->append(indent, ' ');
			}
			void addValue(const var& jv, int indent)
			{
				addIndent(indent);
				if ( jv.is_null() )
				{
					_s->append("null");
				} else if ( jv.is_number() )
				{
					double d = jv.number();
					char gbuf[100];
					bool bOk = false;
					if ( d == floor(d) && fabs(d) < 1e6 ) //if small integer printf with .0f
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
				} else if ( jv.is_boolean() )
				{
					*_s += jv.boolean() ? "true" : "false";
				} else if ( jv.is_array() )
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
					//find the last non-undefined value
					size_t lnu = jv.array_size();
					for(; lnu > 0; --lnu)
						if ( !jv[lnu-1].is_undefined() )
							break;
					for(size_t i = 0; i < lnu; ++i)
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
						addValue(jv[i].is_undefined() ? var::null : jv[i], bNotCompactButFewEols ? 0 : indent+1);
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
				} else if ( jv.is_object() )
				{
					*_s += '{';
					addEol();
					var::const_object_iterator itEnd = jv.object_end();
					bool bFirst = true;
					for(var::const_object_iterator it = jv.object_begin(); it != itEnd; ++it)
					{
						if(it->second.is_undefined())
							continue;
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
							addValue(it->second, 0);
						} else
						{
							const var& v = it->second;
							if ( (v.is_array() && !isArrayOfSimpleValues(v)) || v.is_object() )
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
				} else if ( jv.is_string() )
				{
					addString(jv.string().c_str());
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
	}

	std::string JSON_var::
		stringify(const var& v, Format f)
	{
		std::string s;
		stringify(v, OUT s, f);
		return s;
	}

	void JSON_var::
		stringify(const var& v, OUT std::string& s, Format f)
	{
		Serializator ss;
		ss._bCompactMode = f == FORMAT_COMPACT;
		ss.serialize(v, OUT s);
	}


}
