/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  dataformatter.h
//  GFXBench
//
//  Created by Kishonti Kft on 13/11/2013.
//
//

#ifndef GFXBench_dataformatter_h
#define GFXBench_dataformatter_h

#include <vector>
#include "properties.h"
#include "FormattedDeviceInfo.h"

namespace sysinf {
	struct ApiInfo
    {
        std::string name;
        std::string info;
    };
	
	struct ApiDevice
	{
		std::string name;
		std::vector<ApiInfo> infos;
	};

	struct ApiPlatform
	{
		std::string name;
		std::vector<ApiDevice> devices;
	};

    typedef std::vector<sysinf::FormattedDeviceInfo> FormattedStream;
    typedef std::vector<ApiPlatform> ApiStream;
    
    class DataFormatter
    {
    public:
        FormattedStream GetFormattedStream() const;
        ApiStream GetApiStream(const std::string api_key) const;
        std::vector<std::string> GetNotSupportedTests() const;
        std::vector<std::string> GetHiddenTests() const;
        
        void SetProperties(const Properties *props) { m_properties = props; }
        const Properties* GetProperties() const { return m_properties; }
        
    private:
        bool AutoSetMajor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key) const;
        bool AutoSetMinor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key) const;
        bool AutoSetIndexedMajor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key, int index) const;
        bool AutoSetIndexedMinor(const Properties *const props, FormattedDeviceInfo *info, const std::string &key, int index) const;
        
        const Properties *m_properties;
    };
}

#endif
