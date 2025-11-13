/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "configuration.h"

using namespace tfw;



Configuration::Configuration():
    isEnabled_(true)
{}



void Configuration::fromJsonValue(const ng::JsonValue &jsonValue)
{
    name_ = jsonValue["name_"].string();
    type_ = jsonValue["type_"].string();
    ng::JsonValue jApiDefinitions = jsonValue["apiDefinitions_"];
    for (size_t i = 0; i < jApiDefinitions.size(); ++i) {
        apiDefinitions_.push_back(Serializable::fromJsonValue<ApiDefinition>(jApiDefinitions[i]));
    }
    ng::JsonValue jFeatures = jsonValue["features_"];
    for (size_t i = 0; i < jFeatures.size(); ++i) {
        features_.push_back(jFeatures[i].string());
    }
}



ng::JsonValue Configuration::toJsonValue() const
{
    ng::JsonValue jConfiguration;
    jConfiguration["name_"] = name_;
    jConfiguration["type_"] = type_;
    for (std::vector<ApiDefinition>::const_iterator i = apiDefinitions_.begin(); i != apiDefinitions_.end(); ++i) {
        const ApiDefinition& apiDefinition = *i;
        jConfiguration["apiDefinitions_"].push_back(apiDefinition.toJsonValue());
    }
    for (std::vector<std::string>::const_iterator i = features_.begin(); i != features_.end(); ++i) {
        const std::string& feature = *i;
        jConfiguration["features_"].push_back(feature);
    }
    return jConfiguration;
}



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



bool Configuration::isEnabled() const
{
    return isEnabled_;
}



void Configuration::setEnabled(bool enabled)
{
    isEnabled_ = enabled;
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
    for (std::vector<ApiDefinition>::const_iterator i = apiDefinitions_.begin(); i != apiDefinitions_.end(); ++i) {
        const ApiDefinition& apiDefinition = *i;
        flags |= apiDefinition.type();
    }
    return flags;
}



std::vector<ApiDefinition> Configuration::ApiDefinitions() const
{
    return apiDefinitions_;
}



void Configuration::setApiDefinitions(const std::vector<ApiDefinition> &apiDefinitions)
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
