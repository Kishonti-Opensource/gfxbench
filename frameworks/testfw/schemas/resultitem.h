/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULTITEM_H
#define RESULTITEM_H

#include "descriptors.h"
#include "session.h"
#include "testinfo.h"
#include "result.h"

#include <string>



namespace tfw
{

class ResultItem: public Serializable
{
public:
    ResultItem();

    void fromJsonValue(const ng::JsonValue &jsonValue);
    ng::JsonValue toJsonValue() const;

    std::string testId() const;
    std::string variantOf() const;
    std::string variantName() const;
    std::string groupId() const;
    std::string description() const;
    std::string baseResultId() const;
    std::string baseId() const;
    std::string apiPrefix() const;

    std::string resultId() const;
    double score() const;
    std::string unit() const;
    bool hasSecondaryScore() const;
    double secondaryScore() const;
    std::string secondaryUnit() const;
    Result::Status status() const;
    std::string errorString() const;
    std::string configurationName() const;
    const std::vector<std::string>& flags() const;
    bool isFlagged() const;

    TestInfo testInfo() const;
    void setTestInfo(const TestInfo &testInfo);

    ResultGroup resultGroup() const;
    void setResultGroup(const ResultGroup &result);

    std::string variantPostfix() const;
    int variantIndex() const;
    void setVariantIndex(int variantIndex);

    std::string resultPostfix() const;
    void setResultTypeIndex(int resultTypeIndex);

    bool isFirstInGroup() const;
    void setFirstInGroup(bool isFirstInGroup);

    bool isVisible() const { return true; }
private:
    TestInfo testInfo_;
    size_t variantIndex_;
    size_t resultTypeIndex_;

    ResultGroup resultGroup_;
    bool isFirstInGroup_;
};

}



#endif
