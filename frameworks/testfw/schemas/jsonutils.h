/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef JSONUTILS_H_
#define JSONUTILS_H_

#ifdef _MSC_VER
#pragma warning(disable: 4275)
#endif

#include "ng/json.h"
#include <string>
#include <vector>

namespace tfw {

	namespace JsonUtils
	{
		inline std::string parseString(const ng::JsonValue &data, const std::string& key)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			if (v.isString())
			{
				return v.string();
			}
			else
			{
				throw std::runtime_error("'"+ key + "' not set or invalid type");
			}
		}
		
		inline std::vector<std::string> parseStringVector(const ng::JsonValue &data, const std::string& key)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			if (v.isArray())
			{
				std::vector<std::string> ret(v.size());
				for (size_t i = 0; i < v.size(); ++i)
				{
					const ng::JsonValue &js = v[i];
					if (js.isString())
					{
						ret[i] = js.string();
					}
					else
					{
						throw std::runtime_error("array element is not type of string");
					}
				}
				return ret;
			}
			else
			{
				throw std::runtime_error("'"+ key + "' not set or not array type");
			}
		}

		inline std::vector<double> parseNumberVector(const ng::JsonValue &data, const std::string& key)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			if (v.isArray())
			{
				std::vector<double> ret(v.size());
				for (size_t i = 0; i < v.size(); ++i)
				{
					const ng::JsonValue &js = v[i];
					if (js.isNumber())
					{
						ret[i] = js.number();
					}
					else
					{
						throw std::runtime_error("array element is not number value");
					}
				}
				return ret;
			}
			else
			{
				throw std::runtime_error("'"+ key + "' not set or not array type");
			}
		}

		inline std::vector<bool> parseBoolVector(const ng::JsonValue &data, const std::string& key)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			if (v.isArray())
			{
				std::vector<bool> ret(v.size());
				for (size_t i = 0; i < v.size(); ++i)
				{
					const ng::JsonValue &js = v[i];
					if (js.isNumber())
					{
						ret[i] = js.boolean();
					}
					else
					{
						throw std::runtime_error("array element is not boolean value");
					}
				}
				return ret;
			}
			else
			{
				throw std::runtime_error("'"+ key + "' not set or not array type");
			}
		}


		inline std::string parseString(const ng::JsonValue &data, const std::string& key, const char* def)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			return v.isString() ? v.string() : def;
		}
	
		inline double parseNumber(const ng::JsonValue &data, const std::string& key)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			if (v.isNumber())
			{
				return v.number();
			}
			else
			{
				throw std::runtime_error("'"+ key + "' not set or invalid type");
			}
		}

		inline double parseNumber(const ng::JsonValue &data, const std::string& key, double def)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			return v.isNumber() ? v.number() : def;
		}

		inline bool parseBool(const ng::JsonValue &data, const std::string& key)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			if (v.isBoolean())
			{
				return v.boolean();
			}
			else
			{
				throw std::runtime_error("'"+ key + "' not set or invalid type");
			}
		}

		inline bool parseBool(const ng::JsonValue &data, const std::string& key, bool def)
		{
			const ng::JsonValue &v = data.find(key.c_str());
			return v.isBoolean() ? v.boolean() : def;
		}
	}
}

#endif  // JSONUTILS_H_
