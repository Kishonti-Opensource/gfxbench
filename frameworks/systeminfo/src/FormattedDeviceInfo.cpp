/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  FormattedDeviceInfo.cpp
//  GFXBench
//
//  Created by Kishonti Kft on 14/11/2013.
//
//

#include "FormattedDeviceInfo.h"

using namespace sysinf;

const std::string FormattedDeviceInfo::Name() const
{
    return m_name;
}

const std::string FormattedDeviceInfo::Major() const
{
    return m_major;
}

const std::string FormattedDeviceInfo::Minor() const
{
    return m_minor;
}

const FeatureList* FormattedDeviceInfo::Features() const
{
    return &m_features;
}


void FormattedDeviceInfo::SetName(std::string name)
{
    m_name = name;
}

void FormattedDeviceInfo::SetMajor(std::string major)
{
    m_major = major;
}

void FormattedDeviceInfo::SetMinor(std::string minor)
{
    m_minor = minor;
}

void FormattedDeviceInfo::AddFeature(std::string key, bool b)
{
    m_features.push_back(PropertyFeature(key, b));
}