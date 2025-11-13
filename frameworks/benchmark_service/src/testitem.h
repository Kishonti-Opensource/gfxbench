/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTITEM_H
#define TESTITEM_H

#include "testinfo.h"
#include "variant.h"



class Configuration;



class TestItem
{
public:
    TestItem();

    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;

    std::string testId() const { return testInfo_.testName() + variantPostfix(); }
    std::string variantOf() const { return testInfo_.testName(); }
    std::string variantName() const { return variantPostfix(); }
    std::string groupId() const { return testInfo_.groupName(); }
    std::string description() const { return testInfo_.testName() + "_desc"; }
    std::string baseId() const { return testInfo_.baseId(); }
    std::string apiPrefix() const { return testInfo_.apiPrefix(); }
    std::string iconPath() const { return imagePath_ + "/" + testId() + ".png"; }

    TestInfo testInfo() const { return testInfo_; }
    void setTestInfo(const TestInfo &testInfo) { testInfo_ = testInfo; }

    std::string variantPostfix() const {
        return (variantIndex_ < testInfo_.variantPostfixes().size()) ?
                testInfo_.variantPostfixes().at(variantIndex_) : std::string();
    }
    int variantIndex() const { return static_cast<int>(variantIndex_); }
    void setVariantIndex(int variantIndex) { variantIndex_ = variantIndex; }

    bool isSelected() const { return isSelected_; }
    void setSelected(bool selected) { isSelected_ = selected; }

    bool isGroupSelectionEnabled() const { return isGroupSelectionEnabled_; }
    void setGroupSelectionEnabled(bool enabled) { isGroupSelectionEnabled_ = enabled; }

    bool isGroupSelected() const { return isGroupSelected_; }
    void setGroupSelected(bool selected) { isGroupSelected_ = selected; }

    bool isVisible() const { return isVisible_; }
    void setVisible(bool visible) { isVisible_ = visible; }

    bool isAvailable() const { return incompReasons_.empty(); }
    std::string incompatibleText() const { return incompReasons_.empty() ? "" : incompReasons_.back(); }

    void pushIncompatibilityReason(const std::string& reason);
    void removeIncompatibilityReason(const std::string& reason);

    bool requires(const std::string &feature) const;

    void setImagePath(const std::string& imagePath) { imagePath_ = imagePath; }
private:
    enum Column {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_ICON,
        COLUMN_DESCRIPTION,
        COLUMN_INCOMPATIBILITY_TEXT,
        COLUMN_ENABLED,
        COLUMN_CHECKED,
        COLUMN_GROUP,
        COLUMN_VARIANT_OF,
        COLUMN_VARIANT_NAME,
        COLUMN_RUNALONE,
		COLUMN_API,

        COLUMN_COUNT
    };
    TestInfo testInfo_;
    size_t variantIndex_;
    bool isSelected_;
    bool isGroupSelectionEnabled_;
    bool isGroupSelected_;
    bool isVisible_;
    std::vector<std::string> incompReasons_;
    std::string imagePath_;
};



#endif
