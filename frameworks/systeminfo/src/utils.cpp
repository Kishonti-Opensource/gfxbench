/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "utils.h"
#include <cstdlib>

std::vector<std::string> sysinf::splitString(const std::string &str, const std::string &separator)
{
    std::vector<std::string> out;
    std::vector<size_t> segments;
    segments.push_back(0);
    size_t last = 0;
    size_t index = str.find(separator, last);
    while(index != str.npos)
    {
        segments.push_back(index+separator.length());
        last = index+separator.length();
        index = str.find(separator, last);
    }
    for(size_t i(0); i < segments.size(); i++)
    {
        if(i != segments.size() - 1)
        {
            out.push_back(str.substr(segments[i], segments[i+1]-segments[i]-separator.length()));
        }
        else
        {
            out.push_back(str.substr(segments[i], str.npos-segments[i]));
        }
    }
    return out;
}

std::string sysinf::joinStrings(const std::vector<std::string> &strings, const std::string &separator)
{
    std::stringstream os;
    for(size_t i(0); i < strings.size(); i++)
    {
        os << strings[i];
        if(i != strings.size() -1)
        {
            os << separator;
        }
    }
    return os.str();
}

std::string sysinf::CreateIndexedKey(std::string base, std::string postfix, int index)
{
    std::stringstream ss;
    ss << base << "/" << index << "/" << postfix;
    return ss.str();
}

const std::string sysinf::GetStringPropWithKey(const Properties *props, const std::string &key)
{
    if(props->has(key))
    {
        const Property *prop = props->get(key);
        return prop->getString();
    }
    
    return "";
}

int sysinf::GetIntPropWithKey(const Properties *props, const std::string &key)
{
    if(props->has(key))
    {
        const Property *prop = props->get(key);
        std::string stype = prop->getType();
        if(stype == "int" || stype == "string")
        {
            return prop->getInt();
        }
    }
    
    return 0;
}

int64_t sysinf::GetLongPropWithKey(const Properties *props, const std::string &key)
{
    if(props->has(key))
    {
        const Property *prop = props->get(key);
        std::string stype = prop->getType();
        if(stype == "long" || stype == "string")
        {
            return prop->getLong();
        }
    }
    
    return 0;
}

bool sysinf::GetBoolPropWithKey(const Properties *props, const std::string &key)
{
    if(props->has(key))
    {
        const Property *prop = props->get(key);
        std::string stype = prop->getType();
        if(stype == "bool" || stype == "string")
        {
            return prop->getBool();
        }
    }
    
    return false;
}

double sysinf::GetDoublePropWithKey(const Properties *props, const std::string &key)
{
    if(props->has(key))
    {
        const Property *prop = props->get(key);
        std::string stype = prop->getType();
        if(stype == "double" || stype == "string")
        {
            return prop->getDouble();
        }
    }
    
    return 0;
}


