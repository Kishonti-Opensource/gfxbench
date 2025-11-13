/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testinfo.h"

#include <algorithm>



ResultInfo ResultInfo::fromJsonValue(const ng::JsonValue &jsonValue)
{
    ResultInfo resultInfo;
    
    const ng::JsonValue& jresultPostfix = jsonValue["result_postfix"];
    if (!jresultPostfix.isString()) {
        throw std::runtime_error("Type of required field is incorrect: 'ResultInfo.variant_of'");
    }
    resultInfo.resultPostfix_ = jresultPostfix.string();
    
    const ng::JsonValue& junit = jsonValue["unit"];
    if(!junit.isString()) {
        throw std::runtime_error("Type of required field is incorrect: 'ResultInfo.unit'");
    }
    resultInfo.unit_ = junit.string();

    return resultInfo;
}



ng::JsonValue ResultInfo::toJsonValue() const
{
    ng::JsonValue jresultInfo;
    jresultInfo["result_postfix"] = resultPostfix_;
    jresultInfo["unit"] = unit_;
    return jresultInfo;
}



TestInfo TestInfo::fromJsonValue(const ng::JsonValue &jvalue)
{
    TestInfo testInfo;

    const ng::JsonValue& jtestName = jvalue["test_name"];
    if(!jtestName.isString()) {
        throw std::runtime_error("Type of required field is incorrect: 'TestInfo.test_name'");
    }
    testInfo.testName_ = jtestName.string();

    const ng::JsonValue& jgroupName = jvalue["group_name"];
    if (!jgroupName.isString()) {
        throw std::runtime_error("Type of required field  is incorrect: 'TestInfo.group_name'");
    }
    testInfo.groupName_ = jgroupName.string();

    const ng::JsonValue &jrunOrder = jvalue["run_order"];
    if (!jrunOrder.isNumber()) {
        throw std::runtime_error("Type of required field is incorrect: 'TestInfo.run_order'");
    }
    testInfo.runOrder_ = (int)jrunOrder.number();

    const ng::JsonValue &jvariantPostfixes = jvalue["variant_postfixes"];
    if (!jvariantPostfixes.isArray()) {
        throw std::runtime_error("Type of json object is incorrect: 'TestInfo.variant_postfixes'");
    }
    for (size_t i = 0; i < jvariantPostfixes.size(); ++i) {
        const ng::JsonValue &jvariantPostfix = jvariantPostfixes[i];
        if (!jvariantPostfix.isString()) {
            throw std::runtime_error("Type of json object is incorrect: 'TestInfo.variant_postfixes[]'");
        }
        testInfo.variantPostfixes_.push_back(jvariantPostfix.string());
    }
    
    const ng::JsonValue &jminimumApi = jvalue["minimum_api"];
    if(!jminimumApi.isArray()) {
        throw std::runtime_error("Type of json object is incorrect: 'TestInfo.minimum_api'");
    }
    for (size_t i = 0; i < jminimumApi.size(); ++i) {
        testInfo.minimumGraphicsApi_.push_back(
                tfw::Serializable::fromJsonValue<tfw::ApiDefinition>(jminimumApi[i]));
    }

    const ng::JsonValue &jminimumComputeApi = jvalue["minimum_compute_api"];
    if (jminimumComputeApi.isArray()) {
        for (size_t i = 0; i < jminimumComputeApi.size(); ++i) {
            testInfo.minimumComputeApi_.push_back(
                    tfw::Serializable::fromJsonValue<tfw::ApiDefinition>(jminimumComputeApi[i]));
        }
    }

    const ng::JsonValue &jrequirements = jvalue["requirements"];
    if (!jrequirements.isArray()) {
        throw std::runtime_error("Type of json object is incorrect: 'TestInfo.requirements'");
    }
    for (size_t i = 0; i < jrequirements.size(); ++i) {
        const ng::JsonValue &jfeature = jrequirements[i];
        if (!jfeature.isString()) {
            throw std::runtime_error("Type of json object is incorrect: 'TestInfo.requirements[]'");
        }
        testInfo.requirements_.push_back(jfeature.string());
    }
    std::sort(testInfo.requirements_.begin(), testInfo.requirements_.end());

    const ng::JsonValue& jresults = jvalue["results"];
    if (!jresults.isArray()) {
        throw std::runtime_error("Type of required field  is incorrect: 'TestInfo.results'");
    }
    for (size_t i = 0; i < jresults.size(); i++) {
        ResultInfo resultInfo = ResultInfo::fromJsonValue(jresults[i]);
        testInfo.resultTypes_.push_back(resultInfo);
    }

    // Compute the compare name from the test name.
    // TODO change this that it comes from the descriptor instead.
    testInfo.baseId_ = testInfo.testName_;
    testInfo.apiPrefix_ = "";
    std::size_t found = testInfo.baseId_.find("_");
    if (found!=std::string::npos) {
        testInfo.apiPrefix_ = testInfo.baseId_.substr(0, found + 1);
        testInfo.baseId_.replace(0, found+1, "");
    }

    return testInfo;
}



ng::JsonValue TestInfo::toJsonValue() const
{
    ng::JsonValue jtestInfo;
    jtestInfo["test_name"] = testName_;
    jtestInfo["group_name"] = groupName_;
    jtestInfo["run_order"] = runOrder_;

    for (std::vector<std::string>::const_iterator i = variantPostfixes_.begin(); i != variantPostfixes_.end(); ++i) {
        jtestInfo["variant_postfixes"].push_back(*i);
    }

    for (std::vector<tfw::ApiDefinition>::const_iterator i = minimumGraphicsApi_.begin(); i != minimumGraphicsApi_.end(); ++i) {
        jtestInfo["minimum_api"].push_back(i->toJsonValue());
    }

    for (std::vector<std::string>::const_iterator i = requirements_.begin(); i != requirements_.end(); ++i) {
        jtestInfo["requirements"].push_back(*i);
    }

    for (std::vector<ResultInfo>::const_iterator i = resultTypes_.begin(); i != resultTypes_.end(); ++i) {
        jtestInfo["results"].push_back(i->toJsonValue());
    }
    
    return jtestInfo;
}
