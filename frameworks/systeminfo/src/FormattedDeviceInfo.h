/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  FormattedDeviceInfo.h
//  GFXBench
//
//  Created by Kishonti Kft on 14/11/2013.
//
//

#ifndef __GFXBench__FormattedDeviceInfo__
#define __GFXBench__FormattedDeviceInfo__

#include <iostream>
#include <vector>

namespace sysinf {
    class PropertyFeature
    {
    public:
		PropertyFeature() {}
        PropertyFeature(const std::string s, bool b) :
        m_name(s),
        m_value(b)
        {}
        
        const std::string GetName() const { return m_name; }
        bool GetValue() const { return m_value; }
        
    private:
        std::string m_name;
        bool m_value;
    };
    
    typedef std::vector<PropertyFeature> FeatureList;
    
    class FormattedDeviceInfo
    {
    public:
        const std::string Name() const;
        const std::string Major() const;
        const std::string Minor() const;
        const FeatureList* Features() const;
        
        void SetName(std::string name);
        void SetMajor(std::string major);
        void SetMinor(std::string minor);
        void AddFeature(std::string key, bool b);
        
    private:
        std::string m_name;
        std::string m_major;
        std::string m_minor;
        
        FeatureList m_features;
    };
}

#endif /* defined(__GFXBench__FormattedDeviceInfo__) */
