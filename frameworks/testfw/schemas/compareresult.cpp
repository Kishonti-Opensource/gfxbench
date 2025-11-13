/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compareresult.h"

using namespace tfw;



void CompareResult::fromJsonValue(const ng::JsonValue &jsonValue)
{
    variant_ = jsonValue["variant_"].string();
    testBase_ = jsonValue["testBase_"].string();
    subType_ = jsonValue["subType_"].string();
    os_ = jsonValue["os_"].string();
    api_ = jsonValue["api_"].string();
    testFamily_ = jsonValue["testFamily_"].string();
    deviceName_ = jsonValue["deviceName_"].string();
    deviceImage_ = jsonValue["deviceImage_"].string();
    score_ = jsonValue["score_"].number();
    fps_ = jsonValue["fps_"].number();
}



ng::JsonValue CompareResult::toJsonValue() const
{
    ng::JsonValue jCompareResult;
    jCompareResult["variant_"] = variant_;
    jCompareResult["testBase_"] = testBase_;
    jCompareResult["subType_"] = subType_;
    jCompareResult["os_"] = os_;
    jCompareResult["api_"] = api_;
    jCompareResult["testFamily_"] = testFamily_;
    jCompareResult["deviceName_"] = deviceName_;
    jCompareResult["deviceImage_"] = deviceImage_;
    jCompareResult["score_"] = score_;
    jCompareResult["fps_"] = fps_;
    return jCompareResult;
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



std::string CompareResult::gpu() const
{
    return gpu_;
}



void CompareResult::setGpu(const std::string &gpu)
{
    gpu_ = gpu;
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
