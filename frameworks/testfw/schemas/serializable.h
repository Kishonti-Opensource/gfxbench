/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SERIALIZABLE_H
#define TFW_SERIALIZABLE_H

#ifdef _MSC_VER
#pragma warning(disable: 4275)
#endif

#include "ng/json.h"



namespace tfw {
	class Serializable
	{
	public:
        virtual ~Serializable() {}

		template<class T>
		static T fromJsonValue(const ng::JsonValue& jvalue)
		{
			T obj;
			obj.fromJsonValue(jvalue);
			return obj;
		}

		template<class T>
		static bool fromJsonString(const std::string& jstring, T* obj, std::string* error = 0)
		{
			return obj->fromJsonString(jstring, error);
		}

		bool fromJsonString(const std::string& jstring, std::string* error = 0)
		{
			try
			{
				ng::JsonValue jvalue;
				jvalue.fromString(jstring.c_str(), ng::throws());
				fromJsonValue(jvalue);
			}
			catch (const std::exception &e)
			{
				if (error)
					*error = e.what();
				return false;
			}
			return true;
		}

		bool fromJsonFile(const std::string &path, std::string *error = 0)
		{
			try
			{
				ng::JsonValue jvalue;
				jvalue.fromFile(path, ng::throws());
				fromJsonValue(jvalue);
			}
			catch (const std::exception &e)
			{
				if (error)
					*error = e.what();
				return false;
			}
			return true;
		}

		std::string toJsonString(bool compact = false) const
		{
			std::string str;
			toJsonValue().toString(str, compact);
			return str;
		}

		std::string toString() const
		{
			return toJsonString();
		}

		virtual ng::JsonValue toJsonValue() const = 0;
		virtual void fromJsonValue(const ng::JsonValue& value) = 0;

	};

}

#endif