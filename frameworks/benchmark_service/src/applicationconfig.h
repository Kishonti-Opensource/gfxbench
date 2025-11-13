/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef APPLICATIONCONFIG_H
#define APPLICATIONCONFIG_H

#include <string>
#include <vector>
#include "systeminfo.h"


class ApplicationConfig
{
public:
    ApplicationConfig(): needsSyncBasedOnDate(true), isCorporateVersion(false), isWindowModeEnabled(false) {}
    
    std::string productId;
    std::string productVersion;
    std::string platformId; //windows, linux, macosx, ios, android, 
    std::string installerName;
    std::string packageName;
    std::string locale;
    bool needsSyncBasedOnDate;
    bool isCorporateVersion;
    bool isWindowModeEnabled;

    std::string configPath;
    std::string dataPath;
    std::string pluginPath;
    std::string appDataPath;
    std::string synchronizationPath;
    std::string imagePath;
    std::string assetImagePath;
    
    std::string getTestListJsonName (const sysinf::SystemInfo& systemInfo) const;
    std::vector<std::string> getSyncFlags (const sysinf::SystemInfo& systemInfo) const;
};



#endif
