/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_var_1382046266
#define INCLUDE_GUARD_var_1382046266

#include <iterator>
#include <map>

#include "ng/substring.h"

#ifdef boolean
#undef boolean
#endif

/*
	ng::var is dynamically-typed variable class, inspired by the javascript variable type and
	also the JSON format.

	Differences from JavaScript:

	- typeof null is TYPE_NULL, not TYPE_OBJECT
	- array is a separate type (TYPE_ARRAY), can't be mixed with object
	- elements of array must be continuously indexed
	- in javascript 'undefined' is used for two purposes:
		1. value of an uninitialized variable
		2. a 'void' value, that is, value of a non-existent variable, return value of a function not returning anything
	  In ng::var, the 'undefined' is used only as in (1)
*/

namespace ng {

class var
{
private:
	typedef std::vector<var> array_type;
	typedef std::map<std::string, var> object_type;

public:
	//aux class for forming paths, like "foo.bar[2].goo"
	class path
	{
	public:
		struct item
		{
			item(size_t idx)
				: _b_index(true)
				, _index(idx)
			{}
			item(substring ss)
				: _b_index(false)
				, _property_name(ss)
			{}
			substring property_name() const;
			size_t index() const;
			bool is_property_name() const;
			bool is_index() const;
		private:
			bool _b_index;
			substring _property_name;
			size_t _index;
		};
	private:
		typedef std::vector<item> container;
	public:
		typedef container::const_iterator iterator;
		typedef container::const_iterator const_iterator;

		path() {}
		explicit path(substring ss);
		iterator begin() const;
		iterator end() const;
	private:
		container _v;
	};

	typedef var this_type;

	typedef array_type::iterator array_iterator;
	typedef array_type::const_iterator const_array_iterator;

	typedef object_type::iterator object_iterator;
	typedef object_type::const_iterator const_object_iterator;

	enum type_enum
	{
		TYPE_UNDEFINED, // = uninitialized
		TYPE_NULL, //note: type of null value is null (not object, like in js)
		TYPE_BOOLEAN,
		TYPE_NUMBER,
		TYPE_STRING,
		TYPE_ARRAY, //note: array has distinct type (not object, like in js)
		TYPE_OBJECT //that is, empty object (js: {}) or has some properties
	};

	static const var undefined;
	static const var null;
	static const var empty_array;
	static const var empty_object;

	var(); //makes undefined
	var(const var& jv);
	var(bool b);
	var(double d);
	var(const char* s);
	var(const std::string& s);
	var(substring ss);
	~var();

	void swap(INOUT var& jv);

	//value getters, return content or throw exception if types mismatch
	bool boolean() const;
	bool boolean_or(bool defaultValue) const; //return defaultValue if it's not boolean
	double number() const;
	double number_or(double defaultValue) const; //return defaultValue if it's not a number
	const std::string& string() const;
	const char* c_str() const { return string().c_str(); } //convenience method

	//non-throwing versions
	const std::string& string_or(const std::string& defaultString) const;
    const char* c_str_or(cstring defaultString) const;

	bool is_undefined() const { return _type == TYPE_UNDEFINED; }
	bool is_null() const { return _type == TYPE_NULL; }
	bool is_boolean() const { return _type == TYPE_BOOLEAN; }
	bool is_number() const { return _type == TYPE_NUMBER; }
	bool is_string() const { return _type == TYPE_STRING; }
	bool is_array() const { return _type == TYPE_ARRAY; }
	bool is_object() const { return _type == TYPE_OBJECT; }
	type_enum type() const { return _type; }

	/*
		Return ref to property. This var must be an object or array (size_t versions), throws otherwise.
		For objects, inserts undefined if not exist
		Throws if non-existent prop value is queried.
	*/
	const var& operator[](const std::string& s) const { return at(s); }
	const var& operator[](size_t idx) const { return at(idx); }
	var& operator[](const std::string& s) { return insert(s); }
	var& operator[](size_t idx) { return at(idx); }

	/* same as operator[], except no auto insert for object */
	const var& at(const std::string& s) const;
	const var& at(size_t idx) const;
	var& at(const std::string& s);
	var& at(size_t idx);

	var& insert(const std::string& s); //insert new object member or just returns reference to existing. 'this' must be object

	/*
		Return const ref to object member or array element. If
		there's any errors, return var::undefined
	*/
	const var& maybe(const std::string& s) const;
	const var& maybe(size_t idx) const;

	//find returns the node, or NULL (0) if not exists
	const var* find(const std::string& ss) const;
	var* find(const std::string& ss);
	const var* find(const path& p) const;
	var* find(const path& p);

	//create_path creates the path desribed in the arg
	//all nodes along the path must either be objects or undefined or non-existent
	//expect the leaf which can be anything
	var* find_or_create(const path& p);
	var* find_or_create(const std::string& s) { return &operator[](s); }


	var& operator=(bool b);
	var& operator=(double d);
#define DECLVARINT(T) var(T x); var(unsigned T x); var& operator=(T x); var& operator=(unsigned T x);
DECLVARINT(short)
DECLVARINT(int)
DECLVARINT(long)
DECLVARINT(long long)
#undef DECLVARINT

	var& operator=(substring ss);
	var& operator=(const var& v);
	var& operator=(const char* s) { return operator=(substring(s)); }
	var& operator=(const std::string& s) { return operator=(substring(s)); }

	void be_array(); //if not array, initializes to empty array
	//array operations, throw if not an array
	size_t array_size() const; //return size of array
	void array_resize(size_t size, const var& v = undefined);
	void array_push_back(const var& v);
	bool array_empty() const; //true if array is empty

	var& array_back();
	const var& array_back() const;
	var& array_front();
	const var& array_front() const;

	array_iterator array_begin();
	const_array_iterator array_begin() const;
	array_iterator array_end();
	const_array_iterator array_end() const;

	void be_object(); //if not object, initializes to empty object
	//object operations, throw if not an object
	bool object_empty() const; //true if empty object

	object_iterator object_begin();
	const_object_iterator object_begin() const;
	object_iterator object_end();
	const_object_iterator object_end() const;

private:
	union ValueUnion
	{
		double d;
		bool b;
		std::string* s;
		array_type* a;
		object_type* m;
	};
	
	type_enum _type; //can be any of TypeOfEnum or TYPE_CONST_UNDEFINED
	ValueUnion _value;

	void dtor();
	var(type_enum t);

	friend var make_null_var();
	friend var make_empty_array_var();
	friend var make_empty_object_var();
};

}

#endif

