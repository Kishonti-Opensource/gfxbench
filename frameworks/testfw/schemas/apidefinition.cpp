/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "apidefinition.h"

#include <sstream>

using namespace tfw;



void ApiDefinition::fromJsonValue(const ng::JsonValue &jvalue)
{
    if (!jvalue["type"].isString())
        throw std::runtime_error("Type of required field is incorrect: 'type'");
    setType(typeFromString(jvalue["type"].string()));
    setMajor((int)jvalue["major"].numberD(-1));
    setMinor((int)jvalue["minor"].numberD(-1));
    
    ng::JsonValue extensionsJValue = jvalue["extensions"];
    if(!extensionsJValue.isNull() && extensionsJValue.isArray()) {
        for(size_t i = 0; i < extensionsJValue.size(); ++i) {
            extensions_.push_back(extensionsJValue[i].stringD(""));
        }
    }
}

ng::JsonValue ApiDefinition::toJsonValue() const
{
    ng::JsonValue jversion;
    jversion["type"] = typeToString(type());

#ifdef __QNX__
jversion["major"] = major_;
jversion["minor"] = minor_;
#else
    jversion["major"] = major();
    jversion["minor"] = minor();
#endif // __QNX__

    for (std::vector<std::string>::const_iterator it = extensions_.begin(); it != extensions_.end(); ++it) {
        jversion["extensions"].push_back(*it);
    }
    return jversion;
}

void ApiDefinition::fromVersionString(const std::string &versionString)
{
    std::string buffer = versionString;
    for (size_t i = 0; i < buffer.size(); ++i) {
        if ((buffer[i] < '0') || ('9' < buffer[i])) {
            buffer[i] = ' ';
        }
    }
    std::istringstream iss(buffer);
    iss >> major_;
    iss >> minor_;
}

ApiDefinition::Type ApiDefinition::typeFromString(const std::string& typeString)
{
    if (typeString == "GL")
        return ApiDefinition::GL;
    if (typeString == "ES")
        return ApiDefinition::ES;
    if (typeString == "DX")
        return ApiDefinition::DX;
    if (typeString == "METAL")
        return ApiDefinition::METAL;
    if (typeString == "CL")
        return ApiDefinition::CL;
    if (typeString == "RS")
        return ApiDefinition::RS;
    if (typeString == "CUDA")
        return ApiDefinition::CUDA;
    if (typeString == "VULKAN")
        return ApiDefinition::VULKAN;
    if (typeString == "DX12")
	    return ApiDefinition::DX12;
    if (typeString == "NOT_DEFINED")
	    return ApiDefinition::NOT_DEFINED;
    throw std::runtime_error("Incorrect api type string");
}

std::string ApiDefinition::typeToString(Type type)
{
    switch (type)
    {
    case ApiDefinition::GL:
        return "GL";
    case ApiDefinition::ES:
        return "ES";
    case ApiDefinition::DX:
        return "DX";
    case ApiDefinition::METAL:
        return "METAL";
    case ApiDefinition::CL:
        return "CL";
    case ApiDefinition::RS:
        return "RS";
    case ApiDefinition::CUDA:
        return "CUDA";
    case ApiDefinition::VULKAN:
        return "VULKAN";
	case ApiDefinition::DX12:
		return "DX12";
	case ApiDefinition::NOT_DEFINED:
		return "NOT_DEFINED";
    default:
        return "";
    }
}

std::string ApiDefinition::toFormattedString() const
{
    std::ostringstream oss;
    switch (type_)
    {
    case ApiDefinition::GL:
        oss << "OpenGL ";
        break;
    case ApiDefinition::ES:
        oss << "OpenGL ES ";
        break;
    case ApiDefinition::DX:
        oss << "DirectX ";
        break;
    case ApiDefinition::METAL:
        oss << "Metal ";
        break;
    case ApiDefinition::CL:
        oss << "OpenCL ";
        break;
    case ApiDefinition::RS:
        oss << "RenderScript ";
        break;
    case ApiDefinition::CUDA:
        oss << "CUDA ";
        break;
    case ApiDefinition::VULKAN:
        oss << "VULKAN ";
        break;        
	case ApiDefinition::DX12:
		oss << "DX12 ";
		break;
    default:
        assert(false);
        oss << "Unknown API ";
        break;
    }
    oss << major_ << '.' << minor_;
    for (std::vector<std::string>::const_iterator it = extensions_.begin(); it != extensions_.end(); ++it) {
        if(it == extensions_.begin()) {
            oss << " extensions: ";
        } else {
            oss << ", ";
        }
        oss << *it;
    }
    return oss.str();
}

bool ApiDefinition::isCompatibleWith(const ApiDefinition &other) const
{
    if (type_ != other.type_) return false;
    
    if (major_ < other.major_) return false;
    if (major_ > other.major_) return true;
    
    // major is equal
    
    if (minor_ < other.minor_) return false;
    if (minor_ > other.minor_) return true;
    
    // minor is equal
    
    std::vector<std::string> otherExtensions = other.extensions();
    if(!otherExtensions.empty()) {
        if(extensions_.empty()) return false;
        
        for (std::vector<std::string>::const_iterator it = otherExtensions.begin(); it != otherExtensions.end(); ++it) {
            
            std::vector<std::string>::const_iterator found = std::find(extensions_.begin(), extensions_.end(), *it);
            if (found == extensions_.end()) {
                return false;
            }
        }
    }
    
    return true;
}
