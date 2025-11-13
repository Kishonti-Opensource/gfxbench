/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DUELITEM_H
#define DUELITEM_H

#include "compareresult.h"
#include "resultitem.h"

#include <string>



class DuelItem
{
public:
    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;

    DuelItem();
    DuelItem(const ResultItem& resultItem, double scoreA, double scoreB);
private:
    enum Column {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_ICON,
        COLUMN_UNIT,
        COLUMN_SCORE_A,
        COLUMN_SCORE_B,

        COLUMN_COUNT
    };

    int64_t mRowId;
    std::string mTitle;
    std::string mIcon;
    std::string mUnit;
    double mScoreA;
    double mScoreB;
};



#endif
