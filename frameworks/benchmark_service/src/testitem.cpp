/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testitem.h"

#include "configuration.h"
#include "cursor.h"

#include <algorithm>
#include <ng/log.h>



TestItem::TestItem():
    variantIndex_(0),
    isSelected_(false)
{}



int TestItem::columnCount()
{
    return COLUMN_COUNT;
}



const char* TestItem::columnName(int column)
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_ICON: return "icon";
    case COLUMN_DESCRIPTION: return "description";
    case COLUMN_INCOMPATIBILITY_TEXT: return "incompatibilityText";
    case COLUMN_ENABLED: return "isEnabled";
    case COLUMN_CHECKED: return "isChecked";
    case COLUMN_GROUP: return "group";
    case COLUMN_VARIANT_OF: return "variantOf";
    case COLUMN_VARIANT_NAME: return "variantName";
    case COLUMN_RUNALONE: return "isRunalone";
	case COLUMN_API: return "api";
    default: return "";
    }
}



Variant TestItem::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return reinterpret_cast<long long>(this);
    case COLUMN_TITLE: return testId();
    case COLUMN_ICON: return iconPath();
    case COLUMN_DESCRIPTION: return description();
    case COLUMN_INCOMPATIBILITY_TEXT: return incompatibleText();
    case COLUMN_ENABLED: return isAvailable();
    case COLUMN_CHECKED: return isSelected();
    case COLUMN_GROUP: return groupId();
    case COLUMN_VARIANT_OF: return variantOf();
    case COLUMN_VARIANT_NAME: return variantName();
    case COLUMN_RUNALONE: return requires("runalone");
	case COLUMN_API: return apiPrefix();
    default: return Variant();
    }
}



bool TestItem::requires(const std::string &feature) const {
    const std::vector<std::string> &requirements = testInfo_.requirements();
    return std::binary_search(requirements.begin(), requirements.end(), feature);
}




void TestItem::pushIncompatibilityReason(const std::string& reason) {
    if (std::find(incompReasons_.begin(), incompReasons_.end(), reason) == incompReasons_.end()) {
        incompReasons_.push_back(reason);
    }
}




void TestItem::removeIncompatibilityReason(const std::string& reason) {
    incompReasons_.erase(std::remove(incompReasons_.begin(), incompReasons_.end(), reason), incompReasons_.end());
}
