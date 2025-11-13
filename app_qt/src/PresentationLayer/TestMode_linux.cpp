/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "TestMode.h"



TestMode::TestMode()
{
    (void)m_brightnessChanged;
}

TestMode::~TestMode()
{
    leave();
}

void TestMode::saveBrightness()
{
}

void TestMode::setBrightness(double)
{
}

void TestMode::restoreBrightness()
{
}

void TestMode::setSleepEnabled(bool)
{
}
