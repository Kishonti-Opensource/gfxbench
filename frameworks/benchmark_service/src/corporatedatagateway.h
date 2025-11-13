/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CORPORATEDATAGATEWAY_H
#define CORPORATEDATAGATEWAY_H



#include "datagateway.h"

#include "applicationconfig.h"



class CorporateDataGateway: public DataGateway
{
public:
    CorporateDataGateway(const ApplicationConfig& applicationConfig);
    void addResults(const Session& session, const tfw::ResultGroup& resultGroup) override;
    void addSession(const Session& session) override;
    void openCompareDatabase(const std::string& path) override;
    std::vector<CompareResult> getResultsById(
            const ResultItem& resultItem,
            const std::string& deviceFilter,
            bool hideDesktop) override;
    std::vector<CompareResult> getResultsByDevice(
            const std::string& api,
            const std::string& deviceId) override;
    tfw::Descriptor getDescriptorByTestId(const std::string& testId) override;
private:
    const ApplicationConfig* mApplicationConfig;
    std::string mLastSessionPath;
};



#endif // CORPORATEDATAGATEWAY_H
