/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef APIDEFINITION_H
#define APIDEFINITION_H

#include "serializable.h"

#include <string>
#include <vector>
#include <algorithm>

#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

namespace tfw {
    class ApiDefinition : public Serializable
    {
    public:
        virtual ng::JsonValue toJsonValue() const;
        virtual void fromJsonValue(const ng::JsonValue& value);
        void fromVersionString(const std::string &versionString);

		enum Type
		{
			GL = 1,
			ES = 2,
			DX = 4,
			METAL = 8,
			CL = 16,
			RS = 32,
			CUDA = 64,
			VULKAN = 128,
			DX12 = 256,
			NOT_DEFINED = 512
        };

        static std::string typeToString(Type type);
        static Type typeFromString(const std::string &typeString);

        ApiDefinition() :
            type_(GL),
            major_(-1),
            minor_(-1),
			deviceid_("no device"),
            deviceIndex_(-1) {}

        ApiDefinition(Type type, int major, int minor) :
            type_(type),
            major_(major),
            minor_(minor),
			deviceid_("no device"),
            deviceIndex_(-1) {}

        ApiDefinition(Type type, int major, int minor, const std::vector<std::string> &extensions) :
            type_(type),
            major_(major),
            minor_(minor),
			deviceid_("no device"),
            deviceIndex_(-1),
			extensions_(extensions) {}

        Type type() const { return type_; }
        void setType(Type type) { type_ = type; }
        int major() const { return major_; }
        void setMajor(int major) { major_ = major; }
        int minor() const { return minor_; }
        void setMinor(int minor) { minor_ = minor; }
		std::string deviceId() const { return deviceid_; }
		void setDeviceId(const std::string& deviceid) { deviceid_ = deviceid; }
        int deviceIndex() const { return deviceIndex_; }
        void setDeviceIndex(int deviceIndex) { deviceIndex_ = deviceIndex; }
        std::vector<std::string> extensions() const { return extensions_; }
        void setExtensions(const std::vector<std::string> &extensions) { extensions_ = extensions; }
        std::string toFormattedString() const;
        bool isCompatibleWith(const ApiDefinition &other) const;
    private:
        Type type_;
        int major_;
        int minor_;
		std::string deviceid_;
        int deviceIndex_ = -1;
        std::vector<std::string> extensions_;
    };


inline ApiDefinition::Type operator|(ApiDefinition::Type a, ApiDefinition::Type b)
{
	return static_cast<ApiDefinition::Type>(static_cast<int>(a) | static_cast<int>(b));
}

}//!namespace tfw

#endif // APIDEFINITION_H
