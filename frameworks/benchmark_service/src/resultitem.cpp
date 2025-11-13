/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "resultitem.h"

#include "resultcursor.h"

#include <sstream>



ResultItem::ResultItem() :
    variantIndex_(0),
    resultTypeIndex_(0),
    isFirstInGroup_(false)
{}



int ResultItem::columnCount()
{
    return COLUMN_COUNT;
}



const char* ResultItem::columnName(int column)
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_DESCRIPTION: return "description";
    case COLUMN_MAJOR: return "major";
    case COLUMN_MINOR: return "minor";
    case COLUMN_ICON: return "icon";
    case COLUMN_TEST_ID: return "testId";
    case COLUMN_GROUP: return "group";
    case COLUMN_VARIANT_OF: return "variantOf";
    case COLUMN_VARIANT_NAME: return "variantName";
    case COLUMN_STATUS: return "status";
    case COLUMN_PRIMARY_SCORE: return "primaryScore";
    case COLUMN_PRIMARY_UNIT: return "primaryUnit";
    case COLUMN_SECONDARY_SCORE: return "secondaryScore";
    case COLUMN_SECONDARY_UNIT: return "secondaryUnit";
	case COLUMN_DEVICE_NAME: return "deviceName";
    default: return "";
    }
}



Variant ResultItem::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return rowId();
    case COLUMN_TITLE: return resultId();
    case COLUMN_DESCRIPTION: return description();
    case COLUMN_MAJOR: return formatPrimaryScore();
    case COLUMN_MINOR: return formatSecondaryScore();
    case COLUMN_ICON: return iconPath();
    case COLUMN_TEST_ID: return testId();
    case COLUMN_GROUP: return groupId();
    case COLUMN_VARIANT_OF: return variantOf() + resultPostfix();
    case COLUMN_VARIANT_NAME: return variantName();
    case COLUMN_STATUS: return status();
    case COLUMN_PRIMARY_SCORE: return score();
    case COLUMN_PRIMARY_UNIT: return unit();
    case COLUMN_SECONDARY_SCORE: return secondaryScore();
    case COLUMN_SECONDARY_UNIT: return secondaryUnit();
	case COLUMN_DEVICE_NAME: return deviceName();
    default: return Variant();
    }
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



long long ResultItem::rowId() const
{
    return resultGroup_.rowId();
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


std::string ResultItem::deviceName() const
{
	return deviceName_;
}


void ResultItem::setDeviceName(const std::string& deviceName)
{
	deviceName_ = deviceName;
}

BenchmarkService::ResultStatus ResultItem::status() const
{
    if (resultTypeIndex_ >= resultGroup_.results().size()) return BenchmarkService::NOT_AVAILABLE;

    switch (resultGroup_.results().at(resultTypeIndex_).status()) {
    case tfw::Result::OK:
        return BenchmarkService::OK;
    case tfw::Result::CANCELLED:
        return BenchmarkService::CANCELLED;
    default:
        return BenchmarkService::FAILED;
    }
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



tfw::ResultGroup ResultItem::resultGroup() const
{
    return resultGroup_;
}



void ResultItem::setResultGroup(const tfw::ResultGroup &resultGroup)
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



std::string ResultItem::formatPrimaryScore() const
{
    switch (status()) {
    case BenchmarkService::OK: {
        std::ostringstream oss;
        oss << score() << ' ' << unit() << (isFlagged() ? "*" : "");
        return oss.str();
    }
    case BenchmarkService::CANCELLED:
        return "STATUS_CANCELLED";
    case BenchmarkService::FAILED:
        return "STATUS_FAILED";
    default:
        return "Results_NA";
    }
}



std::string ResultItem::formatSecondaryScore() const
{
    if (status() == BenchmarkService::FAILED) {
        if (errorString() == "Results_NA") {
            return "";
        } else {
            return errorString();
        }
    }
    if (status() != BenchmarkService::OK) {
        return "";
    }
    std::ostringstream oss;
    if (secondaryScore() >= 0.0) {
        oss << '(' << secondaryScore() << ' ' << secondaryUnit() << ')';
    }
    if (!configurationName().empty()) {
        if (!oss.str().empty()) {
            oss << '\n';
        }
        oss << configurationName();
    }
    return oss.str();
}
