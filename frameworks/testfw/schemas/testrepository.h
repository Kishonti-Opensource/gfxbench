/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTREPOSITORY_H
#define TESTREPOSITORY_H

#include "compareitem.h"
#include "configuration.h"
#include "resultitem.h"
#include "session.h"
#include "testinfo.h"
#include "testitem.h"

#include <string>
#include <vector>
#include <set>



namespace tfw
{



class TestRepository
{
public:
    static std::vector<std::vector<TestItem> > singleColumnLayout(
            const std::vector<TestItem> &items);
    static std::vector<std::vector<TestItem> > multiColumnLayout(
            const std::vector<TestItem> &items);
    static std::vector<TestItem> headerRepeatedColumnLayout(
            const std::vector<TestItem> &items);
    static std::vector<std::vector<ResultItem> > singleColumnLayout(
            const std::vector<ResultItem> &items);
    static std::vector<std::vector<ResultItem> > multiColumnLayout(
            const std::vector<ResultItem> &items);
    static std::vector<ResultItem> headerRepeatedColumnLayout(
            const std::vector<ResultItem> &items);
    static std::vector<ResultItem> simpleColumnLayout(
            const std::vector<ResultItem> &items);
    static std::vector<std::vector<ResultItem> > filterNotAvailableRows(
            const std::vector<std::vector<ResultItem> > &items);
    static std::vector<ResultItem> filterNotAvailableRows(
            const std::vector<ResultItem> &items);

    TestRepository();

    void loadTestsFromJsonString(const std::string &json);
    void addTestIncompReason(const std::string &testId, const std::string &reason);
    void removeTestIncompReason(const std::string &testId, const std::string &reason);
    void setTestVisibility(const std::string &testId, bool isVisible);
    std::vector<unsigned> allowedConfigurations() const;
    bool needClConfigurations() const;

    std::vector<Configuration> configurations() const { return configurations_; }
    Configuration findConfiguration(const std::string& configurationId) const;
    Configuration selectedConfiguration() const;
    int selectedConfigurationIndex() const { return selectedConfigurationIndex_; }
    void selectConfiguration(int configurationIndex);
    void selectConfiguration(const std::string &configurationId);
    void addConfiguration(const Configuration &configuration);

    std::vector<TestItem> tests() const { return testItems_; }
    TestItem findTest(const std::string& testId) const;
    void setTestSelection(const std::string &testId, bool isSelected);
    void setGroupSelection(const std::string &groupId, bool isSelected);
    std::vector<TestItem> allAvailableTests() const;
    std::vector<TestItem> selectedAvailableTests() const;
    void addHiddenTest(const std::string &testId);

    std::vector<Session> sessions() const { return sessions_; }
    Session selectedSession() const;
    int selectedSessionIndex() const { return selectedSessionIndex_; }
    void selectSession(long long sessionId);
    void setSessions(const std::vector<Session> &sessions);

    std::vector<ResultItem> bestResults() const { return bestResults_; }
    void setBestResults(const std::vector<ResultGroup> &results);
    ResultItem findBestResult(const std::string &resultId);
    std::vector<ResultItem> resultsForSession() const;
    void setResultsForSession(const std::vector<ResultGroup> &results);

    std::vector<CompareResult> compareResults() const { return compareResults_; }
    void setCompareResults(
            const std::string &resultId,
            const std::vector<CompareResult> &compareResults);
    ResultItem selectedCompareResult() const { return selectedCompareResult_; }

    std::vector<CompareItem> duelResults() const { return duelResults_; }
    void setDuelResults(const std::vector<CompareResult> &duelResults);
    
    TestItem& findTest(const std::string& testId);
    
    std::string getFeaturedTestName() const { return featuredTestName_; }
private:
    void createTestItems();
    std::vector<ResultItem> createResultItems(const std::vector<ResultGroup> &results) const;
    void updateGroupSelections();
    void updateAvailability(TestItem &testItem, const Configuration &configuration);
    bool apiMatches(
            const std::vector<ApiDefinition> &testVersions,
            const std::vector<ApiDefinition> &deviceVersions) const;

    std::vector<Configuration> configurations_;
    std::vector<TestInfo> testInfos_;
    std::vector<TestItem> testItems_;
    std::vector<Session> sessions_;
    std::vector<ResultItem> bestResults_;
    std::vector<ResultItem> resultsForSession_;
    std::vector<CompareResult> compareResults_;
    std::vector<CompareItem> duelResults_;
    int selectedConfigurationIndex_;
    int selectedSessionIndex_;
    std::string featuredTestName_;
    ResultItem selectedCompareResult_;
    std::vector<std::string> hiddenTests_;
};

}

#endif
