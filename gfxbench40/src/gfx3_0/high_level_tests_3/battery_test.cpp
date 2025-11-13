/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "battery_test.h"
#include "kcl_io.h"
#include <float.h>
#include <sstream>

using namespace KCL;

BatteryTest::BatteryTest(const GlobalTestEnvironment* const gte, TestBase* wrappedTest, unsigned int loopCount) : 
	TestWrapper(gte, wrappedTest), 
	m_initialCharge(0),
	m_measureStartCharge(0),
	m_loopCount(loopCount)
{
	m_loopForever = wrappedTest->GetSetting().m_is_endless;
	m_batteryChargeDrop = wrappedTest->GetSetting().m_battery_charge_drop;
    
    // if the battery charge drop invalid, then discard it
    m_batteryChargeDrop = (m_batteryChargeDrop<=0)?200:m_batteryChargeDrop ;
}

KCL::KCL_Status BatteryTest::init0()
{
	KCL::KCL_Status error = KCL_TESTERROR_NOERROR;
	if (KCL::g_os->IsBatteryCharging())
	{
		error = KCL_TESTERROR_BATTERYTEST_PLUGGED_ON_CHARGER;
	} else
	{
		error =  getWrappedTest()->init0();
	}
	SetRuntimeError(error);
	return error;
}

void BatteryTest::updateScore(double currentCharge)
{
    m_score = (float)(m_battery_timer.elapsed().wall*1000) / (m_measureStartCharge - currentCharge) / 1000 / 60;
}

bool BatteryTest::animate(const int time)
{
	if (!getWrappedTest())
	{
		return false;
	}

	if (KCL::g_os->IsBatteryCharging())
	{
		SetRuntimeError(KCL_TESTERROR_BATTERYTEST_PLUGGED_ON_CHARGER);
		return false;
	}

	static double lastCharge = -1;
	static int lastLoopIndex = -1;
	double currentCharge = getBatteryCharge();

	if (m_initialCharge == 0.0)
	{
		m_initialCharge = currentCharge;
		lastCharge = 0;
		lastLoopIndex = -1;
		m_score = 0;
        
        m_battery_timer = ng::cpu_timer(false);
        m_loop_timer = ng::cpu_timer(false);
        m_loop_timer.start() ;
	}
	else if (m_measureStartCharge == 0.0)
	{
		if (currentCharge < m_initialCharge)
		{
			m_measureStartCharge = currentCharge;
            m_battery_timer.start() ;
		}
	}
	else if (currentCharge > lastCharge)
	{
		m_score = -1;
		return false;
	}

	unsigned int loopIndex = time / m_settings->m_play_time;
    bool new_loop = (int)loopIndex > lastLoopIndex ;
	if (new_loop)
	{
        getWrappedTest()->finishTest() ;
        m_loop_timer.stop();

		StatusInfo statusInfo;
		memset(&statusInfo, 0, sizeof(StatusInfo));
		statusInfo.m_time = m_loop_timer.elapsed().wall*1000;
		statusInfo.m_score = getWrappedTest()->getScore();
        
        // Calculate the frame count based normalized score
        float test_length = getWrappedTest()->GetSetting().m_play_time ;
        if (test_length > 0.0f && statusInfo.m_time > 0)
        {
            statusInfo.m_score =  statusInfo.m_score * test_length / float(statusInfo.m_time);
        }
        
		statusInfo.m_battery = currentCharge;
		statusInfo.m_frameCount = getWrappedTest()->getFrames();
		statusInfo.m_fps = (float)statusInfo.m_frameCount / statusInfo.m_time * 1000;

        m_loop_timer = ng::cpu_timer(false);
        m_loop_timer.start() ;
        
        /* FIXME: nothing should return nan */
        if (statusInfo.m_score != statusInfo.m_score) { // isnan
            statusInfo.m_score = -1.0f;
        }

		if (lastLoopIndex>=0) m_subresults.push_back(statusInfo);
		getWrappedTest()->ResetFrameCounter();
		if (m_loopForever)
		{
            updateScore(currentCharge) ;
			if (loopIndex == 0)
			{
				KCL::File file("stabilityruns.csv", KCL::Write, KCL::RWDir);
				file.Printf("\"Index\", \"Time\", \"Score\", \"Frames\", \"FPS\", \"Charge\", \"Estimated battery life\"\n");
			}
			KCL::File file("stabilityruns.csv", KCL::Append, KCL::RWDir);
			StatusInfo& info = statusInfo;

			//substitute of file.Printf(format, ...) with same argument, due to osx error
			fprintf(file.getFilePointer(), "%d, %d, %f, %d, %f, %f, %f",
				loopIndex,
				time,
				info.m_score,
				info.m_frameCount,
				info.m_fps,
				info.m_battery,
				this->m_score
				);

			file.Printf("\n");
		}
	}

    if (getWrappedTest()->GetSetting().m_is_endless == false &&
        (loopIndex >= m_loopCount || ((m_measureStartCharge - currentCharge) * 100.0) >= m_batteryChargeDrop ) &&
        ((currentCharge < m_measureStartCharge) || GetSetting().m_is_debug_battery))
    {
        if (!new_loop)
        {
            getWrappedTest()->finishTest() ;
        }
        updateScore(currentCharge) ;
        return false;
    }
    
	lastCharge = currentCharge;
	lastLoopIndex = loopIndex;

	unsigned int internalTime = time % m_settings->m_play_time;
	getWrappedTest()->animate(internalTime);

	return true;
}

bool BatteryTest::render0(const char* screenshotName)
{
	return getWrappedTest()->render0(screenshotName);
}

void BatteryTest::getTestResult(PrintableResult** results, int* count) const
{
	*count = 2;
	if (!results)
	{
		return;
	}

	float minScore = -1.0f;
	float fps = 0;
    float time = 0;
    KCL::uint32 frameCount = 0;
	for (unsigned int i = 0; i < m_subresults.size(); i++)
	{
        if (m_subresults[i].m_score < minScore || minScore < 0)
		{
			minScore = m_subresults[i].m_score;
			fps = m_subresults[i].m_fps;
            time = m_subresults[i].m_time;
            frameCount = m_subresults[i].m_frameCount;
		}
	}
    
    std::vector<std::string> extraDataDiagram;
    
    for (unsigned int i = 1; i < m_subresults.size(); i++)
	{
        std::stringstream ss;
		const StatusInfo &sinf = m_subresults[i];
        ss << sinf.m_battery << "|" << sinf.m_score;
        extraDataDiagram.push_back(ss.str());
	}
    
	PrintableResult performanceResult(
		getTextureType(),
		minScore,
		KCL_Status_To_Cstr(m_runtime_error),
		fps,
		getWrappedTest()->getUom(),
		false,
		false,
		m_gte->GetTestId() + "_performance",
		GetFrameStepTime(),
        m_settings->GetScreenMode() ?  m_settings->GetTestWidth() : getViewportWidth(),
		m_settings->GetScreenMode() ?  m_settings->GetTestHeight() : getViewportHeight(),
        frameCount,
		-1,
		time,
		m_runtime_error,
        extraDataDiagram);

	PrintableResult batteryResult(
		getTextureType(),
		getScore(),
		KCL::KCL_Status_To_Cstr (m_runtime_error),
		-1.0f,
		getUom(),
		false,
		false,
		m_gte->GetTestId() + "_lifetime",
		GetFrameStepTime(),
        m_settings->GetScreenMode() ?  m_settings->GetTestWidth() : getViewportWidth(),
		m_settings->GetScreenMode() ?  m_settings->GetTestHeight() : getViewportHeight(),
		-1,
		-1,
		-1.0f,
		m_runtime_error,
        std::vector<std::string>());

	*results = new PrintableResult[2];
	(*results)[0] = performanceResult;
    (*results)[1] = batteryResult;
}
