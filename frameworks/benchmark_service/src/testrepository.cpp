/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testrepository.h"

#include "datagateway.h"

#include "ng/log.h"

#include <algorithm>



using namespace tfw;



TestRepository::TestRepository(
        DataGateway &testDataGateway,
        const ApplicationConfig& applicationConfig)
:
    applicationConfig_(&applicationConfig),
    selectedConfigurationIndex_(-1),
    dataGateway_(&testDataGateway)
{}



void TestRepository::loadTestsFromJsonString(const std::string &json, unsigned int renderApiFlags, unsigned int computeApiFlags)
{
    ng::JsonValue jvalue;
    jvalue.fromString(json.c_str(), ng::throws());
    ng::JsonValue jarray = jvalue["tests"];
    if (!jarray.isArray()) {
        throw std::runtime_error("Type of json object is incorrect: 'TestList[].tests");
    }


    allTestInfos_.clear();
    std::vector<TestInfo> testInfos;
    for (size_t i = 0; i < jarray.size(); ++i) {
		
		ng::JsonValue val = jarray[i]["minimum_api"][0]["type"];
		tfw::ApiDefinition::Type rapi = tfw::ApiDefinition::typeFromString (val.string());

        allTestInfos_.push_back(TestInfo::fromJsonValue(jarray[i]));
		if ( (computeApiFlags & tfw::ApiDefinition::NOT_DEFINED ) != 0)
		{
			if (((renderApiFlags & tfw::ApiDefinition::NOT_DEFINED) != 0 )
				|| ((renderApiFlags & rapi) == rapi))
			{
				testInfos.push_back(TestInfo::fromJsonValue(jarray[i]));
			}
		}
		else
		{
			ng::JsonValue comp = jarray[i]["minimum_compute_api"][0]["type"];
			tfw::ApiDefinition::Type capi = tfw::ApiDefinition::typeFromString(comp.string());
            
            bool renderCmp = true;
#if defined(COMPUBENCH_CL) || defined(COMPUBENCH_CU) || defined(COMPUBENCH_METAL)
            renderCmp = (( renderApiFlags & rapi ) == rapi );
#endif
			if (renderCmp  &&  (( computeApiFlags & capi) == capi) ) {
				testInfos.push_back(TestInfo::fromJsonValue(jarray[i]));
			}
		}
    }
    testInfos_ = testInfos;
    createTestItems();

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




bool TestRepository::needCudaConfigurations() const
{
    for (std::vector<TestInfo>::const_iterator i = testInfos_.begin(); i != testInfos_.end(); ++i) {
        const TestInfo &testInfo = *i;
        const std::vector<ApiDefinition>& minCompute = testInfo.minimumComputeApi();
        for (std::vector<ApiDefinition>::const_iterator j = minCompute.begin(); j != minCompute.end(); ++j) {
            const ApiDefinition& computeApi = *j;
            if (computeApi.type() == ApiDefinition::CUDA) {
                return true;
            }
        }
    }
    return false;
}

bool TestRepository::needMetalConfigurations() const
{
    for (std::vector<TestInfo>::const_iterator i = testInfos_.begin(); i != testInfos_.end(); ++i) {
        const TestInfo &testInfo = *i;
        const std::vector<ApiDefinition>& minCompute = testInfo.minimumComputeApi();
        for (std::vector<ApiDefinition>::const_iterator j = minCompute.begin(); j != minCompute.end(); ++j) {
            const ApiDefinition& computeApi = *j;
            if (computeApi.type() == ApiDefinition::METAL) {
                return true;
            }
        }
		
		const std::vector<ApiDefinition>& minGraphics = testInfo.minimumGraphicsApi();
		for (std::vector<ApiDefinition>::const_iterator j = minGraphics.begin(); j != minGraphics.end(); ++j) {
			const ApiDefinition& graphicsApi = *j;
			if (graphicsApi.type() == ApiDefinition::METAL) {
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
	bool needCL = needClConfigurations();
	bool configIsCL = (configuration.apiFlags() & tfw::ApiDefinition::CL) != 0;

	if(needCL == false && configIsCL == true)
		return;

	bool needCUDA = needCudaConfigurations();
	bool configIsCUDA = (configuration.apiFlags() & tfw::ApiDefinition::CUDA) != 0;

	if(needCUDA == false && configIsCUDA == true)
		return;
    
    bool needMetal = needMetalConfigurations();
    bool configIsMetal = (configuration.apiFlags() & tfw::ApiDefinition::METAL) != 0;
    
    if(needMetal == false && configIsMetal == true)
        return;
    
	//bool configIsGraphics =	!configIsCL && !configIsCUDA && !configIsMetal;


	//if((needCL || needCUDA || needMetal) && configIsGraphics == true)
	//	return;

    configurations_.push_back(configuration);
    configurations_.back().setRowId(configurations_.size() - 1);
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

    for (size_t i = 0; i < configurations_.size(); ++i) {
        configurations_.at(i).setChecked(static_cast<int>(i) == configurationIndex);
    }
    
    createTestItems();

    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        selectedConfigurationIndex_ = configurationIndex;
        if (selectedConfigurationIndex_ >= 0) {
            updateAvailability(testItem, selectedConfiguration());
        } else {
            NGLOG_INFO("No configuration selected --> disable all tests!!!");
            testItem.pushIncompatibilityReason("ApiNotSupported");
        }
    }
	
	selectedConfigurationIndex_ = configurationIndex; // needs to be set again, because updateAvailability() might change the value
	
    updateGroupSelections();
}



void TestRepository::selectConfiguration(const std::string &configurationId)
{
    NGLOG_INFO("Selecting configuration: %s", configurationId);
    int index = -1;
    for (size_t i = 0; i < configurations_.size(); ++i) {
        if (configurations_.at(i).name() == configurationId) {
            index = static_cast<int>(i);
            selectConfiguration(index);
        }
    }
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
    Transaction transaction(dataGateway_->database());

    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (testItem.requires("runalone")) {
            testItem.setSelected(false);
            dataGateway_->addTestSelection(testItem.testId(), false);
        }
    }

    TestItem &testItem = findTest(testId);
    if (testItem.requires("runalone")) {
        if (isSelected) {
            for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
                TestItem &testItem = *i;
                testItem.setSelected(false);
                dataGateway_->addTestSelection(testItem.testId(), false);
            }
        }
    }
    testItem.setSelected(isSelected);
    dataGateway_->addTestSelection(testItem.testId(), isSelected);
    updateGroupSelections();

    transaction.commit();
}



void TestRepository::toggleAllTestSelection()
{
    Transaction transaction(dataGateway_->database());

    bool haveUnselected = false;
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (!testItem.isSelected() && !testItem.requires("runalone")) {
            haveUnselected = true;
            break;
        }
    }
    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        bool isRunAlone = testItem.requires("runalone");
        testItem.setSelected(haveUnselected && !isRunAlone);
        dataGateway_->addTestSelection(testItem.testId(), haveUnselected && !isRunAlone);
    }
    updateGroupSelections();

    transaction.commit();
}



void TestRepository::setGroupSelection(const std::string &groupId, bool isSelected)
{
    Transaction transaction(dataGateway_->database());

    for (std::vector<TestItem>::iterator i = testItems_.begin(); i != testItems_.end(); ++i) {
        TestItem &testItem = *i;
        if (testItem.groupId() == groupId) {
            testItem.setSelected(isSelected);
            dataGateway_->addTestSelection(testItem.testId(), isSelected);
            if (isSelected && testItem.requires("runalone")) {
                setTestSelection(testItem.testId(), true);
            }
        }
    }
    updateGroupSelections();

    transaction.commit();
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



std::vector<Session> TestRepository::sessions() const
{
    Session bestSession;
    bestSession.setSessionId(BEST_RESULTS_SESSION_ID);
    bestSession.setFinished(true);

    std::vector<Session> buffer = dataGateway_->getSessions();
    buffer.insert(buffer.begin(), bestSession);
    return buffer;
}



int TestRepository::sessionIndex(long long sessionId) const
{
    std::vector<Session> buffer = sessions();
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (buffer.at(i).sessionId() == sessionId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}



std::vector<ResultItem> TestRepository::bestResults() const
{
	if (selectedConfigurationIndex() >= 0)
	{
		return createResultItems(dataGateway_->getBestResults(selectedConfiguration().name()), true, false);
	}
	else
	{
		return createResultItems(dataGateway_->getBestResults(), true, false);
	}
}



std::vector<CompareResult> TestRepository::compareResults(
        const std::string &resultId,
        const std::string &filter,
        bool hideDesktop) const
{
	ResultItem ourBestResult = findBestResult(resultId);
    std::vector<CompareResult> theirResults = dataGateway_->getResultsById(ourBestResult, filter, hideDesktop);

    for (auto i = theirResults.begin(); i != theirResults.end(); ++i) {
        CompareResult& theirResult = *i;
        theirResult.setPrimaryUnit(ourBestResult.unit());
        theirResult.setSecondaryUnit(ourBestResult.secondaryUnit());
		if (ourBestResult.score() > theirResult.maxScore()) {
			theirResult.setMaxScore(ourBestResult.score());
		}

        /* XXX: image path path hack */
        std::string image = theirResult.deviceImage();
        size_t lastSlash = image.find_last_of('/');
        if (lastSlash == std::string::npos) {
            lastSlash = 0;
        }
        image = applicationConfig_->synchronizationPath + "/image/device" +
                image.substr(lastSlash);
        theirResult.setDeviceImage(image);
    }
    return theirResults;
}



std::vector<DuelItem> TestRepository::duelResults(
        const std::string& apiA,
        const std::string& deviceA,
        const std::string& apiB,
        const std::string& deviceB) const
{
    std::vector<ResultItem> best = bestResults();
    std::vector<CompareResult> resultsA = dataGateway_->getResultsByDevice(apiA, deviceA);
    std::vector<CompareResult> resultsB = dataGateway_->getResultsByDevice(apiB, deviceB);
    std::vector<DuelItem> results;
    for (std::vector<ResultItem>::iterator i = best.begin(); i != best.end(); ++i) {
        ResultItem &resultItem = *i;
        std::string resultItemCompareId = resultItem.baseId() + resultItem.variantName();
        double scoreA = -1.0;
        double scoreB = -1.0;
        auto predicate = [=](const CompareResult& result) {
            return result.compareId() == resultItemCompareId;
        };
        if (deviceA != "own") {
            auto found = std::find_if(resultsA.begin(), resultsA.end(), predicate);
            if (found != resultsA.end()) {
                scoreA = found->score();
            }
        } else {
            if (resultItem.status() == BenchmarkService::OK) {
                scoreA = resultItem.score();
            }
        }
        if (deviceB != "own") {
            auto found = std::find_if(resultsB.begin(), resultsB.end(), predicate);
            if (found != resultsB.end()) {
                scoreB = found->score();
            }
        } else {
            if (resultItem.status() == BenchmarkService::OK) {
                scoreB = resultItem.score();
            }
        }
        results.push_back(DuelItem(resultItem, scoreA, scoreB));
    }
    return results;
}



ResultItem TestRepository::findBestResult(const std::string &resultId) const
{
    std::vector<ResultItem> best = bestResults();
    for (std::vector<ResultItem>::iterator i = best.begin(); i != best.end(); ++i) {
        ResultItem &resultItem = *i;
        if (resultItem.resultId() == resultId) {
            return resultItem;
        }
    }
    throw std::runtime_error("Best result \"" + resultId + "\" not found.");
}



std::vector<ResultItem> TestRepository::resultsForSession(long long sessionId) const
{
    if (sessionId == BEST_RESULTS_SESSION_ID) {
        return bestResults();
    } else {
        return createResultItems(dataGateway_->getResultsForSession(sessionId), false, true);
    }
}



void TestRepository::createTestItems()
{
    testItems_.clear();

    bool configSupportsTessellation = false;
    bool configHas2GbMemory = false;
    if (selectedConfigurationIndex_ < 0) {
        NGLOG_INFO("No configuration selected --> disable all tests!!!");
    } else {
        if (selectedConfiguration().hasFeature("tessellation")) {
            configSupportsTessellation = true;
        }
        if (selectedConfiguration().hasFeature("memory2gb")) {
            configHas2GbMemory = true;
        }
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
                if ((!configSupportsTessellation && testItem.requires("tessellation"))
                    || (!configHas2GbMemory && testItem.requires("memory2gb")))
                {
                    continue;
                }
                
                updateAvailability(testItem, selectedConfiguration());
            } else {
                testItem.pushIncompatibilityReason("ApiNotSupported");
            }
            std::vector<std::string>::iterator found =
                    std::find(hiddenTests_.begin(), hiddenTests_.end(), testItem.testId());
            testItem.setVisible(found == hiddenTests_.end());
            testItem.setImagePath(applicationConfig_->imagePath);
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
        const std::vector<ResultGroup> &results,
        bool addNa,
        bool useAllTest) const
{
    std::vector<ResultItem> items;
    std::vector<TestInfo> testInfoList = testInfos_;
    if (useAllTest)
    {
        testInfoList = allTestInfos_;
    }

    for (std::vector<TestInfo>::const_iterator i = testInfoList.begin(); i != testInfoList.end(); ++i) {
        const TestInfo &testInfo = *i;
        for (size_t j = 0; j < testInfo.resultTypes().size(); ++j) {
            for (size_t k = 0; k < testInfo.variantPostfixes().size(); ++k) {
                ResultItem resultItem;
                resultItem.setTestInfo(testInfo);
                resultItem.setResultTypeIndex(static_cast<int>(j));
                resultItem.setVariantIndex(static_cast<int>(k));
                resultItem.setFirstInGroup(false);
                std::string resultId = resultItem.resultId();
                std::vector<ResultGroup>::const_iterator it = std::find_if(results.begin(), results.end(), ResultIdMatch(resultItem.testId()));

                if (it == results.end()) {
                    it = std::find_if(results.begin(), results.end(), TestIdMatch(resultItem.testId()));
                }

                if (it != results.end()) {
                    // found Result with matching result_id
					resultItem.setResultGroup(*it);
					resultItem.setDeviceName(it->configuration());
                } else {
					resultItem.setDeviceName("Your Device");
				}

                if ((it != results.end()) || addNa) {
                    resultItem.setImagePath(applicationConfig_->imagePath);
                    items.push_back(resultItem);
                }
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
        selectedConfigurationIndex_ = -1;
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
        selectedConfigurationIndex_ = -1;
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
