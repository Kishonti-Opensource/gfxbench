/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//todo remove null pointer optimization

#undef NG_REQUIRE_IS_ASSERT
#define NG_REQUIRE_IS_EXCEPTION

#include "ng/json.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <assert.h>
#include "ng/require.h"
#include "ng/format.h"
#include "jsonserializator.h"
#include "jsondeserializator.h"


namespace ng
{

namespace JsonDetail
{
	typedef std::map<std::string, JsonValue*> ObjectContainer;
	typedef std::vector<JsonValue*> ArrayContainer;

	struct ConstObjectIteratorPrivate
	{
		ConstObjectIteratorPrivate() {}
		ConstObjectIteratorPrivate(const ObjectContainer::const_iterator& y)
			: it(y)
		{}
		ObjectContainer::const_iterator it;
	};

	struct ObjectIteratorPrivate
	{
		ObjectContainer::iterator it;
		ObjectIteratorPrivate() {}
		ObjectIteratorPrivate(const ObjectContainer::iterator& y)
			: it(y)
		{}
	};

	ConstObjectIterator::
		ConstObjectIterator()
		: p(new ConstObjectIteratorPrivate)
	{}

	ConstObjectIterator::
		~ConstObjectIterator()
	{
		delete p;
	}

	ConstObjectIterator::
		ConstObjectIterator(const ConstObjectIterator& y)
		: p(new ConstObjectIteratorPrivate(y.p->it))
	{
	}

	void ConstObjectIterator::
		operator=(const ConstObjectIterator& y)
	{
		p->it = y.p->it;
	}

	bool ConstObjectIterator::
		operator!=(const ConstObjectIterator& y)
	{
		return p->it != y.p->it;
	}

	bool ConstObjectIterator::
		operator==(const ConstObjectIterator& y)
	{
		return p->it == y.p->it;
	}

	ConstObjectIterator& ConstObjectIterator::
		operator++()
	{
		++p->it;
		return *this;
	}

	ConstObjectIterator& ConstObjectIterator::
		operator++(int)
	{
		p->it++;
		return *this;
	}
	
	ConstObjectIterator::pointer ConstObjectIterator::
		operator->() const
	{
		return (pointer)&*p->it;
	}

	ConstObjectIterator::reference ConstObjectIterator::
		operator*() const
	{
		return *(pointer)&*p->it;
	}


	ObjectIterator::
		ObjectIterator()
		: p(new ObjectIteratorPrivate)
	{
	}

	ObjectIterator::
		~ObjectIterator()
	{
		delete p;
	}

	ObjectIterator::
		ObjectIterator(const ObjectIterator& y)
		: p(new ObjectIteratorPrivate(y.p->it))
	{
	}

	void ObjectIterator::
		operator=(const ObjectIterator& y)
	{
		p->it = y.p->it;
	}

	bool ObjectIterator::
		operator!=(const ObjectIterator& y)
	{
		return p->it != y.p->it;
	}

	bool ObjectIterator::
		operator==(const ObjectIterator& y)
	{
		return p->it == y.p->it;
	}

	ObjectIterator& ObjectIterator::
		operator++()
	{
		++p->it;
		return *this;
	}

	ObjectIterator& ObjectIterator::
		operator++(int)
	{
		p->it++;
		return *this;
	}

	ObjectIterator::pointer ObjectIterator::
		operator->() const
	{
		return (pointer)&*p->it;
	}

	ObjectIterator::reference ObjectIterator::
		operator*() const
	{
		return *(pointer)&*p->it;
	}

	enum ValueType
	{
		eNumber,
		eString,
		eBoolean,
		eArray,
		eObject,
		eNull
	};
	union ValueUnion
	{
		double d;
		bool b;
		std::string* s;
		ArrayContainer* a;
		ObjectContainer* m;
	};
	struct Private
	{
		ValueType vt;
		ValueUnion vu;
		static const JsonValue c_nullJsonValue;

		Private()
			: vt(eNull)
		{}
		~Private()
		{
			makeNull();
		}
		void makeObjectItsNull()
		{
			assert(vt == eNull);
			vt = eObject;
			vu.m = new ObjectContainer;
		}
		void makeArrayItsNull()
		{
			assert(vt == eNull);
			vt = eArray;
			vu.a = new ArrayContainer;
		}
		void makeNull()
		{
			if ( vt == eArray )
			{
				assert(vu.a != NULL);
				deleteArrayElementsItsThat();
				delete vu.a;
				vu.a = NULL;
			}
			else if ( vt == eObject )
			{
				assert(vu.m != NULL);
				deleteObjectElementsItsThat();
				delete vu.m;
				vu.m = NULL;
			}
			else if ( vt == eString )
			{
				delete vu.s;
				vu.s = NULL;
			}
			vt = eNull;
		}
		void resize(size_t newSize, const JsonValue& cloneThis)
		{
			if ( vt == eNull )
				makeArrayItsNull();
			else
				require(vt == eArray);
			size_t oldSize = vu.a->size();
			for(size_t i = newSize; i < oldSize; ++i)
				delete (*vu.a)[i];
			vu.a->resize(newSize, NULL);
			for(size_t i = oldSize; i < newSize; ++i)
			{
				assert((*vu.a)[i] == NULL);
				(*vu.a)[i] = new JsonValue(cloneThis);
			}
		}
		void deleteArrayElementsItsThat()
		{
			assert(vt == eArray && vu.a != NULL);
			for(size_t i = 0; i < vu.a->size(); ++i)
				delete (*vu.a)[i];
		}
		void deleteObjectElementsItsThat()
		{
			assert(vt == eObject && vu.m != NULL);
			for(ObjectContainer::const_iterator it = vu.m->begin(); it != vu.m->end(); ++it)
				delete it->second;
		}
		void assignDirect(const Private* p)
		{
			if ( vt != p->vt )
			{
				makeNull();
				vt = p->vt;
				switch(vt)
				{
				case eString:
					vu.s = NULL;
					break;
				case eArray:
					vu.a = NULL;
					break;
				case eObject:
					vu.m = NULL;
					break;
				default:
					;//nothing to do
				}
			}
			switch(vt)
			{
			case eNumber:
				vu.d = p->vu.d;
				break;
			case eString:
				if ( vu.s == NULL )
					vu.s = new std::string(*p->vu.s);
				else
					(*vu.s) = *p->vu.s;
				break;
			case eBoolean:
				vu.b = p->vu.b;
				break;
			case eArray:
				if ( vu.a == NULL )
					vu.a = new ArrayContainer(p->vu.a->size());
				else
				{
					deleteArrayElementsItsThat();
					vu.a->assign(p->vu.a->size(), NULL);
				}
				for(size_t i = 0; i < vu.a->size(); ++i)
				{
					const JsonValue* jv = (*p->vu.a)[i];
					assert(jv != NULL);
					(*vu.a)[i] = new JsonValue(*jv);
				}
				break;
			case eObject:
				if ( vu.m == NULL )
					vu.m = new ObjectContainer;
				else
				{
					deleteObjectElementsItsThat();
					vu.m->clear();
				}
				for(ObjectContainer::const_iterator it = p->vu.m->begin(); it != p->vu.m->end(); ++it)
				{
					const JsonValue* jv = it->second;
					assert(jv != NULL);
					(*vu.m)[it->first] = new JsonValue(*jv);
				}
				break;
			case eNull:
				break;
			default:
				require(false);
			}
		}
	};

	const JsonValue Private::
		c_nullJsonValue;
}


using namespace JsonDetail;

JsonValue::
	JsonValue()
		: p(new Private)
{}

JsonValue::
	JsonValue(const JsonValue& jv)
	: p(new Private)
{
	p->assignDirect(jv.p);
}

JsonValue::
	JsonValue(bool b)
	: p(new Private)
{
	p->vt = eBoolean;
	p->vu.b = b;
}

JsonValue::
	JsonValue(double d)
	: p(new Private)
{
	p->vt = eNumber;
	p->vu.d = d;
}

JsonValue::
	JsonValue(int i)
	: p(new Private)
{
	p->vt = eNumber;
	p->vu.d = (double)i;
}

JsonValue::
	JsonValue(unsigned u)
	: p(new Private)
{
	p->vt = eNumber;
	p->vu.d = (double)u;
}

JsonValue::
	JsonValue(const char* s)
	: p(new Private)
{
	require(s != NULL);
	p->vt = eString;
	p->vu.s = new std::string(s);
}


JsonValue::
	JsonValue(const std::string &s)
	: p(new Private)
{
	p->vt = eString;
	p->vu.s = new std::string(s);
}


JsonValue::
	~JsonValue()
{
	if (this == &p->c_nullJsonValue)
		return;
	delete p;
}

void JsonValue::
	swap(INOUT JsonValue& jv)
{
	std::swap(p, jv.p);
}

bool JsonValue::
	isNull() const
{
	return p->vt == eNull;
}

bool JsonValue::
	isBoolean() const
{
	return p->vt == eBoolean;
}

bool JsonValue::
	isNumber() const
{
	return p->vt == eNumber;
}

bool JsonValue::
	isString() const
{
	return p->vt == eString;
}

bool JsonValue::
	isObject() const
{
	return p->vt == eObject;
}

bool JsonValue::
	isArray() const
{
	return p->vt == eArray;
}

bool JsonValue::
	boolean() const
{
	require(isBoolean());
	return p->vu.b;
}

double JsonValue::
	number() const
{
	require(isNumber());
	return p->vu.d;
}

const char* JsonValue::
	string() const
{
	require(isString());
	return p->vu.s->c_str();
}

bool JsonValue::
	booleanD(bool defaultValue) const
{
	return isBoolean() ? p->vu.b : defaultValue;
}

double JsonValue::
	numberD(double defaultValue) const
{
	return isNumber() ? p->vu.d : defaultValue;
}

const char* JsonValue::
	stringD(const char* defaultValue) const
{
	return isString() ? p->vu.s->c_str() : defaultValue;
}

const JsonValue& JsonValue::
	operator[](const char* s) const
{
	if ( isObject() )
	{
		ObjectContainer::const_iterator it = p->vu.m->find(std::string(s));
		if ( it != p->vu.m->end() )
		{
			assert(it->second != NULL);
			return *(it->second);
		}
	}
	return Private::c_nullJsonValue;
}

JsonValue& JsonValue::
	operator[](const char* s)
{
	if ( isNull() )
		p->makeObjectItsNull();
	else
		require(isObject());
	JsonValue*& m = (*p->vu.m)[std::string(s)];
	if ( m == NULL )
		m = new JsonValue;
	return *m;
}

const JsonValue& JsonValue::
	operator[](size_t idx) const
{
	if ( isArray() && idx < p->vu.a->size() )
	{
		JsonValue* jv = (*p->vu.a)[idx];
		assert(jv != NULL);
		return *jv;
	}
	return Private::c_nullJsonValue;
}

JsonValue& JsonValue::
    operator[](size_t idx)
{
    require( isArray() || isNull() );
    if ( isNull() )
        p->makeArrayItsNull();
    if (size() <= idx)
        resize(idx+1);
    JsonValue*& m = (*p->vu.a)[idx];
    assert(m != NULL);
    return *m;
}

void JsonValue::
	resize(size_t newSize)
{
	p->resize(newSize, p->c_nullJsonValue);
}

void JsonValue::
	resize(size_t newSize, bool defaultValue)
{
	p->resize(newSize, JsonValue(defaultValue));
}

void JsonValue::
	resize(size_t newSize, double defaultValue)
{
	p->resize(newSize, JsonValue(defaultValue));
}

void JsonValue::
	resize(size_t newSize, const char* defaultValue)
{
	p->resize(newSize, JsonValue(defaultValue));
}

void JsonValue::
	resize(size_t newSize, const JsonValue& defaultValue)
{
	p->resize(newSize, defaultValue);
}
	
void JsonValue::
	push_back(bool b)
{
	if ( isNull() )
		p->makeArrayItsNull();
	else
		require(isArray());
	p->vu.a->push_back(new JsonValue(b));
}

void JsonValue::
	push_back(double d)
{
	if ( isNull() )
		p->makeArrayItsNull();
	else
		require(isArray());
	p->vu.a->push_back(new JsonValue(d));
}

void JsonValue::
	push_back(const char *s)
{
	if ( isNull() )
		p->makeArrayItsNull();
	else
		require(isArray());
	p->vu.a->push_back(new JsonValue(s));
}

void JsonValue::
	push_back(const JsonValue& jv)
{
	if ( isNull() )
		p->makeArrayItsNull();
	else
		require(isArray());
	p->vu.a->push_back(new JsonValue(jv));
}

JsonValue& JsonValue::
	back()
{
	require(isArray() && !p->vu.a->empty());
	return *p->vu.a->back();
}

const JsonValue& JsonValue::
	back() const
{
	require(isArray() && !p->vu.a->empty());
	return *p->vu.a->back();
}

JsonValue& JsonValue::
	front()
{
	require(isArray() && !p->vu.a->empty());
	return *p->vu.a->front();
}

const JsonValue& JsonValue::
	front() const
{
	require(isArray() && !p->vu.a->empty());
	return *p->vu.a->front();
}

void JsonValue::
	clear()
{
	p->makeNull();
}

void JsonValue::
	clearArray()
{
	p->makeNull();
	p->makeArrayItsNull();
}

void JsonValue::
	clearObject()
{
	p->makeNull();
	p->makeObjectItsNull();
}

void JsonValue::
	operator=(bool b)
{
	if (this == &p->c_nullJsonValue)
		return;
	clear();
	p->vt = eBoolean;
	p->vu.b = b;
}

void JsonValue::
	operator=(double d)
{
	if (this == &p->c_nullJsonValue)
		return;
	clear();
	p->vt = eNumber;
	p->vu.d = d;
}

void JsonValue::
	operator=(const char* s)
{
	if (this == &p->c_nullJsonValue)
		return;
	require(s != NULL);
	clear();
	p->vt = eString;
	p->vu.s = new std::string(s);
}

void JsonValue::
	operator=(const JsonValue& jv)
{
	if (this == &p->c_nullJsonValue)
		return;
	if ( this == &jv ) return;
	//need indirect assign because jv may contain this
	JsonValue b(jv);
	this->swap(b);
}

size_t JsonValue::
	size() const
{
	if ( isArray() )
		return p->vu.a->size();
	else if ( isObject() )
		return p->vu.m->size();
	else
		require(false, ng::cstr(ng::format("size method is not valid on values of type %s") % typeAsString()));
	return 0;
}

JsonValue::object_iterator JsonValue::
	beginObject()
{
	require(isObject());
	ObjectIterator it;
	it.p->it = p->vu.m->begin();
	return it;
}

JsonValue::object_iterator JsonValue::
	endObject()
{
	require(isObject());
	ObjectIterator it;
	it.p->it = p->vu.m->end();
	return it;
}

JsonValue::const_object_iterator JsonValue::
	beginObject() const
{
	require(isObject());
	ConstObjectIterator it;
	it.p->it = p->vu.m->begin();
	return it;
}

JsonValue::const_object_iterator JsonValue::
	endObject() const
{
	require(isObject());
	ConstObjectIterator it;
	it.p->it = p->vu.m->end();
	return it;
}

const char* JsonValue::
	typeAsString() const
{
	switch(p->vt)
	{
	case eNumber:
		return "number";
	case eString:
		return "string";
	case eBoolean:
		return "boolean";
	case eArray:
		return "array";
	case eObject:
		return "object";
	case eNull:
		return "null";
	default:
		require(false, ng::cstr(ng::format("Invalid type: %s") % int(p->vt)));
		return "";
	}
}

std::string JsonValue::
	toString(bool bCompactMode) const
{
	std::string result;
	toString(result, bCompactMode);
	return result;
}

void JsonValue::
	toString(OUT std::string& s, bool bCompactMode) const
{
	JsonSerializator js;
	js.setCompactMode(bCompactMode);
	js.serialize(*this, OUT s);
}

void JsonValue::
	toFile(const cstring& fileName, ng::Result& result) const
{
	toFile(fileName, false, result);
}

void JsonValue::
	toFile(const cstring& fileName, bool bCompactMode, ng::Result& result) const
{
	JsonSerializator js;
	js.setCompactMode(bCompactMode);
	js.serializeToFile(*this, fileName.c_str(), result);
}

void JsonValue::
	fromString(const char* str, ng::Result& result)
{
	result.clear();
	JsonDeserializator jd;
	jd.deserialize(str, OUT *this, result);
}

void JsonValue::
	fromFile(const cstring& fileName, ng::Result& result)
{
	JsonDeserializator jd;
	jd.deserializeFromFile(fileName.c_str(), OUT *this, result);
}

}
