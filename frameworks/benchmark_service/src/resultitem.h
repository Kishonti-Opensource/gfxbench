/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULTITEM_H
#define RESULTITEM_H

#include "testinfo.h"
#include "variant.h"
#include "benchmarkservice.h"

#include "schemas/descriptors.h"
#include "schemas/result.h"

#include <string>



class ResultItem
{
public:
    ResultItem();

    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;

    std::string testId() const;
    std::string variantOf() const;
    std::string variantName() const;
    std::string groupId() const;
    std::string description() const;
    std::string baseResultId() const;
    std::string baseId() const;
    std::string apiPrefix() const;
    std::string iconPath() const { return imagePath_ + "/" + resultId() + ".png"; }

    long long rowId() const;
    std::string resultId() const;
    double score() const;
    std::string unit() const;
    bool hasSecondaryScore() const;
    double secondaryScore() const;
    std::string secondaryUnit() const;
    BenchmarkService::ResultStatus status() const;
    std::string errorString() const;
    std::string configurationName() const;
    const std::vector<std::string>& flags() const;
    bool isFlagged() const;

    TestInfo testInfo() const;
    void setTestInfo(const TestInfo &testInfo);

    tfw::ResultGroup resultGroup() const;
    void setResultGroup(const tfw::ResultGroup &result);

    std::string variantPostfix() const;
    int variantIndex() const;
    void setVariantIndex(int variantIndex);

    std::string resultPostfix() const;
    void setResultTypeIndex(int resultTypeIndex);

    bool isFirstInGroup() const;
    void setFirstInGroup(bool isFirstInGroup);

    bool isVisible() const { return true; }

    void setImagePath(const std::string& imagePath) { imagePath_ = imagePath; }

	std::string deviceName() const;
	void setDeviceName(const std::string& deviceName);

    std::string formatPrimaryScore() const;
    std::string formatSecondaryScore() const;
private:
    enum Column {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_DESCRIPTION,
        COLUMN_MAJOR,
        COLUMN_MINOR,
        COLUMN_ICON,
        COLUMN_TEST_ID,
        COLUMN_GROUP,
        COLUMN_VARIANT_OF,
        COLUMN_VARIANT_NAME,
        COLUMN_STATUS,
        COLUMN_PRIMARY_SCORE,
        COLUMN_PRIMARY_UNIT,
        COLUMN_SECONDARY_SCORE,
        COLUMN_SECONDARY_UNIT,
		COLUMN_DEVICE_NAME,

        COLUMN_COUNT
    };

    TestInfo testInfo_;
    size_t variantIndex_;
    size_t resultTypeIndex_;

    tfw::ResultGroup resultGroup_;
    std::string imagePath_;
    bool isFirstInGroup_;

	std::string deviceName_;
};



#endif
