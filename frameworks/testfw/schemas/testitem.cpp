/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testitem.h"

#include "configuration.h"

#include <algorithm>
#include <ng/log.h>

using namespace tfw;



TestItem::TestItem():
    variantIndex_(0),
    isFirstInGroup_(true),
    isSelected_(false)
{}



void TestItem::fromJsonValue(const ng::JsonValue &jsonValue)
{
    testInfo_.fromJsonValue(jsonValue["testInfo_"]);
    variantIndex_ = static_cast<int>(jsonValue["variantIndex_"].number());
    isFirstInGroup_ = jsonValue["isFirstInGroup_"].boolean();
    isSelected_ = jsonValue["isSelected_"].boolean();

    for(size_t i = 0; i < jsonValue["incompReasons_"].size(); ++i) {
        incompReasons_.push_back(jsonValue["incompReasons_"][i].string());
    }
}



ng::JsonValue TestItem::toJsonValue() const
{
    ng::JsonValue jTestItem;
    jTestItem["testInfo_"] = testInfo_.toJsonValue();
    jTestItem["variantIndex_"] = static_cast<int>(variantIndex_);
    jTestItem["isFirstInGroup_"] = isFirstInGroup_;
    jTestItem["isSelected_"] = isSelected_;

    int index = 0;
    for(std::vector<std::string>::const_iterator i = incompReasons_.begin(); i != incompReasons_.end(); ++i) {
        jTestItem["incompReasons_"][index] = *i;
        index++;
    }

    return jTestItem;
}



bool TestItem::requires(const std::string &feature) const {
    const std::vector<std::string> &requirements = testInfo_.requirements();
    return std::binary_search(requirements.begin(), requirements.end(), feature);
}



void TestItem::pushIncompatibilityReason(const std::string& reason) {
    if(std::find(incompReasons_.begin(), incompReasons_.end(), reason) == incompReasons_.end()) {
        incompReasons_.push_back(reason);
    }
}



void TestItem::removeIncompatibilityReason(const std::string& reason) {
    incompReasons_.erase(std::remove(incompReasons_.begin(), incompReasons_.end(), reason), incompReasons_.end());
}
