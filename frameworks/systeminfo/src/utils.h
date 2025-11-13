/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <sstream>
#include "properties.h"

namespace sysinf
{
    template <class T>
    std::string toString(const T &val)
    {
        std::stringstream os;
        os << val;
        return os.str();
    }
    
    template <class T>
    T fromString(const std::string &str)
    {
        T val;
        std::stringstream is(str);
        is >> val;
        return val;
    }
    
    std::vector<std::string> splitString(const std::string &str, const std::string &separator);
    
    std::string joinStrings(const std::vector<std::string> &strings, const std::string &separator);
    
    std::string CreateIndexedKey(std::string base, std::string postfix, int index);
    
    const std::string GetStringPropWithKey(const Properties *props, const std::string &key);
    int GetIntPropWithKey(const Properties *props, const std::string &key);
    int64_t GetLongPropWithKey(const Properties *props, const std::string &key);
    bool GetBoolPropWithKey(const Properties *props, const std::string &key);
    double GetDoublePropWithKey(const Properties *props, const std::string &key);
}

#endif
