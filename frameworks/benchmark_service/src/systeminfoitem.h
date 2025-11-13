/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SYSTEMINFOITEM_H
#define SYSTEMINFOITEM_H

#include "listcursor.h"
#include "variant.h"

#include <memory>
#include <string>
#include <vector>



class SystemInfoAttribute
{
public:
    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;
    SystemInfoAttribute(const std::string& name, const Variant& value) : mName(name), mValue(value) {}
    std::string name() const { return mName; }
    Variant value() const { return mValue; }
private:
    enum Column {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_VALUE,

        COLUMN_COUNT
    };

    std::string mName;
    Variant mValue;
};



class SystemInfoItem
{
public:
    static int columnCount();
    static const char* columnName(int column);
    Variant get(int column) const;

    long long rowId() const { return mRowId; }
    void setRowId(long long rowId) { mRowId = rowId; }

    std::string name() const { return mName; }
    void setName(const std::string& name) { mName = name; }

    std::string major() const { return mMajor; }
    void setMajor(const std::string& major) { mMajor = major; }

    std::string minor() const { return mMinor; }
    void setMinor(const std::string& minor) { mMinor = minor; }

    std::string iconPath() const;

    const std::vector<SystemInfoAttribute>& attributes() const { return mAttributes; }
    void addAttribute(const std::string& name, const Variant& value) {
        mAttributes.push_back(SystemInfoAttribute(name, value));
    }
    
    void setImagePath(const std::string& imagePath) { mImagePath = imagePath; }
private:
    enum Column {
        COLUMN_ROW_ID,
        COLUMN_TITLE,
        COLUMN_ICON,
        COLUMN_MAJOR,
        COLUMN_MINOR,
        COLUMN_ENABLED,
        COLUMN_ATTRIBUTES_JSON,
        COLUMN_COMPACT_JSON,

        COLUMN_COUNT
    };

    long long mRowId;
    std::string mName;
    std::string mMajor;
    std::string mMinor;
    std::string mImagePath;
    std::vector<SystemInfoAttribute> mAttributes;

    bool compactAttributes() const;
    std::string attributesJson() const;
    std::string compactAttributesJson() const;
};



#endif // SYSTEMINFOITEM_H
