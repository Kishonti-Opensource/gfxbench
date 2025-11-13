/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CHARTCOLLECTOR_H
#define CHARTCOLLECTOR_H

#include <memory>



namespace tfw
{



class RuntimeInfo;
class ResultGroup;



/**
 * Collects device information periodically on a worker thread and compiles
 * the collected data into charts.
 */
class ChartCollector
{
public:
    ChartCollector();
    ~ChartCollector();
    void start(RuntimeInfo& runtimeInfo, int intervalMilliSeconds);
    ResultGroup stop();
private:
    class Private;
    std::unique_ptr<Private> d;
    
    ChartCollector(const ChartCollector&); // No copy
    ChartCollector& operator=(const ChartCollector&); // No copy
};



}



#endif // CHARTCOLLECTOR_H
