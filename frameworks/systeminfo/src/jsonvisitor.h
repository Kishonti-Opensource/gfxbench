/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef JSONVISITOR_H
#define JSONVISITOR_H

#include "ng/json.h"

#include <string>
#include <type_traits>



namespace sysinf
{



class JsonVisitor
{
public:
    JsonVisitor(ng::JsonValue& jsonValue):
        mJsonValue(jsonValue)
    {}
    template<class T> void operator()(const std::string& name, const T& value) {
        fillJson(mJsonValue[name.c_str()], value);
    }
    template<class T> void operator()(const std::string& name, const std::vector<T>& value) {
        ng::JsonValue jv;
        jv.resize(0);
        for (size_t i = 0; i < value.size(); ++i) {
            jv.push_back(ng::JsonValue());
            fillJson(jv.back(), value.at(i));
        }
        mJsonValue[name.c_str()] = jv;
    }
private:
    ng::JsonValue& mJsonValue;

    void fillJson(ng::JsonValue& jv, bool value) { jv = value; }
    void fillJson(ng::JsonValue& jv, const std::string& value) { jv = value; }
    template<class T> typename std::enable_if<std::is_arithmetic<T>::value, void>::type fillJson(
            ng::JsonValue& jv, T value) { jv = static_cast<double>(value); }
    template<class T> typename std::enable_if<!std::is_arithmetic<T>::value, void>::type fillJson(
            ng::JsonValue& jv, const T& value) { applyVisitor(value, JsonVisitor(jv));
    }
};



}



#endif // JSONVISITOR_H
