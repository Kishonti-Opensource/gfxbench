/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "properties.h"
#include "ng/json.h"
#include "SystemInfoCommonKeys.h"
#include <sstream>
#include "systeminfo.h"
#ifndef DISABLE_COLLECTORS
#include "deviceinfocollector.h"
#include "glinfocollector.h"
#include "vulkaninfocollector.h"
#include "clinfocollector.h"
#include "metalinfocollector.h"
#include "cudainfocollector.h"
#endif
#include "keyvaluevisitor.h"

template<typename T>
void sysinf::Property::get(T &result) const
{
	std::istringstream is(value_);
	is >> result;
}

namespace
{
	const char * const tsBool = "bool";
	const char * const tsInt = "int";
	const char * const tsLong = "long";
	const char * const tsDouble = "double";
	const char * const tsString = "string";

    template<class T> struct TypeOf { static const char* type_string() { return tsLong; } };
    template<> struct TypeOf<bool> { static const char* type_string() { return tsBool; } };
    template<> struct TypeOf<int> { static const char* type_string() { return tsInt; } };
    template<> struct TypeOf<unsigned int> { static const char* type_string() { return tsInt; } };
    template<> struct TypeOf<long> { static const char* type_string() { return tsLong; } };
    template<> struct TypeOf<unsigned long> { static const char* type_string() { return tsLong; } };
    template<> struct TypeOf<long long> { static const char* type_string() { return tsLong; } };
    template<> struct TypeOf<unsigned long long> { static const char* type_string() { return tsLong; } };
    template<> struct TypeOf<double> { static const char* type_string() { return tsDouble; } };
    template<> struct TypeOf<std::string> { static const char* type_string() { return tsString; } };
    template<> struct TypeOf<const char*> { static const char* type_string() { return tsString; } };
}

template<typename T>
void sysinf::Property::set(T &value)
{
	std::ostringstream os;
	os << value;
	value_ = os.str().c_str();
	type_ = TypeOf<T>::type_string();
}

bool sysinf::Property::getBool() const
{
	bool value;
	get(value);
	return value;
}

int32_t sysinf::Property::getInt() const
{
	int32_t value;
	get(value);
	return value;
}

int64_t sysinf::Property::getLong() const
{
	int64_t value;
	get(value);
	return value;
}

double sysinf::Property::getDouble() const
{
	double value;
	get(value);
	return value;
}

const std::string &sysinf::Property::getString() const
{
	return value_;
}

const std::string &sysinf::Property::getType() const
{
	return type_;
}

const sysinf::Property *sysinf::Properties::get(const std::string &name) const
{
	map_t::const_iterator it = value_.find(name);
	
	
	if(it != value_.end())
		return &it->second;
	else
		return 0;
	
	//return it != value_.end() ? &it->second : 0;
}

bool sysinf::Properties::getBool(const std::string &name, bool defaultValue) const
{
	if (has(name)) {
		get(name)->get(defaultValue);
	}
	return defaultValue;
}

void sysinf::Properties::setBool(const std::string &name, bool value)
{
	set(name, value);
}

void sysinf::Properties::set(const std::string &name, bool value)
{
    value_[name].set(value);
}

int32_t sysinf::Properties::getInt(const std::string &name, int32_t defaultValue) const
{
	if (has(name)) {
		get(name)->get(defaultValue);
	}
	return defaultValue;
}

void sysinf::Properties::setInt(const std::string &name, int32_t value)
{
	set(name, value);
}

void sysinf::Properties::set(const std::string &name, int value)
{
    value_[name].set(value);
}

void sysinf::Properties::set(const std::string &name, unsigned value)
{
    value_[name].set(value);
}

int64_t sysinf::Properties::getLong(const std::string &name, int64_t defaultValue) const
{
	if (has(name)) {
		get(name)->get(defaultValue);
	}
	return defaultValue;
}

void sysinf::Properties::setLong(const std::string &name, int64_t value)
{
	set(name, value);
}

void sysinf::Properties::set(const std::string &name, long value)
{
    value_[name].set(value);
}

void sysinf::Properties::set(const std::string &name, unsigned long value)
{
    value_[name].set(value);
}

void sysinf::Properties::set(const std::string &name, long long value)
{
    value_[name].set(value);
}

void sysinf::Properties::set(const std::string &name, unsigned long long value)
{
    value_[name].set(value);
}

double sysinf::Properties::getDouble(const std::string &name, double defaultValue) const
{
	if (has(name)) {
		get(name)->get(defaultValue);
	}
	return defaultValue;
}

void sysinf::Properties::setDouble(const std::string &name, double value)
{
	set(name, value);
}

void sysinf::Properties::set(const std::string &name, double value)
{
    value_[name].set(value);
}

std::string sysinf::Properties::getString(const std::string &name, std::string defaultValue) const
{
	if (has(name)) {
		defaultValue = get(name)->getString();
	}
	return defaultValue;
}

void sysinf::Properties::setString(const std::string &name, const std::string &value)
{
	set(name, value);
}

void sysinf::Properties::set(const std::string &name, const std::string &value)
{
    value_[name].set(value);
}

sysinf::PropertyIter sysinf::Properties::iterator() const
{
	return PropertyIter(value_);
}

bool sysinf::Properties::has(const std::string &name) const
{
	return get(name) != 0;
}

sysinf::PropertyIter sysinf::Properties::groupIterator(const std::string &group) const
{
	return PropertyIter(value_, group);
}

sysinf::PropertyIter::const_it sysinf::PropertyIter::findPrefix(const Properties::map_t &values_, const std::string &group) const
{
	sysinf::PropertyIter::const_it i = values_.lower_bound(group);
	if (i != values_.end()) {
		const std::string& key = i->first;
		if (key.compare(0, group.size(), group) == 0) // Really a prefix?
			return i;
	}
	return values_.end();
}

sysinf::PropertyIter::PropertyIter(const Properties::map_t &values, const std::string &group)
	: values_(values)
	, iter_(findPrefix(values_, group))
	, group_(group)
{}

void sysinf::PropertyIter::first()
{
	iter_ = values_.begin();
}

void sysinf::PropertyIter::next()
{
	++iter_;
	if (!done() && iter_->first.compare(0, group_.length(), group_))
		iter_ = values_.end();
}

bool sysinf::PropertyIter::done() const
{
	return iter_ == values_.end();
}

const std::string &sysinf::PropertyIter::name() const
{
	return iter_->first;
}

const sysinf::Property &sysinf::PropertyIter::value() const
{
	return iter_->second;
}

const std::string sysinf::Properties::toJsonString(bool compact) const
{
	ng::JsonValue json;
	
	for (PropertyIter it = iterator(); !it.done(); it.next())
	{
		json[it.name().c_str()]=it.value().getString().c_str();
	};
	
	std::string str;
	json.toString(str,compact);
	return str;
}

void sysinf::Properties::updateFromJsonString(const std::string &s)
{
	ng::JsonValue json;
	ng::Result result;
	json.fromString(s.c_str(), result);
	
	
	if(!json.isNull())
	{
		for (ng::JsonValue::object_iterator it=json.beginObject(); it!=json.endObject(); it++)
		{
			if(!it->second->isNull())
			{
				if(it->second->isBoolean()) setBool(it->first, it->second->boolean());
				if(it->second->isNumber()) setDouble(it->first, it->second->number());
				if(it->second->isString()) setString(it->first, it->second->string());
			}
		}
	}
}

void sysinf::Properties::updateFromFolder(const std::string &folder)
{
	if(has("device/id"))
	{
		//iOS
		std::string device_name = this->get("device/id")->getString();
		std::stringstream ss;
		ss << folder << device_name << ".json";
		
		ng::JsonValue json;
		ng::Result result;
		json.fromFile(ss.str(), result);
		if(result.ok())
		{
			if(!json.isNull())
			{
				for (ng::JsonValue::object_iterator it=json.beginObject(); it!=json.endObject(); it++)
				{
					if(!it->second->isNull())
					{
						if(it->second->isBoolean()) setBool(it->first, it->second->boolean());
						if(it->second->isNumber()) setDouble(it->first, it->second->number());
						if(it->second->isString()) setString(it->first, it->second->string());
					}
				}
			}
		}
	}
}
#ifndef DISABLE_COLLECTORS

void sysinf::Properties::collect(DeviceInfoCollector& deviceInfoCollector, sysinf::SystemInfo& systeminfo)
{
    sysinf::KeyValueVisitor visitor(*this);
    
    deviceInfoCollector.collectAll(systeminfo);
#ifndef __APPLE__
    sysinf::collectClInfo(systeminfo);
    sysinf::collectCudaInfo(systeminfo);
#endif
    sysinf::collectVulkanInfo(systeminfo);
    sysinf::collectMetalInfo(systeminfo);
    sysinf::collectGlInfo(systeminfo);
    
    systeminfo.applyVisitor(visitor);
}
#endif

