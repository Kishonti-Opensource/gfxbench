/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CRASHREPORTER_H
#define CRASHREPORTER_H

#include <string>



class CrashReporter
{
public:
    virtual ~CrashReporter() {}
    virtual void start() = 0;
};



class NullCrashReporter: public CrashReporter
{
public:
    virtual void start() {}
};



#endif
