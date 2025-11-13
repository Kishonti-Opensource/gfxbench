/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/var.h"
#include "ng/log.h"
#include "ng/stringutil.h"
#include "ng/safecast.h"

namespace ng {
var::
	var()
	: _type(TYPE_UNDEFINED)
{
}

var::
	var(type_enum t)
	: _type(t)
{
	memset(&_value, 0, sizeof(_value));
}

var::
	var(const var& v)
	: _type(v._type)
{
	switch(_type)
	{
	case TYPE_UNDEFINED:
	case TYPE_NULL:
		break;
	case TYPE_BOOLEAN:
		_value.b = v._value.b;
		break;
	case TYPE_NUMBER:
		_value.d = v._value.d;
		break;
	case TYPE_STRING:
		_value.s = new std::string(*v._value.s);
		break;
	case TYPE_ARRAY:
		_value.a = new array_type(*v._value.a);
		break;
	case TYPE_OBJECT:
		_value.m = new object_type(*v._value.m);
		break;
	default:
		require(false, "Invalid type in var cctor");
	}
}

var::
	var(bool b)
	: _type(TYPE_BOOLEAN)
{
	_value.b = b;
}

var::
	var(double d)
	: _type(TYPE_NUMBER)
{
	_value.d = d;
}

var::
	var(substring ss)
	: _type(TYPE_STRING)
{
	_value.s = new std::string(ss.begin(), ss.end());
}

var::
	var(const char* p)
	: _type(TYPE_STRING)
{
	_value.s = new std::string(p);
}

var::
	var(const std::string& s)
	: _type(TYPE_STRING)
{
	_value.s = new std::string(s);
}

var::
	~var()
{
	dtor();
}

void var::
	dtor()
{
	switch(_type)
	{
	case TYPE_UNDEFINED:
	case TYPE_NULL:
	case TYPE_BOOLEAN:
	case TYPE_NUMBER:
		break;
	case TYPE_STRING:
		delete _value.s;
		break;
	case TYPE_ARRAY:
		delete _value.a;
		break;
	case TYPE_OBJECT:
		delete _value.m;
		break;
	default:
		NGLOG_ERROR("ng::js::var::~var() invalid type");
	}
}

void var::
	swap(INOUT var& y)
{
	if ( this == &y )
		return;

	std::swap(_type, y._type);
	ValueUnion vu;
	memcpy(&vu, &_value, sizeof(ValueUnion));
	memcpy(&_value, &y._value, sizeof(ValueUnion));
	memcpy(&y._value, &vu, sizeof(ValueUnion));
}

bool var::
	boolean() const
{
	require(is_boolean(), "js::var is not boolean");
	return _value.b;
}

bool var::
	boolean_or(bool b) const
{
	return is_boolean() ? _value.b : b;
}

double var::
	number() const
{
	require(is_number(), "js::var is not a number");
	return _value.d;
}

double var::
	number_or(double d) const
{
	return is_number() ? _value.d : d;
}

const std::string& var::
	string() const
{
	require(is_string(), "js::var is not a string");
	return *_value.s;
}

const std::string& var::
	string_or(const std::string& defaultString) const
{
	return is_string() ? *_value.s : defaultString;
}

const char* var::
c_str_or(cstring defaultString) const
{
    return is_string() ? _value.s->c_str() : defaultString.c_str();
}

const var& var::
	at(const std::string& s) const
{
	require(is_object(), "var::operator[](string): must be an object");

	const_object_iterator it = _value.m->find(s);
	require(it != _value.m->end(), "var::operator[](string): property not found");

	return it->second;
}

var& var::
	insert(const std::string& s)
{
	require(is_object(), "var::operator[](string): must be an object");

    return _value.m->operator[](s);
}

const var& var::
	maybe(const std::string& s) const
{
	do {
		if (!is_object()) break;
        const_object_iterator it = _value.m->find(s);
		if (it == _value.m->end()) break;
		return it->second;
	} while(false);
	return undefined;
}

var& var::
	at(const std::string& s)
{
	require(is_object(), "var::operator[](string): must be an object");
		
    object_iterator it = _value.m->find(s);
	require(it != _value.m->end(), "var::operator[](string): property not found");

	return it->second;
}
	
const var& var::
	at(size_t idx) const
{
	require(is_array() && 0 <= idx && idx < _value.a->size(), "var::operator[](size_t): not an array or invalid idx");
	return _value.a->operator[](idx);
}

const var& var::
	maybe(size_t idx) const
{
	return
		is_array() && idx < _value.a->size()
		? _value.a->at(idx)
		: undefined;
}

var& var::
	at(size_t idx)
{
	require(is_array() && 0 <= idx && idx < _value.a->size(), "var::operator[](size_t): not an array or invalid idx");
	return _value.a->operator[](idx);
}

void var::
	array_resize(size_t size, const var& d)
{
	require(is_array(), "var::array_resize: not an array");
	_value.a->resize(size, d);
}
	
void var::
	array_push_back(const var& v)
{
	require(is_array(), "var::array_push_back not an array");
	_value.a->push_back(v);
}

var& var::
	array_back()
{
	require(is_array() && !_value.a->empty(), "var::array_back: not a non-empty array");
	return _value.a->back();
}

const var& var::
	array_back() const
{
	require(is_array() && !_value.a->empty(), "var::array_back: not a non-empty array");
	return _value.a->back();
}

var& var::
	array_front()
{
	require(is_array() && !_value.a->empty(), "var::array_front: not a non-empty array");
	return _value.a->front();
}

const var& var::
	array_front() const
{
	require(is_array() && !_value.a->empty(), "var::array_front: not a non-empty array");
	return _value.a->front();
}
	
var& var::
	operator=(bool b)
{
	dtor();
	_type = TYPE_BOOLEAN;
	_value.b = b;
	return *this;
}
#define DEFVARINT(T) \
	var::var(T x) : _type(TYPE_NUMBER) { _value.d = SAFE_CAST<double>(x); } \
	var::var(unsigned T x) : _type(TYPE_NUMBER) { _value.d = SAFE_CAST<double>(x); } \
	var& var::operator=(T x) { return operator=(SAFE_CAST<double>(x)); } \
	var& var::operator=(unsigned T x) { return operator=(SAFE_CAST<double>(x)); }
DEFVARINT(short)
DEFVARINT(int)
DEFVARINT(long)
DEFVARINT(long long)
#undef DEFVARINT

var& var::
	operator=(double d)
{
	dtor();
	_type = TYPE_NUMBER;
	_value.d = d;
	return *this;
}

var& var::
	operator=(substring ss)
{
	dtor();
	_type = TYPE_STRING;
	_value.s = new std::string(ss.begin(), ss.end());
	return *this;
}

var& var::
	operator=(const var& v)
{
	var v2(v);
	swap(v2);
	return *this;
}

size_t var::
	array_size() const
{
	require(is_array(), "var::array_size: not an array");
	return _value.a->size();
}

var::array_iterator var::
	array_begin()
{
	require(is_array(), "var::array_begin: not an array");
	return _value.a->begin();
}

var::const_array_iterator var::
	array_begin() const
{
	require(is_array(), "var::array_begin: not an array");
	return _value.a->begin();
}

var::array_iterator var::
	array_end()
{
	require(is_array(), "var::array_end: not an array");
	return _value.a->end();
}

var::const_array_iterator var::
	array_end() const
{
	require(is_array(), "var::array_end: not an array");
	return _value.a->end();
}

bool var::
	array_empty() const
{
	require(is_array(), "var::array_empty: not an array");
	return _value.a->empty();
}

void var::
	be_array()
{
	if(is_array())
		return;
	*this = empty_array;
}

void var::
	be_object()
{
	if(is_object())
		return;
	*this = empty_object;
}

bool var::
	object_empty() const
{
	require(is_object(), "var::object_empty: not an object");
	return _value.m->empty();
}

var::object_iterator var::
	object_begin()
{
	require(is_object(), "var::object_begin: not an object");
	return _value.m->begin();
}

var::const_object_iterator var::
	object_begin() const
{
	require(is_object(), "var::object_begin: not an object");
	return _value.m->begin();
}

var::object_iterator var::
	object_end()
{
	require(is_object(), "var::object_end: not an object");
	return _value.m->end();
}

var::const_object_iterator var::
	object_end() const
{
	require(is_object(), "var::object_end: not an object");
	return _value.m->end();
}

var make_null_var()
{
	return var(var::TYPE_NULL);
}

var make_empty_array_var()
{
	var v(var::TYPE_ARRAY);
	v._value.a = new var::array_type();
	return v;
}

var make_empty_object_var()
{
	var v(var::TYPE_OBJECT);
	v._value.m = new var::object_type();
	return v;
}

const var var::
	undefined;

const var var::
	null = make_null_var();

const var var::
	empty_array = make_empty_array_var();

const var var::
	empty_object = make_empty_object_var();

const var* var::
	find(const std::string& ss) const
{
	if ( !is_object() )
		return 0;
    const_object_iterator it = _value.m->find(ss);
	return it == _value.m->end()
		? 0
		: &it->second;
}

var* var::
	find(const std::string& ss)
{
	return const_cast<var*>(const_cast<const var*>(this)->find(ss));
}

const var* var::
	find(const path& p) const
{
	const var* v = this;
	for(path::const_iterator it = p.begin(); !!v && it != p.end(); ++it)
	{
		if(v->is_object() && it->is_property_name())
		{
			v = v->find(it->property_name().to_string());
		} else if (v->is_array() && it->is_index())
		{
			if (it->index() < v->array_size())
				v = &((*v)[it->index()]);
		} else
			v = 0;
	}
	return v;
}

var* var::
	find(const path& p)
{
	return const_cast<var*>(const_cast<const var*>(this)->find(p));
}

var* var::
	find_or_create(const path& p)
{
	var* v = this;
	for(path::const_iterator it = p.begin(); !!v && it != p.end(); ++it)
	{
		if(it->is_property_name())
		{
			if(v->is_undefined())
				*v = empty_object;
			v = &((*v)[it->property_name().to_string()]);
		} else if (it->is_index())
		{
			if(v->is_undefined())
				*v = empty_array;
			if(v->array_size() <= it->index())
				v->array_resize((int)(it->index() + 1));
			v = &((*v)[it->index()]);
		} else
			require(false);
	}
	return v;
}

substring var::path::item::
	property_name() const
{
	require(is_property_name());
	return _property_name;
}

size_t var::path::item::
	index() const
{
	require(is_index());
	return _index;
}

bool var::path::item::
	is_property_name() const
{
	return !_b_index;
}

bool var::path::item::
	is_index() const
{
	return _b_index;
}

var::path::
	path(substring pathString)
{
	if ( pathString.empty() )
		return;

	std::vector<substring> pathvec = strtok(pathString, ".[]");
	int nSeparatorsToSkip = 0;
	for(size_t i = 0; i < pathvec.size(); ++i)
	{
		substring tok = pathvec[i];
		char separator;
		if ( i == 0 )
		{
			if ( tok.begin() == pathString.begin() )
				separator = '.';
			else
			{
				require(tok.begin() == pathString.begin() + 1 && *pathString.begin() == '[');
				separator = '[';
			}
		}
		else
		{
			separator = tok.begin()[-1];
			require(pathvec[i-1].end() + nSeparatorsToSkip == tok.begin());
		}
			
		if ( separator == '.' )
		{
			_v.push_back(item(tok));
			nSeparatorsToSkip = 1;
		} else
		{
			require(separator == '[' && tok.end() < pathString.end() && *tok.end() == ']');
			_v.push_back(item(strtoui(tok)));
			nSeparatorsToSkip = 2;
		}
	}
	require(pathvec.back().end() + nSeparatorsToSkip - 1 == pathString.end());
}

var::path::iterator var::path::
	begin() const
{
	return _v.begin();
}

var::path::iterator var::path::
	end() const
{
	return _v.end();
}

}
