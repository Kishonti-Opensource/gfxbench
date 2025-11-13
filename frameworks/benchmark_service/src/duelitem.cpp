/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "duelitem.h"



int DuelItem::columnCount()
{
    return COLUMN_COUNT;
}



const char* DuelItem::columnName(int column)
{

    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_ICON: return "icon";
    case COLUMN_UNIT: return "unit";
    case COLUMN_SCORE_A: return "scoreA";
    case COLUMN_SCORE_B: return "scoreB";
    default: return "";
    }
}



Variant DuelItem::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return mRowId;
    case COLUMN_TITLE: return mTitle;
    case COLUMN_ICON: return mIcon;
    case COLUMN_UNIT: return mUnit;
    case COLUMN_SCORE_A: return (mScoreA >= 0.0) ? mScoreA : Variant();
    case COLUMN_SCORE_B: return (mScoreB >= 0.0) ? mScoreB : Variant();
    default: return Variant();
    }
}



DuelItem::DuelItem() :
    mRowId(-1),
    mScoreA(-1.0),
    mScoreB(-1.0)
{}



DuelItem::DuelItem(const ResultItem& resultItem, double scoreA, double scoreB)
:
    mRowId(resultItem.rowId()),
    mTitle(resultItem.resultId()),
    mIcon(resultItem.iconPath()),
    mUnit(resultItem.unit()),
    mScoreA(scoreA),
    mScoreB(scoreB)
{}
