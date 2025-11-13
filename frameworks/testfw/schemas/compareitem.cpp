/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compareitem.h"

using namespace tfw;



void CompareItem::fromJsonValue(const ng::JsonValue &jsonValue)
{
    ResultItem::fromJsonValue(jsonValue["yourResult_"]);
    compareResult_.fromJsonValue(jsonValue["compareResult_"]);
}



ng::JsonValue CompareItem::toJsonValue() const
{
    ng::JsonValue jCompareItem;
    jCompareItem["yourResult_"] = ResultItem::toJsonValue();
    jCompareItem["compareResult_"] = compareResult_.toJsonValue();
    return jCompareItem;
}



std::string CompareItem::compareDeviceName() const
{
    return compareResult_.deviceName();
}



std::string CompareItem::compareDeviceImage() const
{
    return compareResult_.deviceImage();
}



double CompareItem::compareScore() const
{
    return compareResult_.score();
}



double CompareItem::compareFps() const 
{
    return compareResult_.fps();
}



void CompareItem::setCompareResult(const CompareResult &compareResult)
{
    compareResult_ = compareResult;
}



tfw::Result::Status CompareItem::compareStatus() const
{
    return compareResult_.score() > 0 ? tfw::Result::OK : tfw::Result::FAILED;
}



std::string CompareItem::compareErrorString() const
{
    return "Results_NA";
}
