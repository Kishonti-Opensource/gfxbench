/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef JSONVISITOR_H
#define JSONVISITOR_H

#include "ng/json.h"

#include <cstdint>



namespace tfw
{



class JsonVisitor
{
public:
    JsonVisitor(ng::JsonValue& jsonValue):
        mJsonValue(jsonValue)
    {}
    virtual ~JsonVisitor() {}
    template<class T> void operator()(const std::string& name, const std::vector<T>& value) {
        ng::JsonValue jv;
        jv.resize(0);
        for (size_t i = 0; i < value.size(); ++i) {
            jv.push_back(cast(value.at(i)));
        }
        mJsonValue[name.c_str()] = jv;
    }
    template<class T> void operator()(const std::string& name, const T& value) {
        mJsonValue[name.c_str()] = cast(value);
    }
protected:
    ng::JsonValue& mJsonValue;

    template<class T> const T& cast(const T& value) { return value; }
    double cast(long value) { return (double)value; }
    double cast(unsigned long value) { return (double)value; }
    double cast(long long value) { return (double)value; }
    double cast(unsigned long long value) { return (double)value; }
};



}



#endif // JSONVISITOR_H
