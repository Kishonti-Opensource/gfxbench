/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "corporatedatagateway.h"

#include "ng/log.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>

#include <sstream>



CorporateDataGateway::CorporateDataGateway(const ApplicationConfig& applicationConfig)
:
    mApplicationConfig(&applicationConfig)
{}



void CorporateDataGateway::addResults(
        const Session &session,
        const tfw::ResultGroup &resultGroup)
{
    DataGateway::addResults(session, resultGroup);
    if (resultGroup.results().empty()) {
        return;
    }
    Poco::File(mLastSessionPath).createDirectories();
    Poco::FileOutputStream file(
            mLastSessionPath + "/" + resultGroup.results()[0].testId() + ".json");
    std::string json = resultGroup.toJsonString();
    file.write(json.c_str(), json.size());
}



void CorporateDataGateway::addSession(const Session &session)
{
    mLastSessionPath = mApplicationConfig->appDataPath + "/results/" +
            Poco::DateTimeFormatter::format(Poco::DateTime(), "%Y_%m_%d_%h_%M_%S");
    DataGateway::addSession(session);
}



void CorporateDataGateway::openCompareDatabase(const std::string& path)
{
    try {
        DataGateway::openCompareDatabase(path);
    } catch (const DatabaseException& e) {
        NGLOG_WARN("Compare database error: %s", e.what());
    }
}



std::vector<CompareResult> CorporateDataGateway::getResultsById(
        const ResultItem& resultItem,
        const std::string& deviceFilter,
        bool hideDesktop)
{
    try {
        return DataGateway::getResultsById(resultItem, deviceFilter, hideDesktop);
    } catch (const DatabaseException& e) {
        NGLOG_WARN("Compare database error: %s", e.what());
    }
    return std::vector<CompareResult>();
}



std::vector<CompareResult> CorporateDataGateway::getResultsByDevice(
        const std::string& api,
        const std::string& deviceId)
{
    try {
        DataGateway::getResultsByDevice(api, deviceId);
    } catch (const DatabaseException& e) {
        NGLOG_WARN("Compare database error: %s", e.what());
    }
    return std::vector<CompareResult>();
}



tfw::Descriptor CorporateDataGateway::getDescriptorByTestId(const std::string& testId)
{
    Poco::File file(mApplicationConfig->configPath + "/" + testId + ".json");
    if (!file.exists()) {
        NGLOG_DEBUG("Loading %s.json from resource", testId);
        return DataGateway::getDescriptorByTestId(testId);
    }
    NGLOG_DEBUG("Loading %s", file.path());
    Poco::FileInputStream stream(file.path());
    std::ostringstream json;
    json << stream.rdbuf();
    tfw::Descriptor descriptor;
    descriptor.fromJsonString(json.str());
    return descriptor;
}
