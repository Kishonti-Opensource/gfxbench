/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "chartcollector.h"

#include "runtimeinfo.h"
#include "schemas/result.h"

#include <Poco/Clock.h>
#include <Poco/Timer.h>

#include <sstream>



using namespace tfw;



class ChartCollector::Private
{
public:
    Private():
        callback(*this, &Private::collect)
    {}

    RuntimeInfo* runtimeInfo;
    Poco::Clock clock;
    Poco::Clock::ClockVal startTime;
    Poco::Timer timer;
    Poco::TimerCallback<Private> callback;
    int intervalMilliSeconds;
    bool isRunning;
    Chart processorFrequencyChart;
    Chart batteryTemperatureChart;
    Chart batteryLevelChart;
    
    ResultGroup createResultGroup();
    void initializeProcessorFrequencyChart();
    void initializeBatteryLevelChart();
    void initializeBatteryTemperatureChart();
    void collect(Poco::Timer&);
};



/**
 * Contructor.
 */
ChartCollector::ChartCollector():
    d(new Private)
{
    d->runtimeInfo = 0;
    d->isRunning = false;
}



/**
 * Destructor. Stops collection if started.
 */
ChartCollector::~ChartCollector()
{
    stop();
}



/**
 * Starts data collection. Does nothing if called more than once.
 * @param runtimeInfo data source for charts.
 * @param periodMilliSeconds time interval between data collections.
 */
void ChartCollector::start(RuntimeInfo& runtimeInfo, int intervalMilliSeconds)
{
    if (d->isRunning) {
        return;
    }

    d->runtimeInfo = &runtimeInfo;

    d->initializeProcessorFrequencyChart();
    d->initializeBatteryLevelChart();
    d->initializeBatteryTemperatureChart();

    d->startTime = d->clock.elapsed();
    d->timer.setPeriodicInterval(intervalMilliSeconds);
    d->timer.start(d->callback);
}



/**
 * Stops data collection. Calling accessors is safe after this returns. Calling 
 * this is safe even if not started.
 * @returns a ResultGroup containing all collected charts.
 */
ResultGroup ChartCollector::stop()
{
    d->timer.stop();
    d->isRunning = false;
    return d->createResultGroup();
}



ResultGroup ChartCollector::Private::createResultGroup()
{
    ResultGroup resultGroup;
    resultGroup.charts().push_back(processorFrequencyChart);
    resultGroup.charts().push_back(batteryTemperatureChart);
    resultGroup.charts().push_back(batteryLevelChart);
    return resultGroup;
}



void ChartCollector::Private::initializeProcessorFrequencyChart()
{
    processorFrequencyChart = Chart();
    processorFrequencyChart.setChartID("frequency");
    processorFrequencyChart.setSampleAxis("Freq. (MHz)");
    processorFrequencyChart.setDomainAxis("Time (ms)");
    processorFrequencyChart.domain().setName(processorFrequencyChart.domainAxis());
    for (int i = 0; i < runtimeInfo->cpuCount(); ++i) {
        std::ostringstream name;
        name << "CPU" << i;
        Samples samples;
        samples.setName(name.str().c_str());
        processorFrequencyChart.values().push_back(samples);
    }
    Samples samples;
    samples.setName("GPU");
    processorFrequencyChart.values().push_back(samples);
}



void ChartCollector::Private::initializeBatteryLevelChart()
{
    batteryLevelChart = Chart();
    batteryLevelChart.setChartID("level");
    batteryLevelChart.setSampleAxis("Level (%)");
    batteryLevelChart.setDomainAxis("Time (ms)");
    batteryLevelChart.domain().setName(batteryLevelChart.domainAxis());
    Samples samples;
    samples.setName("Level");
    batteryLevelChart.values().push_back(samples);
}



void ChartCollector::Private::initializeBatteryTemperatureChart()
{
    batteryTemperatureChart = Chart();
    batteryTemperatureChart.setChartID("temperature");
    batteryTemperatureChart.setSampleAxis("Temp. (\xC2\xB0""C)");
    batteryTemperatureChart.setDomainAxis("Time (ms)");
    batteryTemperatureChart.domain().setName(batteryTemperatureChart.domainAxis());
    Samples samples;
    samples.setName("Temperature");
    batteryTemperatureChart.values().push_back(samples);
}



void ChartCollector::Private::collect(Poco::Timer&)
{
    double currentTime = (clock.elapsed() - startTime) / 1000.0;

    processorFrequencyChart.domain().values().push_back(currentTime);
    batteryTemperatureChart.domain().values().push_back(currentTime);
    batteryLevelChart.domain().values().push_back(currentTime);

    for (int i = 0; i < runtimeInfo->cpuCount(); ++i) {
        processorFrequencyChart.values().at(i).values().push_back(
                runtimeInfo->currentCpuFrequencyMHz(i));
    }
    processorFrequencyChart.values().at(runtimeInfo->cpuCount()).values().push_back(
            runtimeInfo->currentGpuFrequencyMHz());

    batteryLevelChart.values().front().values().push_back(
            runtimeInfo->batteryLevelPercent());

    batteryTemperatureChart.values().front().values().push_back(
            runtimeInfo->batteryTemperatureCelsius());
}
