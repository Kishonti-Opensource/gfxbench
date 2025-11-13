/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "chartcursor.h"



ChartCursor::ChartCursor():
    mCallback(nullptr)
{}



int BENCHMARK_SERVICE_API ChartCursor::getColumnCount() const
{
    return COLUMN_COUNT;
}



const char* BENCHMARK_SERVICE_API ChartCursor::getColumnName(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_DATA: return "data";
    case COLUMN_METRIC: return "metric";
    case COLUMN_UNIT: return "unit";
    default: return "";
    }
}



int BENCHMARK_SERVICE_API ChartCursor::getCount() const
{
    return static_cast<int>(mChart.values().size()) + 1;
}



int BENCHMARK_SERVICE_API ChartCursor::getType(int column) const
{
    return getVariant(column).type;
}



bool BENCHMARK_SERVICE_API ChartCursor::getBoolean(int column) const
{
    return getVariant(column).toBoolean();
}



short BENCHMARK_SERVICE_API ChartCursor::getShort(int column) const
{
    return static_cast<short>(getVariant(column).toLong());
}



int BENCHMARK_SERVICE_API ChartCursor::getInt(int column) const
{
    return static_cast<int>(getVariant(column).toLong());
}



long long BENCHMARK_SERVICE_API ChartCursor::getLong(int column) const
{
    return getVariant(column).toLong();
}



float BENCHMARK_SERVICE_API ChartCursor::getFloat(int column) const
{
    return static_cast<float>(getVariant(column).toDouble());
}



double BENCHMARK_SERVICE_API ChartCursor::getDouble(int column) const
{
    return getVariant(column).toDouble();
}



void BENCHMARK_SERVICE_API ChartCursor::getBlobPointer(
        int column,
        const char** pointer,
        size_t* size) const
{
    mBuffer = getVariant(column).toString();
    *pointer = &mBuffer[0];
    *size = mBuffer.size();
}



bool BENCHMARK_SERVICE_API ChartCursor::isNull(int column) const
{
    return getVariant(column).type == Cursor::FIELD_TYPE_NULL;
}



int BENCHMARK_SERVICE_API ChartCursor::getPosition() const
{
    return mRow;
}



bool BENCHMARK_SERVICE_API ChartCursor::moveToPosition(int row)
{
    mRow = row;
    return (0 <= row) && (row < getCount());
}



void BENCHMARK_SERVICE_API ChartCursor::setCallback(CursorCallback* callback)
{
    mCallback = callback;
}



void BENCHMARK_SERVICE_API ChartCursor::destroy()
{
    mLock.reset();
}



void ChartCursor::setChart(const tfw::Chart& chart)
{
    if (mCallback != nullptr) {
        mCallback->dataSetWillBeInvalidated();
        mChart = chart;
        mCallback->dataSetInvalidated();
    } else {
        mChart = chart;
    }
}



ChartCursor* ChartCursor::lock() {
    /*
    * std::shared_ptr-s are not passed through the API boundary to maintain a COM-like
    * interface. mLock keeps the reference count above zero while the object is referenced
    * outside the service layer. Calling destroy releases the lock.
    */
    mLock = shared_from_this();
    return mLock.get();
}



Variant ChartCursor::getVariant(int column) const
{
    if (mRow == 0) {
        switch (column) {
        case COLUMN_ROW_ID: return reinterpret_cast<long long>(&mChart.domain());
        case COLUMN_TITLE: return mChart.domain().name();
        case COLUMN_DATA: return Variant(std::string(reinterpret_cast<const char*>(mChart.domain().values().data()), sizeof(double) * mChart.domain().values().size()), true);
        case COLUMN_METRIC: return mChart.domain().name();
        default: return Variant();
        }
    } else {
        const tfw::Samples& samples = mChart.values().at(mRow - 1);
        switch (column) {
        case COLUMN_ROW_ID: return reinterpret_cast<long long>(&samples);
        case COLUMN_TITLE: return samples.name();
        case COLUMN_DATA: return Variant(std::string(reinterpret_cast<const char*>(samples.values().data()), sizeof(double) * samples.values().size()), true);
        case COLUMN_METRIC: return mChart.sampleAxis();
        default: return Variant();
        }
    }
}
