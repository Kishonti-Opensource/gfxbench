/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef HOCKEYAPPCRASHREPORTER_H
#define HOCKEYAPPCRASHREPORTER_H

#include "ApplicationConfig.h"
#include "CrashReporter.h"



class HockeyAppCrashReporter: public CrashReporter
{
public:
    HockeyAppCrashReporter(const ApplicationConfig &applicationConfig);
    ~HockeyAppCrashReporter();
    void start();
private:
    const ApplicationConfig *m_applicationConfig;
};



#endif
