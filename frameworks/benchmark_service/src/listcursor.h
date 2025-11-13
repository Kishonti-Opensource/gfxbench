/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include "cursor.h"
#include "variant.h"

#include "ng/log.h"

#include <algorithm>
#include <functional>
#include <vector>

#ifdef WIN32
//warning C4858 : discarding return value :
#pragma warning(disable: 4858)
#endif

template<class T>
class ListCursor : public Cursor, public std::enable_shared_from_this<ListCursor<T> >
{
public:
    typedef std::function<std::vector<T>(void)> Query;

    ListCursor() :
        mCallback(nullptr),
        mRow(0)
    {}

    int BENCHMARK_SERVICE_API getColumnCount() const override {
        return T::columnCount();
    }
    const char* BENCHMARK_SERVICE_API getColumnName(int column) const override {
        return T::columnName(column);
    }
    int BENCHMARK_SERVICE_API getCount() const override {
        return static_cast<int>(mItems.size());
    }
    int BENCHMARK_SERVICE_API getType(int column) const override {
        return mItems.at(mRow).get(column).type;
    }
    bool BENCHMARK_SERVICE_API getBoolean(int column) const override {
        return mItems.at(mRow).get(column).toBoolean();
    }
    short BENCHMARK_SERVICE_API getShort(int column) const override {
        return static_cast<short>(mItems.at(mRow).get(column).toLong());
    }
    int BENCHMARK_SERVICE_API getInt(int column) const override {
        return static_cast<int>(mItems.at(mRow).get(column).toLong());
    }
    long long BENCHMARK_SERVICE_API getLong(int column) const override {
        return mItems.at(mRow).get(column).toLong();
    }
    float BENCHMARK_SERVICE_API getFloat(int column) const override {
        return static_cast<float>(mItems.at(mRow).get(column).toDouble());
    }
    double BENCHMARK_SERVICE_API getDouble(int column) const override {
        return mItems.at(mRow).get(column).toDouble();
    }
    void BENCHMARK_SERVICE_API getBlobPointer(
        int column,
        const char** pointer,
        size_t* size) const override
    {
        mBuffer = mItems.at(mRow).get(column).toString();
        *pointer = &mBuffer[0];
        *size = mBuffer.size();
    }
    bool BENCHMARK_SERVICE_API isNull(int column) const override {
        return mItems.at(mRow).get(column).type == Cursor::FIELD_TYPE_NULL;
    }
    int BENCHMARK_SERVICE_API getPosition() const override {
        return mRow;
    }
    bool BENCHMARK_SERVICE_API moveToPosition(int row) override {
        mRow = row;
        return (0 <= row) && (row < getCount());
    }
    void BENCHMARK_SERVICE_API setCallback(CursorCallback* callback) override {
        mCallback = callback;
    }
    void BENCHMARK_SERVICE_API destroy() override {
        mLock.reset();
    }

    ListCursor<T>* lock() {
        /*
         * std::shared_ptr-s are not passed through the API boundary to maintain a COM-like
         * interface. mLock keeps the reference count above zero while the object is referenced
         * outside the service layer. Calling destroy releases the lock.
         */
        mLock = this->shared_from_this();
        return mLock.get();
    }
    const Query& getQuery() const { return mQuery; }
    void setQuery(const Query& query) { mQuery = query; }
    void updateItems(std::vector<T>& newItems) {
        if (mCallback != nullptr) {
            if (mItems.size() != newItems.size()) {
                mCallback->dataSetWillBeInvalidated();
                mItems.swap(newItems);
                mCallback->dataSetInvalidated();
            }
            else {
                mCallback->dataSetWillChange(0, static_cast<int>(mItems.size()) - 1);
                mItems.swap(newItems);
                mCallback->dataSetChanged(0, static_cast<int>(mItems.size()) - 1);
            }
        }
        else {
            mItems.swap(newItems);
        }
    }
private:
    std::shared_ptr<ListCursor<T> > mLock;
    std::vector<T> mItems;
    CursorCallback* mCallback;
    Query mQuery;
    int mRow;
    mutable std::string mBuffer;
};



class CursorUpdateCallback
{
public:
    virtual ~CursorUpdateCallback() {}
    virtual void dispatchCursorUpdate(const std::function<void()>& task) = 0;
};



template<class T>
class ListCursorContainer
{
public:
    ListCursorContainer() :
        mCallback(nullptr),
        mIsUpdateBlocked(true)
    {}
    void setCallback(CursorUpdateCallback* callback) {
        mCallback = callback;
    }
    void registerCursor(const std::shared_ptr<ListCursor<T> >& cursor) {
        std::remove_if(mCursors.begin(), mCursors.end(),
            [](const std::weak_ptr<ListCursor<T> > c) { return c.expired(); });
        mCursors.push_back(cursor);
        if (!mIsUpdateBlocked) {
            auto items = std::make_shared<std::vector<T> >(cursor->getQuery()());
            if (mCallback != nullptr) {
                mCallback->dispatchCursorUpdate([=]() {
                    cursor->updateItems(*items);
                    });
            }
        }
    }
    void updateCursors() {
        mIsUpdateBlocked = false;
        for (auto i = mCursors.begin(); i != mCursors.end();) {
            std::shared_ptr<ListCursor<T> > cursor = i->lock();
            if (cursor) {
                auto items = std::make_shared<std::vector<T> >(cursor->getQuery()());
                if (mCallback != nullptr) {
                    mCallback->dispatchCursorUpdate([=]() {
                        cursor->updateItems(*items);
                        });
                }
                ++i;
            }
            else {
                i = mCursors.erase(i);
            }
        }
    }
    void setBlockUpdate(bool isBlocked) {
        mIsUpdateBlocked = isBlocked;
    }
private:
    std::vector<std::weak_ptr<ListCursor<T> > > mCursors;
    CursorUpdateCallback* mCallback;
    bool mIsUpdateBlocked;
};
