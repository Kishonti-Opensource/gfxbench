/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_JSON_INCLUDED
#define NG_JSON_INCLUDED

#include <cstddef> // size_t
#include <iterator>
#include <string>

#include "ng/ngrtl_core_export.h"
#include "ng/macros.h"
#include "ng/result.h"
#include "ng/cstring.h"

#ifdef boolean
#undef boolean
#endif

namespace ng
{

class JsonValue;

namespace JsonDetail
{
	struct Private;
	struct ConstObjectIteratorPrivate;
	struct ObjectIteratorPrivate;

	class NGRTL_EXPORT ConstObjectIterator
		: public std::iterator<
		std::bidirectional_iterator_tag,
		std::pair<const std::string, JsonValue* const>,
		ptrdiff_t,
		const std::pair<const std::string, const JsonValue* const>*,
		const std::pair<const std::string, const JsonValue* const>&>
	{
	public:
		ConstObjectIterator();
		~ConstObjectIterator();
		ConstObjectIterator(const ConstObjectIterator& y);
		void operator=(const ConstObjectIterator& y);
		bool operator!=(const ConstObjectIterator& y);
		bool operator==(const ConstObjectIterator& y);
		ConstObjectIterator& operator++();
		ConstObjectIterator& operator++(int);
		pointer operator->() const;
		reference operator*() const;
	private:
		struct ConstObjectIteratorPrivate* p;
		friend class ng::JsonValue;
	};

	class NGRTL_EXPORT ObjectIterator
		: public std::iterator<
		std::bidirectional_iterator_tag,
		std::pair<const std::string, JsonValue* const>,
		ptrdiff_t,
		std::pair<const std::string, JsonValue* const>*,
		std::pair<const std::string, JsonValue* const>&>
	{
	public:
		ObjectIterator();
		~ObjectIterator();
		ObjectIterator(const ObjectIterator& y);
		void operator=(const ObjectIterator& y);
		bool operator!=(const ObjectIterator& y);
		bool operator==(const ObjectIterator& y);
		ObjectIterator& operator++();
		ObjectIterator& operator++(int);
		pointer operator->() const;
		reference operator*() const;
	private:
		struct ObjectIteratorPrivate* p;
		friend class ng::JsonValue;
	};
}

class NGRTL_EXPORT JsonValue
{
public:
	typedef JsonDetail::ConstObjectIterator const_object_iterator;
	typedef JsonDetail::ObjectIterator object_iterator;

	JsonValue();
	JsonValue(bool b);
	JsonValue(double d);
	JsonValue(int i);
	JsonValue(unsigned u);
	JsonValue(const char* s);
	JsonValue(const std::string &s);
	JsonValue(const JsonValue& jv);
	~JsonValue();
	void swap(INOUT JsonValue& jv);

	std::string toString(bool bCompactMode = false) const;
	void toString(OUT std::string& s, bool bCompactMode = false) const;
	void toFile(const cstring& fileName, ng::Result& result) const; //compactMode = false
	void toFile(const cstring& fileName, bool bCompactMode, ng::Result& result) const;
	void fromString(const char* str, ng::Result& result);
	void fromFile(const cstring& fileName, ng::Result& result);

	bool isNull() const;
	bool isBoolean() const;
	bool isNumber() const;
	bool isString() const;
	bool isObject() const;
	bool isArray() const;

	//value getters, return content or throw exception if types mismatch
	bool boolean() const;
	double number() const;
	const char* string() const;

	//value getters with default value: return defaultValue if types mismatch or null
	bool booleanD(bool defaultValue) const; 
	double numberD(double defaultValue) const;
	const char* stringD(const char* defaultValue) const;

	//if this JsonValue is a json object and value at 's' found, return value
	//return null otherwise
	const JsonValue& operator[](const char* s) const;
	//use this on non-const JsonValues to avoid inserting null values if not found
	//(the non-const operator[const char* s] //inserts null automatically if not found)
	const JsonValue& find(const char* s) const { return ((const JsonValue*)this)->operator[](s); }

	//if this JsonValue is null, convert to json object, insert null value at 's', return value
	//if this JsonValue is object return value at 's' if found, null otherwise
	//throw exception if this JsonValue is nor null neither object
	JsonValue& operator[](const char* s);

	//if this JsonValue is an array and 'idx' found, return value
	//return null otherwise
	const JsonValue& operator[](size_t idx) const;
	const JsonValue& operator[](int idx) const {return (*this)[(size_t)idx];}

	//if this JsonValue is an array and 'idx' found, return value
	//throw exception otherwise
	JsonValue& operator[](size_t idx);
	JsonValue& operator[](int idx) {return (*this)[(size_t)idx];}

	//if this JsonValue is null or array, resize it to size. Initialize new elements with null or defaultValue
	//throw exception otherwise
	void resize(size_t size);
	void resize(size_t size, bool defaultValue);
	void resize(size_t size, double defaultValue);
	void resize(size_t size, int defaultValue) { resize(size, (double)defaultValue); }
	void resize(size_t size, unsigned defaultValue) { resize(size, (double)defaultValue); }
	void resize(size_t size, const char* defaultValue);
	void resize(size_t size, const JsonValue& defaultValue);
	
	//if this JsonValue is null or array, push back value
	//throw exception otherwise
	void push_back(bool b);
	void push_back(double d);
	void push_back(const char *s);
	void push_back(int i) { push_back((double)i); }
	void push_back(unsigned i) { push_back((double)i); }
	void push_back(const JsonValue& defaultValue);

	//if this JsonValue is a non-empty array, return corresponding element
	//exception otherwise
	JsonValue& back();
	const JsonValue& back() const;
	JsonValue& front();
	const JsonValue& front() const;
	
	void clear(); //clear content, set type to null
	void clearArray(); //clear content set type to array
	void clearObject(); //clear content, set type to object

	void operator=(bool b);
	void operator=(int i) { operator=((double)i); }
	void operator=(unsigned u) { operator=((double)u); }
	void operator=(double d);
	void operator=(const char* s);
	void operator=(const JsonValue& jv);

	size_t size() const; //return size of object or array, exception otherwise

	object_iterator beginObject();
	object_iterator endObject();
	const_object_iterator beginObject() const;
	const_object_iterator endObject() const;

private:
	JsonDetail::Private* p;

	const char* typeAsString() const;
};

}

#endif

