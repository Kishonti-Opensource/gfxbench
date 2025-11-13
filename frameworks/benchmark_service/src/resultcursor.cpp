/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "resultcursor.h"



int BENCHMARK_SERVICE_API ResultCursor::getColumnCount() const
{
    return COLUMN_COUNT;
}



const char* BENCHMARK_SERVICE_API ResultCursor::getColumnName(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_MAJOR: return "major";
    case COLUMN_MINOR: return "minor";
    default: return "";
    }
}



ResultCursor::ResultCursor() :
    mCallback(nullptr)
{}



int BENCHMARK_SERVICE_API ResultCursor::getCount() const
{
    return static_cast<int>(
            mResultGroup.results().size() +
            2 + // benchmark version and renderer
            mResultGroup.flags().size() +
            mResultGroup.charts().size());
}



int BENCHMARK_SERVICE_API ResultCursor::getType(int column) const
{
    return getVariant(column).type;
}



bool BENCHMARK_SERVICE_API ResultCursor::getBoolean(int column) const
{
    return getVariant(column).toBoolean();
}



short BENCHMARK_SERVICE_API ResultCursor::getShort(int column) const
{
    return static_cast<short>(getVariant(column).toLong());
}



int BENCHMARK_SERVICE_API ResultCursor::getInt(int column) const
{
    return static_cast<int>(getVariant(column).toLong());
}



long long BENCHMARK_SERVICE_API ResultCursor::getLong(int column) const
{
    return getVariant(column).toLong();
}



float BENCHMARK_SERVICE_API ResultCursor::getFloat(int column) const
{
    return static_cast<float>(getVariant(column).toDouble());
}



double BENCHMARK_SERVICE_API ResultCursor::getDouble(int column) const
{
    return getVariant(column).toDouble();
}



void BENCHMARK_SERVICE_API ResultCursor::getBlobPointer(
        int column,
        const char** pointer,
        size_t* size) const
{
    mBuffer = getVariant(column).toString();
    *pointer = &mBuffer[0];
    *size = mBuffer.size();
}



bool BENCHMARK_SERVICE_API ResultCursor::isNull(int column) const
{
    return getVariant(column).type == Cursor::FIELD_TYPE_NULL;
}



int BENCHMARK_SERVICE_API ResultCursor::getPosition() const
{
    return mRow;
}



bool BENCHMARK_SERVICE_API ResultCursor::moveToPosition(int row)
{
    mRow = row;
    return (0 <= row) && (row < getCount());
}



void BENCHMARK_SERVICE_API ResultCursor::setCallback(CursorCallback* callback)
{
    mCallback = callback;
}



void BENCHMARK_SERVICE_API ResultCursor::destroy()
{
    mLock.reset();
}



void ResultCursor::setResultGroup(const tfw::ResultGroup& resultGroup)
{
    if (mCallback != nullptr) {
        mCallback->dataSetWillBeInvalidated();
        mResultGroup = resultGroup;
        mCallback->dataSetInvalidated();
    } else {
        mResultGroup = resultGroup;
    }
}



ResultCursor* ResultCursor::lock() {
    /*
    * std::shared_ptr-s are not passed through the API boundary to maintain a COM-like
    * interface. mLock keeps the reference count above zero while the object is referenced
    * outside the service layer. Calling destroy releases the lock.
    */
    mLock = shared_from_this();
    return mLock.get();
}



Variant ResultCursor::getVariant(int column) const
{
    size_t row = mRow;

    if (row < mResultGroup.results().size()) {
        const tfw::Result& result = mResultGroup.results().at(row);
        switch (column) {
        case COLUMN_ROW_ID: return static_cast<long long>(mRow);
        case COLUMN_TITLE: return result.resultId();
        case COLUMN_MAJOR: return result.score();
        case COLUMN_MINOR: return result.unit();
        default: return Variant();
        }
    }
    row -= static_cast<int>(mResultGroup.results().size());

    if (row == 0) {
        switch (column) {
        case COLUMN_ROW_ID: return static_cast<long long>(mRow);
        case COLUMN_TITLE: return "Benchmark version";
        case COLUMN_MAJOR: return mResultGroup.results().empty() ? std::string() : mResultGroup.results().front().benchmarkVersion();
        default: return Variant();
        }
    }
    --row;

    if (row == 0) {
        switch (column) {
        case COLUMN_ROW_ID: return static_cast<long long>(mRow);
        case COLUMN_TITLE: return "Renderer";
        case COLUMN_MAJOR: return mResultGroup.results().empty() ? std::string() : mResultGroup.results().front().gfxResult().renderer();
        default: return Variant();
        }
    }
    --row;

    if (row < mResultGroup.flags().size()) {
        switch (column) {
        case COLUMN_ROW_ID: return static_cast<long long>(mRow);
        case COLUMN_TITLE: return "*";
        case COLUMN_MAJOR: return mResultGroup.flags().at(row);
        default: return Variant();
        }
    }
    row -= mResultGroup.flags().size();

    if (row < mResultGroup.charts().size()) {
        const tfw::Chart& chart = mResultGroup.charts().at(row);
        switch (column) {
        case COLUMN_ROW_ID: return static_cast<long long>(mRow);
        case COLUMN_TITLE: return chart.chartID();
        default: return Variant();
        }
    }
    return Variant();
}
