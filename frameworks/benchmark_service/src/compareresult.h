/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPARERESULT_H
#define COMPARERESULT_H

#include "variant.h"

#include <string>



class CompareResult
{
public:
    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;

    CompareResult(): maxScore_(-1), score_(-1), fps_(-1) {};

    long long rowId() const;
    void setRowId(long long rowId);

    std::string variant() const;
    void setVariant(const std::string &variant);
    
    std::string testBase() const;
    void setTestBase(const std::string &testBase);

    std::string subType() const;
    void setSubType(const std::string &subType);

    std::string os() const;
    void setOs(const std::string &os);

    std::string api() const;
    void setApi(const std::string &api);

    std::string testFamily() const;
    void setTestFamily(const std::string &testFamily);

    std::string deviceName() const;
    void setDeviceName(const std::string &deviceName);

    std::string deviceImage() const;
    void setDeviceImage(const std::string &deviceImage);

    std::string vendor() const;
    void setVendor(const std::string &vendor);

    std::string gpu() const;
    void setGpu(const std::string &gpu);

    std::string architecture() const;
    void setArchitecture(const std::string &architecture);

    std::string primaryUnit() const;
    void setPrimaryUnit(const std::string &unit);

    std::string secondaryUnit() const;
    void setSecondaryUnit(const std::string &unit);

    double score() const;
    void setScore(double score);
    
    double maxScore() const;
    void setMaxScore(double maxScore);

    double fps() const;
    void setFps(double fps);

    std::string resultId() const;
    std::string testId() const;
    std::string compareId() const;
private:
    enum {
        COLUMN_ROW_ID,
        COLUMN_DEVICE_NAME,
        COLUMN_ICON,
        COLUMN_API,
        COLUMN_TEST_ID,
        COLUMN_MAX_SCORE,
        COLUMN_PRIMARY_SCORE,
        COLUMN_PRIMARY_UNIT,
        COLUMN_SECONDARY_SCORE,
        COLUMN_SECONDARY_UNIT,

        COLUMN_COUNT
    };

    long long rowId_;
    std::string variant_;
    std::string testBase_;
    std::string subType_;
    std::string os_;
    std::string api_;
    std::string testFamily_;
    std::string deviceName_;
    std::string deviceImage_;
    std::string vendor_;
    std::string gpu_;
    std::string architecture_;
    std::string primaryUnit_;
    std::string secondaryUnit_;
    double maxScore_;
    double score_;
    double fps_;
};



#endif
