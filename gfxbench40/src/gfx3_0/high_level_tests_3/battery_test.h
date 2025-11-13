/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef BATTERYTEST_H
#define BATTERYTEST_H

#include "test_wrapper.h"
#include "kcl_os.h"
#include <vector>
#include "ng/timer.h"

#define MAX_CPU_CORE_COUNT 32

class BatteryTest : public TestWrapper
{
private:
	struct StatusInfo
	{
		unsigned int m_frameCount;
		unsigned int m_time;
		float m_score;
		float m_fps;
		double m_battery;
	};

	std::vector<StatusInfo> m_subresults;
    
    ng::cpu_timer m_battery_timer ;
    ng::cpu_timer m_loop_timer ;

	double m_batteryChargeDrop;
	double m_initialCharge;
	double m_measureStartCharge;
	float m_score;
	unsigned int m_loopCount;
    bool m_loopForever;
    
    void updateScore(double currentCharge) ;

public:
	BatteryTest(const GlobalTestEnvironment* const gte, TestBase* wrappedTest, unsigned int loopCount = 30);
	virtual KCL::KCL_Status init0();
	

	virtual bool animate(const int time);
	virtual bool render0(const char* screenshotName);
	const char* getUom() const  { return " min"; };
	virtual void getTestResult(PrintableResult** results, int* count) const;

	inline float getScore () const	{ return m_score; };

protected:
	double getBatteryCharge() const	{ return KCL::g_os->GetBatteryLevel();	}
};

template <class T>
class BatteryTestA : public BatteryTest
{
public:
	BatteryTestA(const GlobalTestEnvironment* const gte, unsigned int loopCount = 30) : BatteryTest(gte, new T(gte), gte->GetTestDescriptor()->m_loop_count > 0 ? gte->GetTestDescriptor()->m_loop_count : loopCount)
	{
	}
};

#endif

