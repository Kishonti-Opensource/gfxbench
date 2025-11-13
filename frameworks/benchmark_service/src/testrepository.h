/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTREPOSITORY_H
#define TESTREPOSITORY_H

#include "applicationconfig.h"
#include "duelitem.h"
#include "configuration.h"
#include "resultitem.h"
#include "session.h"
#include "testinfo.h"
#include "testitem.h"

#include <string>
#include <vector>
#include <set>



class DataGateway;



class TestRepository
{
public:
    TestRepository(DataGateway &DataGateway, const ApplicationConfig& applicationConfig);

    void loadTestsFromJsonString(const std::string &json, unsigned int renderApiFlags, unsigned int computeApiFlags);
    void addTestIncompReason(const std::string &testId, const std::string &reason);
    void removeTestIncompReason(const std::string &testId, const std::string &reason);
    void addHiddenTest(const std::string &testId);
    bool needClConfigurations() const;
	bool needCudaConfigurations() const;
    bool needMetalConfigurations() const;
	
    std::vector<Configuration> configurations() const { return configurations_; }
    Configuration findConfiguration(const std::string& configurationId) const;
    Configuration selectedConfiguration() const;
    int selectedConfigurationIndex() const { return selectedConfigurationIndex_; }
    void selectConfiguration(int configurationIndex);
    void selectConfiguration(const std::string &configurationId);
    void addConfiguration(const Configuration &configuration);
	void clearConfigurations() { configurations_.clear(); selectedConfigurationIndex_ = -1; }

    std::vector<TestItem> tests() const { return testItems_; }
    TestItem findTest(const std::string& testId) const;
    TestItem& findTest(const std::string& testId);
    std::string getFeaturedTestName() const { return featuredTestName_; }
    void setTestSelection(const std::string &testId, bool isSelected);
    void toggleAllTestSelection();
    void setGroupSelection(const std::string &groupId, bool isSelected);
    std::vector<TestItem> allAvailableTests() const;
    std::vector<TestItem> selectedAvailableTests() const;
  
    std::vector<Session> sessions() const;
    int sessionIndex(long long sessionId) const;
    std::vector<ResultItem> bestResults() const;
    ResultItem findBestResult(const std::string &resultId) const;
    std::vector<ResultItem> resultsForSession(long long sessionId) const;
    std::vector<CompareResult> compareResults(
            const std::string& resultId,
            const std::string& filter,
            bool hideDesktop) const;
    std::vector<DuelItem> duelResults(
            const std::string& api,
            const std::string& deviceA,
            const std::string& apiB,
            const std::string& deviceB) const;
private:
    void createTestItems();
    std::vector<ResultItem> createResultItems(const std::vector<tfw::ResultGroup> &results, bool addNA, bool useAllTest) const;
    void updateGroupSelections();
    void updateAvailability(TestItem &testItem, const Configuration &configuration);
    bool apiMatches(
            const std::vector<tfw::ApiDefinition> &testVersions,
            const std::vector<tfw::ApiDefinition> &deviceVersions) const;

    const ApplicationConfig* applicationConfig_;
    std::vector<Configuration> configurations_;
    mutable std::vector<TestInfo> testInfos_;
    mutable std::vector<TestInfo> allTestInfos_;
    std::vector<TestItem> testItems_;
    int selectedConfigurationIndex_;
    std::string featuredTestName_;
    std::vector<std::string> hiddenTests_;
    DataGateway *dataGateway_;
};



#endif
