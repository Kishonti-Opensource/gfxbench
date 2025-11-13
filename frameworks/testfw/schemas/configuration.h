/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "apidefinition.h"
#include "serializable.h"

#include <algorithm>
#include <string>
#include <vector>



namespace tfw
{

class Configuration: public Serializable
{
public:
    Configuration();

    void fromJsonValue(const ng::JsonValue &jsonValue);
    ng::JsonValue toJsonValue() const;
    
    std::string name() const;
    void setName(const std::string &name);

    std::string type() const;
    void setType(const std::string &type);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    std::string errorString() const;
    void setErrorString(const std::string& errorString);

    unsigned apiFlags() const;

    std::vector<ApiDefinition> ApiDefinitions() const;
    void setApiDefinitions(const std::vector<ApiDefinition> &apiDefinitions);

    std::vector<std::string> features() const;
    void setFeatures(const std::vector<std::string> &features);
private:
    std::string name_;
    std::string type_;
    std::string errorString_;
    std::vector<ApiDefinition> apiDefinitions_;
    std::vector<std::string> features_;
    bool isEnabled_;
};

}



#endif
