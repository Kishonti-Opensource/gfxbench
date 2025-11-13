/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "systeminfoitem.h"

#include "cursor.h"

#include "ng/json.h"

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif



int SystemInfoAttribute::columnCount()
{
    return COLUMN_COUNT;
}



const char* SystemInfoAttribute::columnName(int column)
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_VALUE: return "value";
    default: return "";
    }
}



Variant SystemInfoAttribute::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return reinterpret_cast<long long>(this);
    case COLUMN_TITLE: return mName;
    case COLUMN_VALUE: return mValue;
    default: return Variant();
    }
}



int SystemInfoItem::columnCount()
{
    return COLUMN_COUNT;
}



const char* SystemInfoItem::columnName(int column)
{
    switch (column) {
    case COLUMN_ROW_ID: return "_id";
    case COLUMN_TITLE: return "title";
    case COLUMN_ICON: return "icon";
    case COLUMN_MAJOR: return "major";
    case COLUMN_MINOR: return "minor";
    case COLUMN_ENABLED: return "isEnabled";
    case COLUMN_ATTRIBUTES_JSON: return "attributesJson";
    case COLUMN_COMPACT_JSON: return "compactAttributesJson";
    default: return "";
    }
}



Variant SystemInfoItem::get(int column) const
{
    switch (column) {
    case COLUMN_ROW_ID: return rowId();
    case COLUMN_TITLE: return name();
    case COLUMN_ICON: return iconPath();
    case COLUMN_MAJOR: return major();
    case COLUMN_MINOR: return minor();
    case COLUMN_ENABLED: return !attributes().empty() && !compactAttributes();
    case COLUMN_ATTRIBUTES_JSON: return attributesJson();
    case COLUMN_COMPACT_JSON: return compactAttributesJson();
    default: return Variant();
    }
}



std::string SystemInfoItem::iconPath() const
{
    if (mName == "device") {
        return mImagePath + "/device.png";
    } else if (mName == "display") {
        return mImagePath + "/display.png";
    } else if (mName == "chipset") {
        return mImagePath + "/chipset.png";
    } else if (mName == "cpu") {
        return mImagePath + "/cpu.png";
    } else if (mName == "gpu") {
        return mImagePath + "/gpu.png";
    } else if (mName == "OpenCL device") {
        for (size_t i = 0; i < mAttributes.size(); ++i) {
            if (mAttributes.at(i).value().toString() == "CL_DEVICE_TYPE_GPU") {
                return mImagePath + "/gpu.png";
            } else if (mAttributes.at(i).value().toString() == "CL_DEVICE_TYPE_CPU") {
                return mImagePath + "/cpu.png";
            }
        }
        return mImagePath + "/gpu.png";
    } else if (mName == "Cuda device") {
        return mImagePath + "/gpu.png";
    } else if (mName == "dx") {
        return mImagePath + "/3d_api.png";
    } else if (mName == "gl") {
        return mImagePath + "/3d_api.png";
    } else if (mName == "gles") {
        return mImagePath + "/3d_api.png";
    } else if (mName == "OpenGL") {
        return mImagePath + "/3d_api.png";
	} else if (mName == "Vulkan device") {
		return mImagePath + "/3d_api.png";
	} else if (mName == "DirectX 11") {
		return mImagePath + "/3d_api.png";
	} else if (mName == "DirectX 12") {
		return mImagePath + "/3d_api.png";
	} else if (mName == "DirectX 11 device") {
		return mImagePath + "/3d_api.png";
	} else if (mName == "DirectX 12 device") {
		return mImagePath + "/3d_api.png";
    } else if (mName == "metal") {
        return mImagePath + "/3d_api.png";
    } else if (mName == "Metal device") {
        return mImagePath + "/3d_api.png";
    } else if (mName == "memory") {
        return mImagePath + "/ram.png";
    } else if (mName == "storage") {
        return mImagePath + "/storage.png";
    } else if (mName == "features") {
        return mImagePath + "/features.png";
    } else if (mName == "battery") {
        return mImagePath + "/battery.png";
    } else if (mName == "frontCamera" || (mName == "camera" && mMinor == "CAMERA_TYPE_FRONT")) {
        return mImagePath + "/front_camera.png";
    } else if (mName == "backCamera" || (mName == "camera" && mMinor == "CAMERA_TYPE_BACK")) {
        return mImagePath + "/back_camera.png";
    } else if (mName == "os") {
#if defined(_WIN32)
        return mImagePath + "/windows.png";
#elif defined(__APPLE__) && !TARGET_OS_IPHONE
        return mImagePath + "/macosx.png";
#elif defined(__linux__)
        return mImagePath + "/linux.png";
#else
        return mImagePath + "/os.png";
#endif
    } else {
        return mImagePath + "/none.png";
    }
}



bool SystemInfoItem::compactAttributes() const
{
    return (mName == "features") || (mName == "frontCamera") || (mName == "backCamera");
}



std::string SystemInfoItem::compactAttributesJson() const
{
    if (mAttributes.empty() || !((mName == "features") || (mName == "frontCamera") || (mName == "backCamera"))) {
        return std::string();
    }
    
    ng::JsonValue json;
    for (size_t i = 0; i < mAttributes.size(); ++i) {
        const SystemInfoAttribute& attribute = mAttributes.at(i);
        switch (attribute.value().type) {
            case Cursor::FIELD_TYPE_INTEGER:
                if(mName == "features") {
                    json[attribute.name().c_str()] = attribute.value().toDouble();
                    
                } else if(mName == "frontCamera" || mName == "backCamera") {
                    std::string prefix = "has";
                    if(attribute.name().compare(0, prefix.length(), prefix) == 0) {
                        std::string attrib = attribute.name().replace(0, prefix.length(), "");
                        if(attrib == "Hdr") {
                            attrib = "HDR";
                        }
                        attrib = "Camera_" + attrib;
                        json[attrib.c_str()] = attribute.value().toDouble();
                    }
                    
                }
                break;
                
            default:
                break;
        }
    }
    return json.toString();
}



std::string SystemInfoItem::attributesJson() const
{
    if (mAttributes.empty()) {
        return std::string();
    }

    ng::JsonValue json;
    for (size_t i = 0; i < mAttributes.size(); ++i) {
        const SystemInfoAttribute& attribute = mAttributes.at(i);
        switch (attribute.value().type) {
        case Cursor::FIELD_TYPE_NULL:
            json[attribute.name().c_str()] = ng::JsonValue();
            break;
        case Cursor::FIELD_TYPE_INTEGER:
        case Cursor::FIELD_TYPE_FLOAT:
            json[attribute.name().c_str()] = attribute.value().toDouble();
            break;
        case Cursor::FIELD_TYPE_STRING:
        case Cursor::FIELD_TYPE_BLOB:
            json[attribute.name().c_str()] = attribute.value().toString();
            break;
        }
    }
    return json.toString();
}
