/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "configuration.h"

#include "cursor.h"

#include <algorithm>



int Configuration::columnCount()
{
    return COLUMN_COUNT;
}



const char* Configuration::columnName(int column)
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_ICON: return "icon";
    case COLUMN_TYPE: return "type";
    case COLUMN_ERROR: return "error";
    case COLUMN_ENABLED: return "isEnabled";
    case COLUMN_CHECKED: return "isChecked";
	case COLUMN_API: return "api";
    default: return "";
    }
}



Variant Configuration::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return rowId_;
    case COLUMN_TITLE: return name();
    case COLUMN_ICON: {
        std::string str = type();
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return "asset:/config_" + str;
    }
    case COLUMN_TYPE: return type();
    case COLUMN_ERROR: return errorString();
    case COLUMN_ENABLED: return isEnabled();
    case COLUMN_CHECKED: return isChecked();
	case COLUMN_API:
	{
		std::ostringstream oss;
		for (size_t i = 0; i < apiDefinitions_.size(); i++)
		{
			auto api = apiDefinitions_[i];
			if (i > 0)
				oss << "|";
			oss << api.typeToString(api.type());
		}
		return oss.str();
	}
    default: return Variant();
    }
}



Configuration::Configuration() :
    isEnabled_(true),
    isChecked_(false)
{}



std::string Configuration::name() const
{
    return name_;
}



void Configuration::setName(const std::string &name)
{
    name_ = name;
}



std::string Configuration::type() const
{
    return type_;
}



void Configuration::setType(const std::string &type)
{
    type_ = type;
}



long long Configuration::rowId() const
{
    return rowId_;
}



void Configuration::setRowId(long long rowId)
{
    rowId_ = rowId;
}



bool Configuration::isEnabled() const
{
    return isEnabled_;
}



void Configuration::setEnabled(bool enabled)
{
    isEnabled_ = enabled;
}



bool Configuration::isChecked() const
{
    return isChecked_;
}



void Configuration::setChecked(bool checked)
{
    isChecked_ = checked;
}



std::string Configuration::errorString() const
{
    return errorString_;
}



void Configuration::setErrorString(const std::string& errorString)
{
    errorString_ = errorString;
}



unsigned Configuration::apiFlags() const
{
    unsigned flags = 0;
    for (std::vector<tfw::ApiDefinition>::const_iterator i = apiDefinitions_.begin(); i != apiDefinitions_.end(); ++i) {
        const tfw::ApiDefinition& apiDefinition = *i;
        flags |= apiDefinition.type();
    }
    return flags;
}



std::vector<tfw::ApiDefinition> Configuration::ApiDefinitions() const
{
    return apiDefinitions_;
}



void Configuration::setApiDefinitions(const std::vector<tfw::ApiDefinition> &apiDefinitions)
{
    apiDefinitions_ = apiDefinitions;
}



std::vector<std::string> Configuration::features() const
{
    return features_;
}



void Configuration::setFeatures(const std::vector<std::string> &features) {
    features_ = features;
    std::sort(features_.begin(), features_.end());
}



bool Configuration::hasFeature(std::string feature) const
{
    return std::binary_search(features_.begin(), features_.end(), feature);
}
