/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <IOKit/graphics/IOGraphicsLib.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Foundation/Foundation.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

#include "TestMode.h"
#include <iostream>
#include <sstream>

#include <assert.h>
#include <pthread.h>



float g_originalBrightness = -1.0;
NSString *g_originalIdleTime = 0;
bool *g_testModeIsOn = 0; //pointer
NSString *g_backupPath;
NSData *g_data;
IOPMAssertionID assertionDisplay = 0;
IOPMAssertionID assertionIdle = 0;

TestMode::TestMode()
{
    g_testModeIsOn = &m_testModeIsOn;
}

TestMode::~TestMode()
{
    leave();
    g_testModeIsOn = 0;
}

void TestMode::saveBrightness()
{
    //saving brightness
    io_iterator_t iter = 0;
    kern_return_t result = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IODisplayConnect"), &iter);
    
    if(result == kIOReturnSuccess)
    {
        io_service_t service = 0;
        while((service = IOIteratorNext(iter)))
        {
            IODisplayGetFloatParameter(service, kNilOptions, CFSTR(kIODisplayBrightnessKey), &g_originalBrightness);
            IOObjectRelease(service);
            break;
        }
    }
    if(iter)
        IOObjectRelease(iter);
}

void TestMode::setBrightness(double value)
{
    if(value > 1.0)
        value = 1.0;
    if(value < 0.0)
    {
        if(g_originalBrightness < 0)
            value = 1.0;
        else
            value = g_originalBrightness;
    }
    io_iterator_t iter = 0;
    kern_return_t result = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IODisplayConnect"), &iter);
    
    if(result == kIOReturnSuccess)
    {
        io_service_t service = 0;
        while((service = IOIteratorNext(iter)))
        {
            IODisplaySetFloatParameter(service, kNilOptions, CFSTR(kIODisplayBrightnessKey), value);
            IOObjectRelease(service);
            instance()->m_brightnessChanged = true;
            break;
        }
    }
    if(iter)
        IOObjectRelease(iter);
}

void TestMode::restoreBrightness()
{
    if(instance()->m_brightnessChanged)
    {
        double value = g_originalBrightness;
        setBrightness(value);
        instance()->m_brightnessChanged = false;
    }
}

void TestMode::setSleepEnabled(bool enable)
{
    if(!enable)
    {
        if(assertionIdle == 0 && assertionDisplay == 0)
        {
            CFStringRef reasonForActivity= CFSTR("Benchmarking");
            IOReturn success0 = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reasonForActivity, &assertionDisplay);
            IOReturn success1 = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn, reasonForActivity, &assertionIdle);
            if(success0 != kIOReturnSuccess || success1 != kIOReturnSuccess)
            {
                std::cerr << "Cannot create power management assertion." << std::endl;
                assertionDisplay = assertionIdle = 0;
            }
        }
    }
    else
    {
        if(assertionIdle > 0 && assertionDisplay > 0)
        {
            IOReturn success0 = IOPMAssertionRelease(assertionDisplay);
            IOReturn success1 = IOPMAssertionRelease(assertionIdle);
            if(success0 != kIOReturnSuccess || success1 != kIOReturnSuccess)
            {
                std::cerr << "Cannot release power management assertion." << std::endl;
            }
            assertionDisplay = assertionIdle = 0;
        }
    }
}
