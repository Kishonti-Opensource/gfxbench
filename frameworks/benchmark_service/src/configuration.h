/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "schemas/apidefinition.h"

#include "variant.h"

#include <string>
#include <vector>



class Configuration
{
public:
    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;

    Configuration();

    std::string name() const;
    void setName(const std::string &name);

    std::string type() const;
    void setType(const std::string &type);

    long long rowId() const;
    void setRowId(long long rowId);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isChecked() const;
    void setChecked(bool checked);

    std::string errorString() const;
    void setErrorString(const std::string& errorString);

    unsigned apiFlags() const;

    std::vector<tfw::ApiDefinition> ApiDefinitions() const;
    void setApiDefinitions(const std::vector<tfw::ApiDefinition> &apiDefinitions);

    std::vector<std::string> features() const;
    void setFeatures(const std::vector<std::string> &features);
    bool hasFeature(std::string feature) const;
private:
    enum {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_ICON,
        COLUMN_TYPE,
        COLUMN_ERROR,
        COLUMN_ENABLED,
        COLUMN_CHECKED,
		COLUMN_API,

        COLUMN_COUNT
    };

    std::string name_;
    std::string type_;
    std::string errorString_;
    std::vector<tfw::ApiDefinition> apiDefinitions_;
    std::vector<std::string> features_;
    long long rowId_;
    bool isEnabled_;
    bool isChecked_;
};



#endif
