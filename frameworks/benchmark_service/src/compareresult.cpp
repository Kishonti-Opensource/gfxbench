/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compareresult.h"

#include "cursor.h"



int CompareResult::columnCount()
{
    return COLUMN_COUNT;
}



const char* CompareResult::columnName(int column)
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_DEVICE_NAME: return "title";
    case COLUMN_ICON: return "icon";
    case COLUMN_API: return "api";
    case COLUMN_TEST_ID: return "testId";
    case COLUMN_MAX_SCORE: return "maxScore";
    case COLUMN_PRIMARY_SCORE: return "primaryScore";
    case COLUMN_PRIMARY_UNIT: return "primaryUnit";
    case COLUMN_SECONDARY_SCORE: return "secondaryScore";
    case COLUMN_SECONDARY_UNIT: return "secondaryUnit";
    default: return "";
    }
}



Variant CompareResult::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return rowId();
    case COLUMN_DEVICE_NAME: return deviceName();
    case COLUMN_API: return api();
    case COLUMN_ICON: return deviceImage();
    case COLUMN_TEST_ID: return testId();
    case COLUMN_MAX_SCORE: return maxScore();
    case COLUMN_PRIMARY_SCORE: return score();
    case COLUMN_PRIMARY_UNIT: return primaryUnit();
    case COLUMN_SECONDARY_SCORE: return fps();
    case COLUMN_SECONDARY_UNIT: return secondaryUnit();
    default: return Variant();
    }
}



long long CompareResult::rowId() const {
    return rowId_;
}



void CompareResult::setRowId(long long id) {
    rowId_ = id;
}



std::string CompareResult::variant() const
{
    return variant_;
}



void CompareResult::setVariant(const std::string &variant)
{
    if(variant == "on")
        variant_ = "";
    else
        variant_ = variant;
}



std::string CompareResult::testBase() const
{
    return testBase_;
}



void CompareResult::setTestBase(const std::string &testBase)
{
    testBase_ = testBase;
}



std::string CompareResult::subType() const
{
    return subType_;
}



void CompareResult::setSubType(const std::string &subType)
{
    subType_ = subType;
}



std::string CompareResult::os() const
{
    return os_;
}



void CompareResult::setOs(const std::string &os)
{
    os_ = os;
}



std::string CompareResult::api() const
{
    return api_;
}



void CompareResult::setApi(const std::string &api)
{
    api_ = api;
}



std::string CompareResult::testFamily() const
{
    return testFamily_;
}



void CompareResult::setTestFamily(const std::string &testFamily)
{
    testFamily_ = testFamily;
}



std::string CompareResult::deviceName() const
{
    return deviceName_;
}



void CompareResult::setDeviceName(const std::string &deviceName)
{
    deviceName_ = deviceName;
}



std::string CompareResult::deviceImage() const
{
    return deviceImage_;
}



void CompareResult::setDeviceImage(const std::string &deviceImage)
{
    deviceImage_ = deviceImage;
}



std::string CompareResult::vendor() const
{
    return vendor_;
}



void CompareResult::setVendor(const std::string &vendor)
{
    vendor_ = vendor;
}



std::string CompareResult::architecture() const
{
    return architecture_;
}



void CompareResult::setArchitecture(const std::string &architecture)
{
    architecture_ = architecture;
}




std::string CompareResult::primaryUnit() const
{
    return primaryUnit_;
}



void CompareResult::setPrimaryUnit(const std::string &unit)
{
    primaryUnit_ = unit;
}



std::string CompareResult::secondaryUnit() const
{
    return secondaryUnit_;
}



void CompareResult::setSecondaryUnit(const std::string &unit)
{
    secondaryUnit_ = unit;
}



std::string CompareResult::gpu() const
{
    return gpu_;
}



void CompareResult::setGpu(const std::string &gpu)
{
    gpu_ = gpu;
}




double CompareResult::maxScore() const
{
    return maxScore_;
}



void CompareResult::setMaxScore(double maxScore)
{
    maxScore_ = maxScore;
}




double CompareResult::score() const
{
    return score_;
}



void CompareResult::setScore(double score)
{
    score_ = score;
}



double CompareResult::fps() const
{
    return fps_;
}



void CompareResult::setFps(double fps)
{
    fps_ = fps;
}



std::string CompareResult::resultId() const
{
    std::string ret = api() + "_" + testBase();
    if(variant() != "")
        ret += "_" + variant();
    if(subType() != "")
        ret += "_" + subType();
    return ret;
}



std::string CompareResult::testId() const
{
    std::string ret = api() + "_" + testBase();
    if(variant() != "")
        ret += "_" + variant();
    return ret;
}



std::string CompareResult::compareId() const
{
    std::string ret = testBase();
    if(variant() != "")
        ret += "_" + variant();
    if(subType() != "")
        ret += "_" + subType();
    return ret;
}
