/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KEYVALUEVISITOR_H
#define KEYVALUEVISITOR_H

#include "properties.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>



namespace sysinf
{



class KeyValueVisitor
{
public:
    KeyValueVisitor(Properties& properties) : properties(&properties) {}
    void pushPrefix(const std::string prefix) {
        if (prefix.substr(0, sizeof("CL_PLATFORM_") - 1) == "CL_PLATFORM_") {
            std::string temp = prefix.substr(sizeof("CL_PLATFORM_") - 1);
            std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
            prefixes.push_back(temp);
        } else if (prefix.substr(0, sizeof("CL_DEVICE_") - 1) == "CL_DEVICE_") {
            std::string temp = prefix.substr(sizeof("CL_DEVICE_") - 1);
            std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
            prefixes.push_back(temp);
        } else if (prefix == "CL_DRIVER_VERSION") {
            prefixes.push_back("driver_version");
        } else if (prefix == "widthPixels") {
            prefixes.push_back("res/x");
        } else if (prefix == "heightPixels") {
            prefixes.push_back("res/y");
        } else if (prefix == "xDpi") {
            prefixes.push_back("dpi/x");
        } else if (prefix == "yDpi") {
            prefixes.push_back("dpi/y");
        } else if (prefix == "pictureWidthPixels") {
            prefixes.push_back("pic/x");
        } else if (prefix == "pictureHeightPixels") {
            prefixes.push_back("pic/y");
        } else if (prefix == "pictureResolutionMP") {
            prefixes.push_back("pic/mp");
        } else if (prefix == "videoWidthPixels") {
            prefixes.push_back("vid/x");
        } else if (prefix == "videoHeightPixels") {
            prefixes.push_back("vid/y");
        } else if (prefix == "videoResolutionMP") {
            prefixes.push_back("vid/mp");
        } else if (prefix == "isRemovable") {
            prefixes.push_back("is/removable");
        } else if (prefix == "isConnected") {
            prefixes.push_back("is/connected");
        } else if (prefix == "isCharging") {
            prefixes.push_back("is/charging");
        } else if (prefix == "hasAutofocus") {
            prefixes.push_back("has/autofocus");
        } else if (prefix == "hasFaceDetection") {
            prefixes.push_back("has/face_detection");
        } else if (prefix == "hasFlash") {
            prefixes.push_back("has/flash");
        } else if (prefix == "hasHdr") {
            prefixes.push_back("has/hdr");
        } else if (prefix == "hasTouchFocus") {
            prefixes.push_back("has/touch_focus");
        } else {
            prefixes.push_back(prefix);
        }
    }
    void popPrefix() { prefixes.pop_back(); }
    std::string prefix() const {
        std::string result;
        if (!prefixes.empty()) {
            result = prefixes.front();
        }
        for (size_t i = 1; i < prefixes.size(); ++i) {
            result += '/' + prefixes.at(i);
        }
        return result;
    }

    void operator()(const std::string& name, const std::string& value) {
        pushPrefix(name);
        properties->set(prefix(), trim(value));
        popPrefix();
    }
    void operator()(const std::string& name, const std::vector<std::string>& value) {
        std::ostringstream oss;
        if (!value.empty()) {
            oss << trim(value.front());
        }
        for (size_t i = 1; i < value.size(); ++i) {
            oss << ' ' << trim(value[i]);
        }
        pushPrefix(name);
        properties->set(prefix(), oss.str());
        popPrefix();
    }
    template<class T> typename std::enable_if<std::is_arithmetic<T>::value, void>::type operator()
        (const std::string& name, T value)
    {
        pushPrefix(name);
        properties->set(prefix(), value);
        popPrefix();
    }
    template<class T> typename std::enable_if<std::is_arithmetic<T>::value, void>::type operator()
        (const std::string& name, const std::vector<T>& value)
    {
        std::ostringstream oss;
        if (!value.empty()) {
            oss << value.front();
        }
        for (size_t i = 1; i < value.size(); ++i) {
            oss << ' ' << value[i];
        }
        pushPrefix(name);
        properties->set(prefix(), oss.str());
        popPrefix();
    }
    template<class T> typename std::enable_if<!std::is_arithmetic<T>::value, void>::type operator()
        (const std::string& name, const T& value)
    {
		bool isApi = (name == "gl") 
			|| (name == "gles") 
			|| (name == "egl")
			|| (name == "cl") 
			|| (name == "cuda")
			|| (name == "metal")
			|| (name == "vulkan")
			|| (name == "directx") 
			|| (name == "directx12");

        if (isApi) {
            pushPrefix("api");
        }

        pushPrefix(name);
        if ((name == "gl") || (name == "gles") || (name == "egl")) {
            pushPrefix("features");
            value.applyVisitor(*this);
            popPrefix();
        } else {
            value.applyVisitor(*this);
        }
        popPrefix();

        if (isApi) {
            popPrefix();
        }
    }
    template<class T> typename std::enable_if<!std::is_arithmetic<T>::value, void>::type operator()
        (const std::string& name, const std::vector<T>& value)
    {
        pushPrefix(name);
        operator()("count", value.size());
        for (size_t i = 0; i < value.size(); ++i) {
            std::ostringstream oss;
            oss << i;
            operator()(oss.str(), value.at(i));
        }
        popPrefix();
    }
    void operator()(const std::string& name, const SensorInfo& sensorInfo) {
        int counter = 0;
        pushPrefix(name);
        for (auto i = sensorInfo.sensors.begin(); i != sensorInfo.sensors.end(); ++i) {
            std::ostringstream oss;
            oss << counter;
            pushPrefix(oss.str());
            operator()("name", i->first);
            operator()("type", i->second);
            popPrefix();
            ++counter;
        }
        operator()("count", counter);
        popPrefix();
    }
private:
    Properties* properties;
    std::vector<std::string> prefixes;

    static std::string trim(std::string s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }
};



}



#endif // KEYVALUEVISITOR_H
