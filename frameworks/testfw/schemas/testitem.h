/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTITEM_H
#define TESTITEM_H

#include "testinfo.h"



namespace tfw
{

class Configuration;



class TestItem: public Serializable
{
public:
    TestItem();

    void fromJsonValue(const ng::JsonValue &jsonValue);
    ng::JsonValue toJsonValue() const;

    std::string testId() const { return testInfo_.testName() + variantPostfix(); }
    std::string variantOf() const { return testInfo_.testName(); }
    std::string variantName() const { return variantPostfix(); }
    std::string groupId() const { return testInfo_.groupName(); }
    std::string description() const { return testInfo_.testName() + "_desc"; }
    std::string baseId() const { return testInfo_.baseId(); }
    std::string apiPrefix() const { return testInfo_.apiPrefix(); }

    TestInfo testInfo() const { return testInfo_; }
    void setTestInfo(const TestInfo &testInfo) { testInfo_ = testInfo; }

    std::string variantPostfix() const {
        return (variantIndex_ < testInfo_.variantPostfixes().size()) ?
                testInfo_.variantPostfixes().at(variantIndex_) : std::string();
    }
    int variantIndex() const { return static_cast<int>(variantIndex_); }
    void setVariantIndex(int variantIndex) { variantIndex_ = variantIndex; }

    bool isFirstInGroup() const { return isFirstInGroup_; }
    void setFirstInGroup(bool isFirstInGroup) { isFirstInGroup_ = isFirstInGroup; }

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
private:
    TestInfo testInfo_;
    size_t variantIndex_;
    bool isFirstInGroup_;
    bool isSelected_;
    bool isGroupSelectionEnabled_;
    bool isGroupSelected_;
    bool isVisible_;
    std::vector<std::string> incompReasons_;
};

}



#endif
