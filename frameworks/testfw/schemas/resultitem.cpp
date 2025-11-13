/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "resultitem.h"

using namespace tfw;


ResultItem::ResultItem() :
    variantIndex_(0),
    resultTypeIndex_(0),
    isFirstInGroup_(false)
{}



void ResultItem::fromJsonValue(const ng::JsonValue &jsonValue)
{
    testInfo_.fromJsonValue(jsonValue["testInfo_"]);
    resultGroup_.fromJsonValue(jsonValue["resultGroup_"]);
    variantIndex_ = static_cast<int>(jsonValue["variantIndex_"].number());
    resultTypeIndex_ = static_cast<int>(jsonValue["resultTypeIndex_"].number());
    isFirstInGroup_ = jsonValue["isFirstInGroup_"].boolean();
}



ng::JsonValue ResultItem::toJsonValue() const
{
    ng::JsonValue jResultItem;
    jResultItem["testInfo_"] = testInfo_.toJsonValue();
    jResultItem["resultGroup_"] = resultGroup_.toJsonValue();
    jResultItem["variantIndex_"] = static_cast<int>(variantIndex_);
    jResultItem["resultTypeIndex_"] = static_cast<int>(resultTypeIndex_);
    jResultItem["isFirstInGroup_"] = isFirstInGroup_;
    return jResultItem;
}



std::string ResultItem::testId() const
{
    return testInfo_.testName() + variantPostfix();
}



std::string ResultItem::variantOf() const
{
    return testInfo_.testName();
}



std::string ResultItem::variantName() const
{
    return variantPostfix();
}



std::string ResultItem::groupId() const
{
    return testInfo_.groupName();
}



std::string ResultItem::description() const
{
    return testInfo_.testName() + "_desc";
}



std::string ResultItem::baseResultId() const
{
    return testInfo_.baseId() + variantPostfix() + resultPostfix();
}



std::string ResultItem::baseId() const
{
    return testInfo_.baseId();
}



std::string ResultItem::apiPrefix() const
{
    return testInfo_.apiPrefix();
}



std::string ResultItem::resultId() const
{
    return testInfo_.testName() + variantPostfix() + resultPostfix();
}



double ResultItem::score() const
{
    if (resultTypeIndex_ >= resultGroup_.results().size()) return 0.0;
    return resultGroup_.results().at(resultTypeIndex_).score();
}



std::string ResultItem::unit() const
{
    if (resultTypeIndex_ >= testInfo_.resultTypes().size()) return std::string();
    return testInfo_.resultTypes().at(resultTypeIndex_).unit();
}



bool ResultItem::hasSecondaryScore() const
{
    return unit() == "Frames";
}



double ResultItem::secondaryScore() const
{
    if (resultTypeIndex_ >= resultGroup_.results().size()) return 0.0;
    return resultGroup_.results().at(resultTypeIndex_).gfxResult().fps();
}



std::string ResultItem::secondaryUnit() const
{
    return "Fps";
}



tfw::Result::Status ResultItem::status() const
{
    if (resultTypeIndex_ >= resultGroup_.results().size()) return Result::FAILED;
    return resultGroup_.results().at(resultTypeIndex_).status();
}



std::string ResultItem::errorString() const
{
    if (resultTypeIndex_ >= resultGroup_.results().size()) return "Results_NA";
    return resultGroup_.results().at(resultTypeIndex_).errorString();
}



std::string ResultItem::configurationName() const
{
    return resultGroup_.configuration();
}



const std::vector<std::string>& ResultItem::flags() const
{
    return resultGroup_.flags();
}



bool ResultItem::isFlagged() const
{
    return !resultGroup_.flags().empty();
}



TestInfo ResultItem::testInfo() const
{
    return testInfo_;
}



void ResultItem::setTestInfo(const TestInfo &testInfo)
{
    testInfo_ = testInfo;
}



ResultGroup ResultItem::resultGroup() const
{
    return resultGroup_;
}



void ResultItem::setResultGroup(const ResultGroup &resultGroup)
{
    resultGroup_ = resultGroup;
}



std::string ResultItem::variantPostfix() const
{
    return (variantIndex_ < testInfo_.variantPostfixes().size()) ?
            testInfo_.variantPostfixes().at(variantIndex_) : std::string();
}



int ResultItem::variantIndex() const
{
    return static_cast<int>(variantIndex_);
}



void ResultItem::setVariantIndex(int variantIndex) {
    variantIndex_ = variantIndex;
}



std::string ResultItem::resultPostfix() const
{
    return (resultTypeIndex_ < testInfo_.resultTypes().size()) ?
        testInfo_.resultTypes().at(resultTypeIndex_).resultPostfix() : std::string();
}



void ResultItem::setResultTypeIndex(int resultTypeIndex)
{
    resultTypeIndex_ = resultTypeIndex;
}



bool ResultItem::isFirstInGroup() const
{
    return isFirstInGroup_;
}



void ResultItem::setFirstInGroup(bool isFirstInGroup)
{
    isFirstInGroup_ = isFirstInGroup;
}
