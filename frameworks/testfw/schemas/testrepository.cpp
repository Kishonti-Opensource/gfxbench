/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testrepository.h"

#include <algorithm>
#include <ng/log.h>

using namespace tfw;



template<class T>
std::vector<std::vector<T> > layout(const std::vector<T> &items, bool isMultiColumn)
{
    std::vector<std::vector<T> > layout;
    std::string lastTestName;
    std::string lastGroupId;
    std::vector<T> row;
    for (typename std::vector<T>::const_iterator i = items.begin(); i != items.end(); ++i) {
        T item = *i;
        if (!item.isVisible()) {
            continue;
        }
        int variantCount = (int)item.testInfo().variantPostfixes().size();
        lastTestName = item.testInfo().testName();
        item.setFirstInGroup(lastGroupId != item.groupId());
        lastGroupId = item.groupId();
        if (isMultiColumn) {
            row.resize(variantCount);
            row[item.variantIndex()] = item;
        } else {
            row.clear();
            row.push_back(item);
        }
        if (!isMultiColumn || (item.variantIndex() == variantCount - 1))
        {
            layout.push_back(row);
            row.clear();
        }
    }
    return layout;
}

template<class T>
std::vector<T> simpleLayout(const std::vector<T> &items, const bool isHeadered = true)
{
    std::vector<T> layout;
    std::string lastGroupId;
    for (typename std::vector<T>::const_iterator i = items.begin(); i != items.end(); ++i) {
        T item = *i;
        if (!item.isVisible()) {
            continue;
        }

        if(lastGroupId != item.groupId() && isHeadered) {
            item.setFirstInGroup(true);
            layout.push_back(item);
        }
        lastGroupId = item.groupId();

        item.setFirstInGroup(false);
        layout.push_back(item);
    }
    return layout;
}



std::vector<std::vector<TestItem> > TestRepository::singleColumnLayout(
        const std::vector<TestItem> &items)
{
    return ::layout(items, false);
}



std::vector<std::vector<TestItem> > TestRepository::multiColumnLayout(
        const std::vector<TestItem> &items)
{
    return ::layout(items, true);
}



std::vector<TestItem> TestRepository::headerRepeatedColumnLayout(
        const std::vector<TestItem> &items)
{
    return ::simpleLayout(items);
}



std::vector<std::vector<ResultItem> > TestRepository::singleColumnLayout(
        const std::vector<ResultItem> &items)
{
    return ::layout(items, false);
}



std::vector<std::vector<ResultItem> > TestRepository::multiColumnLayout(
        const std::vector<ResultItem> &items)
{
    return ::layout(items, true);
}



std::vector<ResultItem> TestRepository::headerRepeatedColumnLayout(
        const std::vector<ResultItem> &items)
{
    return ::simpleLayout(items);
}

std::vector<ResultItem> TestRepository::simpleColumnLayout(
        const std::vector<ResultItem> &items)
{
    return ::simpleLayout(items, false);
}



std::vector<std::vector<ResultItem> > TestRepository::filterNotAvailableRows(
       const std::vector<std::vector<ResultItem> > &items)
{
    std::vector<std::vector<ResultItem> > result;
    bool isFirstInGroup = true;
    for (std::vector<std::vector<ResultItem> >::const_iterator i = items.begin(); i != items.end(); ++i) {
        const std::vector<ResultItem> &row = *i;
        bool hasContent = false;
        for (std::vector<ResultItem>::const_iterator j = row.begin(); j != row.end(); ++j) {
            const ResultItem &resultItem = *j;
            isFirstInGroup = isFirstInGroup || resultItem.isFirstInGroup();
            if (resultItem.errorString() != "Results_NA") {
                hasContent = true;
            }
        }
        if (hasContent) {
            result.push_back(row);
            for (std::vector<ResultItem>::iterator j = result.back().begin(); j != result.back().end(); ++j) {
                ResultItem &resultItem = *j;
                resultItem.setFirstInGroup(isFirstInGroup);
            }
            isFirstInGroup = false;
        }
    }
    return result;
}


std::vector<ResultItem> TestRepository::filterNotAvailableRows(
        const std::vector<ResultItem> &items)
{
    std::vector<ResultItem> result;
    bool isFirstInGroup = true;
    std::string lastGroupId = "";
    for (std::vector<ResultItem>::const_iterator i = items.begin(); i != items.end(); ++i) {
        ResultItem resultItem = *i;
        if(!resultItem.isFirstInGroup() && resultItem.errorString() != "Results_NA") {
            if(lastGroupId != resultItem.groupId()) {
                isFirstInGroup = true;
                lastGroupId = resultItem.groupId();
            }
            if(isFirstInGroup) {
                resultItem.setFirstInGroup(true);
                result.push_back(resultItem);
                isFirstInGroup = false;
            }
            resultItem.setFirstInGroup(false);
            result.push_back(resultItem);
        }
    }
    return result;
}



TestRepository::TestRepository():
    selectedConfigurationIndex_(-1),
    selectedSessionIndex_(-1)
{}



void TestRepository::loadTestsFromJsonString(const std::string &json)
{
    ng::JsonValue jvalue;
    jvalue.fromString(json.c_str(), ng::throws());
    ng::JsonValue jarray = jvalue["tests"];
    if (!jarray.isArray()) {
        throw std::runtime_error("Type of json object is incorrect: 'TestList[].tests");
    }
    std::vector<TestInfo> testInfos;
    for (size_t i = 0; i < jarray.size(); ++i) {
        testInfos.push_back(TestInfo::fromJsonValue(jarray[i]));
    }
    testInfos_ = testInfos;
    createTestItems();
    bestResults_ = createResultItems(std::vector<tfw::ResultGroup>());
    resultsForSession_ = bestResults_;

    ng::JsonValue featured = jvalue["featured"];
    if(featured.isString()) {
        featuredTestName_ = featured.string();
    } else {
        if (!testItems_.empty())
            featuredTestName_ = testItems_.front().testId();
        else
            featuredTestName_ = "";
    }
}



void TestRepository::addTestIncompReason(const std::string &testId, const std::string &reason) {
    TestItem &item = findTest(testId);
    item.pushIncompatibilityReason(reason);
    updateGroupSelections();
}



void TestRepository::removeTestIncompReason(const std::string &testId, const std::string &reason) {
    TestItem &item = findTest(testId);
    item.removeIncompatibilityReason(reason);
    updateGroupSelections();
}



void TestRepository::setTestVisibility(const std::string &testId, bool isVisible)
{
    TestItem &item = findTest(testId);
    if(item.isVisible() != isVisible) {
        item.setVisible(isVisible);
        item.setSelected(false);
    }
    updateGroupSelections();
}



std::vector<unsigned> TestRepository::allowedConfigurations() const
{
    std::set<unsigned> result;
    for (std::vector<TestInfo>::const_iterator i = testInfos_.begin(); i != testInfos_.end(); ++i) {
        const TestInfo &testInfo = *i;
        std::vector<ApiDefinition> minGraphics = testInfo.minimumGraphicsApi();
        for (std::vector<ApiDefinition>::const_iterator j = minGraphics.begin(); j != minGraphics.end(); ++j) {
            const ApiDefinition& graphicsApi = *j;
            std::vector<ApiDefinition> minCompute = testInfo.minimumComputeApi();
            for (std::vector<ApiDefinition>::const_iterator k = minCompute.begin(); k != minCompute.end(); ++k) {
                const ApiDefinition& computeApi = *k;
                unsigned apiFlags = graphicsApi.type() | computeApi.type();
                result.insert(apiFlags);
            }
            if (testInfo.minimumComputeApi().empty()) {
                result.insert(graphicsApi.type());
            }
        }
    }
    std::vector<unsigned> output(result.begin(), result.end());
    return output;
}



bool TestRepository::needClConfigurations() const
{
    for (std::vector<TestInfo>::const_iterator i = testInfos_.begin(); i != testInfos_.end(); ++i) {
        const TestInfo &testInfo = *i;
        const std::vector<ApiDefinition>& minCompute = testInfo.minimumComputeApi();
        for (std::vector<ApiDefinition>::const_iterator j = minCompute.begin(); j != minCompute.end(); ++j) {
            const ApiDefinition& computeApi = *j;
            if (computeApi.type() == ApiDefinition::CL) {
                return true;
            }
        }
    }
    return false;
}



Configuration TestRepository::findConfiguration(const std::string& configurationId) const
{
    for (std::vector<Configuration>::const_iterator i = configurations_.begin(); i != configurations_.end(); ++i) {
        const Configuration &configuration = *i;
        if (configuration.name() == configurationId) {
            return configuration;
        }
    }
    throw std::runtime_error("Configuration \"" + configurationId + "\" not found.");
}



void TestRepository::addConfiguration(const Configuration &configuration)
{
    NGLOG_INFO("Trying to add configuration: name     - %s", configuration.name());
    NGLOG_INFO("                             flags    - %s", configuration.apiFlags());
    NGLOG_INFO("Comparing to configuration flags...");
    std::vector<unsigned> allowed = allowedConfigurations();
    std::vector<unsigned>::const_iterator found = allowed.end();
    for (std::vector<unsigned>::const_iterator i = allowed.begin(); i != allowed.end(); ++i) {
        NGLOG_INFO(" - Flag: %s", *i);
        if( ((*i) & configuration.apiFlags()) != 0)
            found = i;
    }
    
    if (found == allowed.end()) {
        NGLOG_INFO("No match found!");
        return;
    }
    NGLOG_INFO("Match found!");

    configurations_.push_back(configuration);
    if ((selectedConfigurationIndex_ < 0) && configuration.isEnabled()) {
        selectConfiguration(configuration.name());
    }
}



Configuration TestRepository::selectedConfiguration() const
{
    if (selectedConfigurationIndex_ < 0) {
        throw std::runtime_error("No configuration selected.");
    }
    return configurations_.at(selectedConfigurationIndex_);
}



void TestRepository::selectConfiguration(int configurationIndex)
{
    NGLOG_INFO("Selecting configuration: %s", configurationIndex);
    selectedConfigurationIndex_ = configurationIndex;

    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (selectedConfigurationIndex_ >= 0) {
            updateAvailability(testItem, selectedConfiguration());
        } else {
            NGLOG_INFO("No configuration selected --> disable all tests!!!");
            testItem.pushIncompatibilityReason("ApiNotSupported");
        }
    }
    updateGroupSelections();
}



void TestRepository::selectConfiguration(const std::string &configurationId)
{
    NGLOG_INFO("Selecting configuration: %s", configurationId);
    int index = -1;
    for (size_t i = 0; i < configurations_.size(); ++i) {
        if (configurations_.at(i).name() == configurationId) {
            index = static_cast<int>(i);
            break;
        }
    }
    selectConfiguration(index);
}



TestItem TestRepository::findTest(const std::string& testId) const
{
    return const_cast<TestRepository*>(this)->findTest(testId);
}



TestItem& TestRepository::findTest(const std::string& testId)
{
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (testItem.testId() == testId) {
            return testItem;
        }
    }
    throw std::runtime_error("Test \"" + testId + "\" not found.");
}



void TestRepository::setTestSelection(const std::string &testId, bool isSelected)
{
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (testItem.requires("runalone")) {
            testItem.setSelected(false);
        }
    }

    TestItem &testItem = findTest(testId);
    if (testItem.requires("runalone")) {
        if (isSelected) {
            for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
                TestItem &testItem = *i;
                testItem.setSelected(false);
            }
        }
    }
    testItem.setSelected(isSelected);
    updateGroupSelections();
}



void TestRepository::setGroupSelection(const std::string &groupId, bool isSelected)
{
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (testItem.requires("runalone")) {
            testItem.setSelected(false);
        }
    }
    
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (testItem.groupId() == groupId) {
            testItem.setSelected(isSelected);
            testItem.setGroupSelected(isSelected);
            if (isSelected && testItem.requires("runalone")) {
                setTestSelection(testItem.testId(), true);
            }
        }
    }
    updateGroupSelections();
}



bool lessByRunorder(const TestItem &lhs, const TestItem &rhs)
{
    return lhs.testInfo().runOrder() < rhs.testInfo().runOrder();
}

std::vector<TestItem> TestRepository::allAvailableTests() const
{
    std::vector<TestItem> availableTests;
    for (std::vector<TestItem>::const_iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        const TestItem &testItem = *i;
        if (testItem.isAvailable() && !testItem.requires("runalone")) {
            availableTests.push_back(testItem);
        }
    }
    std::stable_sort(availableTests.begin(), availableTests.end(), lessByRunorder);
    return availableTests;
}

std::vector<TestItem> TestRepository::selectedAvailableTests() const
{
    std::vector<TestItem> availableTests;
    for (std::vector<TestItem>::const_iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        const TestItem &testItem = *i;
        if (testItem.isAvailable() && testItem.isSelected()) {
            availableTests.push_back(testItem);
        }
    }
    std::stable_sort(availableTests.begin(), availableTests.end(), lessByRunorder);
    return availableTests;
}



void TestRepository::addHiddenTest(const std::string &testId)
{
    hiddenTests_.push_back(testId);
}



Session TestRepository::selectedSession() const
{
    return (selectedSessionIndex_ >= 0) ? sessions_.at(selectedSessionIndex_) : Session();
}



void TestRepository::selectSession(long long sessionId)
{
    selectedSessionIndex_ = -1;
    for (size_t i = 0; i < sessions_.size(); ++i) {
        if (sessions_.at(i).sessionId() == sessionId) {
            selectedSessionIndex_ = static_cast<int>(i);
            break;
        }
    }
}



void TestRepository::setSessions(const std::vector<Session> &sessions)
{
    sessions_ = sessions;
}



void TestRepository::setBestResults(const std::vector<ResultGroup> &results)
{
    std::vector<ResultGroup> updatedResults = results;
    for (size_t i = 0; i < updatedResults.size(); ++i) {
        for (size_t j = 0; j < updatedResults.at(i).results().size(); ++j) {
            tfw::Result &result = updatedResults.at(i).results().at(j);
            if (result.status() != tfw::Result::OK) {
                result.setStatus(tfw::Result::FAILED);
                result.setErrorString("Results_NA");
            }
        }
    }
    bestResults_ = createResultItems(updatedResults);
}



tfw::ResultItem TestRepository::findBestResult(const std::string &resultId)
{
    for (std::vector<ResultItem>::iterator i = bestResults_.begin(); i != bestResults_.end(); ++i) {
        ResultItem &resultItem = *i;
        if (resultItem.resultId() == resultId) {
            return resultItem;
        }
    }
    throw std::runtime_error("Best result \"" + resultId + "\" not found.");
}



std::vector<ResultItem> TestRepository::resultsForSession() const
{
    return (selectedSessionIndex_ < 0) ? bestResults_ : resultsForSession_;
}



void TestRepository::setResultsForSession(const std::vector<ResultGroup> &results)
{
    resultsForSession_ = createResultItems(results);
}



void TestRepository::setCompareResults(
        const std::string &resultId,
        const std::vector<CompareResult> &compareResults)
{
    compareResults_ = compareResults;
    selectedCompareResult_ = findBestResult(resultId);
}



void TestRepository::setDuelResults(const std::vector<CompareResult> &duelResults)
{
    duelResults_.clear();
    for (std::vector<ResultItem>::iterator i = bestResults_.begin(); i != bestResults_.end(); ++i) {
        ResultItem &resultItem = *i;
        if (resultItem.unit() == "chart") {
            continue;
        }
        CompareItem compareItem(resultItem);
        for (std::vector<CompareResult>::const_iterator j = duelResults.begin(); j != duelResults.end(); ++j) {
            const CompareResult &compareResult = *j;
            if (compareResult.compareId() == resultItem.baseResultId()) {
                compareItem.setCompareResult(compareResult);
            }
        }
        duelResults_.push_back(std::move(compareItem));
    }
}



void TestRepository::createTestItems()
{
    testItems_.clear();

    if (selectedConfigurationIndex_ < 0) {
        NGLOG_INFO("No configuration selected --> disable all tests!!!");
    }

    std::vector<TestItem> items;
    for (std::vector<TestInfo>::const_iterator i = testInfos_.begin(); i != testInfos_.end(); ++i) {
        const TestInfo &testInfo = *i;
        for (size_t j = 0; j < testInfo.variantPostfixes().size(); ++j) {
            TestItem testItem;
            testItem.setTestInfo(testInfo);
            testItem.setVariantIndex(static_cast<int>(j));
            testItem.setSelected(!testItem.requires("runalone"));
            testItem.setGroupSelected(true);
            if (selectedConfigurationIndex_ >= 0) {
                updateAvailability(testItem, selectedConfiguration());
            } else {
                testItem.pushIncompatibilityReason("ApiNotSupported");
            }
            std::vector<std::string>::iterator found =
                    std::find(hiddenTests_.begin(), hiddenTests_.end(), testItem.testId());
            testItem.setVisible(found == hiddenTests_.end());
            items.push_back(testItem);
        }
    }

    testItems_ = items;
    updateGroupSelections();
}



struct ResultIdMatch {
    ResultIdMatch(const std::string &id) : id_(id) {}
    bool operator()(const ResultGroup &r) { return r.results().front().resultId() == id_; }
    std::string id_;
};

struct TestIdMatch {
    TestIdMatch(const std::string &id) : id_(id) {}
    bool operator()(const ResultGroup &r) { return r.results().front().testId() == id_; }
    std::string id_;
};

std::vector<ResultItem> TestRepository::createResultItems(
        const std::vector<ResultGroup> &results) const
{
    std::vector<ResultItem> items;
    for (std::vector<TestInfo>::const_iterator i = testInfos_.begin(); i != testInfos_.end(); ++i) {
        const TestInfo &testInfo = *i;
        for (size_t j = 0; j < testInfo.resultTypes().size(); ++j) {
            for (size_t k = 0; k < testInfo.variantPostfixes().size(); ++k) {
                ResultItem resultItem;
                resultItem.setTestInfo(testInfo);
                resultItem.setResultTypeIndex(static_cast<int>(j));
                resultItem.setVariantIndex(static_cast<int>(k));
                resultItem.setFirstInGroup(false);
                std::string resultId = resultItem.resultId();
                std::vector<ResultGroup>::const_iterator it = std::find_if(
                        results.begin(), results.end(), ResultIdMatch(resultItem.testId()));
                if (it == results.end()) {
                    it = std::find_if(results.begin(), results.end(),
                           TestIdMatch(resultItem.testId()));
                }
                if (it != results.end()) {
                    // found Result with matching result_id
                    resultItem.setResultGroup(*it);
                }
                items.push_back(resultItem);
            }
        }
    }
    return items;
}



void TestRepository::updateGroupSelections()
{
    std::set<std::string> nonEmptyGroups;
    std::set<std::string> deselectedGroups;
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (!testItem.isAvailable()) continue;
        if (!testItem.isSelected()) {
            deselectedGroups.insert(testItem.groupId());
        }
        nonEmptyGroups.insert(testItem.groupId());
        testItem.setGroupSelected(true);
    }
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        bool isGroupSelectionEnabled =
                (nonEmptyGroups.find(testItem.groupId()) != nonEmptyGroups.end());
        bool isGroupSelected =
                (deselectedGroups.find(testItem.groupId()) == deselectedGroups.end());
        testItem.setGroupSelectionEnabled(isGroupSelectionEnabled);
        testItem.setGroupSelected(isGroupSelectionEnabled && isGroupSelected);
    }
}



void TestRepository::updateAvailability(TestItem &testItem, const Configuration &configuration)
{
    NGLOG_INFO("Updating test %s availability for config %s", testItem.testId(), configuration.name());
    if (!apiMatches(testItem.testInfo().minimumGraphicsApi(), configuration.ApiDefinitions()) ||
        !apiMatches(testItem.testInfo().minimumComputeApi(), configuration.ApiDefinitions()))
    {
        NGLOG_INFO("Not compatible (different api level)!!!");
        testItem.pushIncompatibilityReason("ApiNotSupported");
        return;
    }

    std::vector<std::string> features = configuration.features();
    features.push_back("runalone");
    std::sort(features.begin(), features.end());

    std::vector<std::string> difference(testItem.testInfo().requirements().size());
    std::vector<std::string> requirements = testItem.testInfo().requirements();
    std::vector<std::string>::iterator it = std::set_difference(
            requirements.begin(), requirements.end(),
            features.begin(), features.end(),
            difference.begin());
    if (it - difference.begin() > 0) {
        NGLOG_INFO("Not compatible (missing feature)!!!");
        std::string missing = difference.front();
        testItem.pushIncompatibilityReason("Missing_" + missing);
        return;

    } else {
        for (std::vector<std::string>::const_iterator i = features.begin(); i != features.end(); ++i) {
            testItem.removeIncompatibilityReason("Missing_" + *i);    
        }
    }

    NGLOG_INFO("Compatible");
    testItem.removeIncompatibilityReason("ApiNotSupported");
}



bool TestRepository::apiMatches(
        const std::vector<ApiDefinition> &testVersions,
        const std::vector<ApiDefinition> &deviceVersions) const
{
    for (std::vector<ApiDefinition>::const_iterator i = testVersions.begin(); i != testVersions.end(); ++i) {
        const ApiDefinition &required = *i;
        for (std::vector<ApiDefinition>::const_iterator j = deviceVersions.begin(); j != deviceVersions.end(); ++j) {
            const ApiDefinition &device = *j;
            if (device.isCompatibleWith(required)) return true;
        }
    }
    return testVersions.empty();
}
