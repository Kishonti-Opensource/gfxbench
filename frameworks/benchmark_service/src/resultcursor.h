/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULTCURSOR_H
#define RESULTCURSOR_H

#include "cursor.h"
#include "variant.h"

#include "schemas/result.h"

#include <memory>



class ResultCursor: public Cursor, public std::enable_shared_from_this<ResultCursor>
{
public:
    ResultCursor();
    int BENCHMARK_SERVICE_API getColumnCount() const override;
    const char* BENCHMARK_SERVICE_API getColumnName(int column) const override;
    int BENCHMARK_SERVICE_API getCount() const override;
    int BENCHMARK_SERVICE_API getType(int column) const override;
    bool BENCHMARK_SERVICE_API getBoolean(int column) const override;
    short BENCHMARK_SERVICE_API getShort(int column) const override;
    int BENCHMARK_SERVICE_API getInt(int column) const override;
    long long BENCHMARK_SERVICE_API getLong(int column) const override;
    float BENCHMARK_SERVICE_API getFloat(int column) const override;
    double BENCHMARK_SERVICE_API getDouble(int column) const override;
    void BENCHMARK_SERVICE_API getBlobPointer(
            int column,
            const char** pointer,
            size_t* size) const override;
    bool BENCHMARK_SERVICE_API isNull(int column) const override;
    int BENCHMARK_SERVICE_API getPosition() const override;
    bool BENCHMARK_SERVICE_API moveToPosition(int row) override;
    void BENCHMARK_SERVICE_API setCallback(CursorCallback* callback) override;
    void BENCHMARK_SERVICE_API destroy() override;

    void setResultGroup(const tfw::ResultGroup& resultGroup);
    ResultCursor* lock();
private:
    enum {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_MAJOR,
        COLUMN_MINOR,

        COLUMN_COUNT
    };

    std::shared_ptr<ResultCursor> mLock;
    tfw::ResultGroup mResultGroup;
    CursorCallback* mCallback;
    int mRow;
    mutable std::string mBuffer;

    Variant getVariant(int column) const;
};



#endif // RESULTCURSOR_H
