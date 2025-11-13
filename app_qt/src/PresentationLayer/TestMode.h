/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <string>

class TestMode
{
public:
    static TestMode * instance()
    {
        static TestMode testMode;
        return &testMode;
    }
    
    void enter()
    {
        if (m_testModeIsOn) {
            return;
        }
        setSleepEnabled(false);
        saveBrightness();
        m_testModeIsOn = true;
    }
    
    void leave()
    {
        if (!m_testModeIsOn) {
            return;
        }
        setSleepEnabled(true);
        restoreBrightness();
        m_testModeIsOn = false;
    }
    static void setBrightness(double value);
    static void restoreBrightness();
private:
    TestMode();
    ~TestMode();

    void saveBrightness();
    void setSleepEnabled(bool enable);


    
    bool m_testModeIsOn;
	bool m_brightnessChanged;
    std::string m_backupPath;
};
