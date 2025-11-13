/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTINFO_H
#define TESTINFO_H

#include "apidefinition.h"

#include <string>
#include <vector>


namespace ng {
    class JsonValue;
}


namespace tfw
{

class ResultInfo
{
public:
    static ResultInfo fromJsonValue(const ng::JsonValue &jsonValue);
    ng::JsonValue toJsonValue() const;

    std::string resultPostfix() const { return resultPostfix_; }
    void resultPostfix(const std::string& resultPostfix) { resultPostfix_ = resultPostfix; }

    const std::string& unit() const { return unit_; }
    void setUnit(const std::string& unit) { unit_ = unit; }
private:
    std::string resultPostfix_;
    std::string unit_;
};



class TestInfo
{
public:
    static TestInfo fromJsonValue(const ng::JsonValue &jsonValue);
    ng::JsonValue toJsonValue() const;
    
    TestInfo() :
        runOrder_(-1)
    {}

    std::string testName() const { return  testName_; }
    std::string groupName() const { return groupName_; }
    std::string baseId() const { return baseId_; }
    std::string apiPrefix() const { return apiPrefix_; }
    const std::vector<std::string>& variantPostfixes() const { return variantPostfixes_; }
    const std::vector<ResultInfo>& resultTypes() const { return resultTypes_; }
    int runOrder() const { return runOrder_; }
    const std::vector<ApiDefinition>& minimumGraphicsApi() const { return minimumGraphicsApi_; }
    const std::vector<ApiDefinition>& minimumComputeApi() const { return minimumComputeApi_; }
    const std::vector<std::string>& requirements() const { return requirements_; }
private:
    std::string testName_;
    std::string groupName_;
    std::string baseId_;
    std::string apiPrefix_;
    int runOrder_;
    std::vector<std::string> variantPostfixes_;
    std::vector<ApiDefinition> minimumGraphicsApi_;
    std::vector<ApiDefinition> minimumComputeApi_;
    std::vector<std::string> requirements_;
    std::vector<ResultInfo> resultTypes_;
};

}



#endif
