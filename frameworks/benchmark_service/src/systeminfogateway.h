/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SYSTEMINFOGATEWAY_H
#define SYSTEMINFOGATEWAY_H

#include "configuration.h"
#include "systeminfo.h"
#include "systeminfoitem.h"
#include "applicationconfig.h"

#include "properties.h"



class SystemInfoGateway
{
public:
    SystemInfoGateway();
    virtual ~SystemInfoGateway();
    void collectSystemInfo();
    sysinf::Properties getProperties() const;
    sysinf::SystemInfo getSystemInfo() const;
    void updateSystemInfo(const sysinf::Properties& properties);
    std::vector<SystemInfoItem> getItems(const std::string& imagePath) const;
    std::vector<Configuration> getConfigurations() const;
    std::map<std::string, std::string> extractUnsupportedTests(
            const sysinf::Properties& properties) const;
private:
    class Private;
    std::unique_ptr<Private> d;


    bool haveAep() const;

	std::vector<Configuration> graphicsConfiguration() const;
    std::vector<Configuration> clConfigurations() const;
	std::vector<Configuration> cuConfigurations() const;
    std::vector<Configuration> mtlConfigurations() const;
};



#endif
