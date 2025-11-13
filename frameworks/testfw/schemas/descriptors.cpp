/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef _MSC_VER
#pragma warning(disable: 4275)
#endif

#include "descriptors.h"
#include "ng/json.h"
#include "ng/log.h"
#include "jsonutils.h"
#include <iostream>
#include <sstream>

using namespace tfw;

void Descriptor::fromJsonValue(const ng::JsonValue &jvalue)
{
		if(jvalue.isNull())
		{
			throw std::runtime_error("Object is null.");
		}

		const ng::JsonValue &jversion = jvalue["version"];
		if(!jversion.isNull())
		{
			if(jversion.isNumber())
			{
				setVersion((int)jversion.number());
			}
			else
			{
				throw std::runtime_error("Type of required field  is incorrect: 'version'");
			}

		}
		else
		{
			throw std::runtime_error("Value of required field is null: 'version'");
		}

		const ng::JsonValue &jtestId = jvalue["test_id"];
		if(!jtestId.isNull())
		{
			if(jtestId.isString())
			{
				setTestId(jtestId.string());
			}
			else
			{
				throw std::runtime_error("Type of required field  is incorrect: 'test_id'");
			}

		}
		else
		{
			throw std::runtime_error("Value of required field is null: 'test_id'");
		}

		const ng::JsonValue &jfactoryMethod = jvalue["factory_method"];
		if (!jfactoryMethod.isNull())
		{
			if (jfactoryMethod.isString())
			{
				setFactoryMethod(jfactoryMethod.string());
			}
			else
			{
				throw std::runtime_error("Type of required field  is incorrect: 'factory_method'");
			}

		}
		else
		{
			throw std::runtime_error("Value of required field is null: 'factory_method'");
		}
		const ng::JsonValue &jclass = jvalue["jclass"];
		setJclass(jclass.stringD(""));

        raw_config_ = jvalue["raw_config"];
		const ng::JsonValue &jPreloadLibs = jvalue["preload_libs"];
		if (jPreloadLibs.isArray())
		{
			std::vector<std::string> preload_libs;
			for (size_t i = 0; i < jPreloadLibs.size(); ++i)
			{
				const ng::JsonValue &jLib = jPreloadLibs[i];
				if (jLib.isString())
				{
					std::string native_lib(jLib.string() );
					preload_libs.push_back(jLib.string());
				}
				else
				{
					throw std::runtime_error("preload_libs array must contain strings");
				}
			}
			setPreloadLibs(preload_libs);
		}


		const ng::JsonValue &jdataPrefix = jvalue["data_prefix"];
		if (jdataPrefix.isString())
		{
			setDataPrefix(jdataPrefix.string());
		}

		const ng::JsonValue &jenv = jvalue["env"];
		if(!jenv.isNull())
		{
			env_.fromJsonValue(jenv);
		}

}


ng::JsonValue Descriptor::toJsonValue() const
{
	ng::JsonValue jdesc;
	jdesc["version"] = version_;
	jdesc["test_id"] = test_id_;
	jdesc["factory_method"] = factory_method_;

	jdesc["data_prefix"] = dataPrefix();
	if (!preload_libs_.empty())
	{
		size_t s = preload_libs_.size();
		ng::JsonValue jpl;
		jpl.resize(s);
		for (size_t i = 0; i < s; ++i)
		{
			jpl[i] = preload_libs_[i];
		}
		jdesc["preload_libs"] = jpl;
	}

	jdesc["env"] = env_.toJsonValue();
	jdesc["raw_config"] = raw_config_;
	jdesc["jclass"] = jclass_;
	return jdesc;
}


bool Descriptor::rawConfigHasKey(const std::string &key) const
{
	return !raw_config_.find(key.c_str()).isNull();
}


double Descriptor::rawConfign(const std::string &key) const
{
	return tfw::JsonUtils::parseNumber(raw_config_, key);
}

std::vector<double> Descriptor::rawConfignv(const std::string &key) const
{
	return tfw::JsonUtils::parseNumberVector(raw_config_, key);
}


bool Descriptor::rawConfigb(const std::string &key) const
{
	return tfw::JsonUtils::parseBool(raw_config_, key);
}

std::vector<bool> Descriptor::rawConfigbv(const std::string &key) const
{
	return tfw::JsonUtils::parseBoolVector(raw_config_, key);
}

std::string Descriptor::rawConfigs(const std::string &key) const
{
	return tfw::JsonUtils::parseString(raw_config_, key);
}

std::vector<std::string> Descriptor::rawConfigsv(const std::string &key) const
{
	return tfw::JsonUtils::parseStringVector(raw_config_, key);
}

double Descriptor::rawConfign(const std::string &key, double def) const
{
	return tfw::JsonUtils::parseNumber(raw_config_, key, def);
}


bool Descriptor::rawConfigb(const std::string &key, bool def) const
{
	return tfw::JsonUtils::parseBool(raw_config_, key, def);
}


std::string Descriptor::rawConfigs(const std::string &key, const std::string &def) const
{
	return tfw::JsonUtils::parseString(raw_config_, key, def.c_str());
}

void DescriptorList::fromJsonValue(const ng::JsonValue &jvalue)
{
	descriptors_.resize(jvalue.size());
	for (size_t i=0;i<jvalue.size();++i)
	{
		 descriptors_[i].fromJsonValue(jvalue[i]);
	}
}

ng::JsonValue DescriptorList::toJsonValue() const
{
	ng::JsonValue jdesclist;
	for (std::vector<Descriptor>::const_iterator i = descriptors_.cbegin(); i != descriptors_.cend(); ++i)
	{
		jdesclist.push_back((*i).toJsonValue());
	}
	return jdesclist;
}