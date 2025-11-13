/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "backgroundtasks.h"

#include "applicationconfig.h"
#include "benchmarkexception.h"
#include "datagateway.h"
#include "networkgateway.h"
#include "systeminfogateway.h"
#include "testrepository.h"
#include "testrunner.h"

#include "deviceinfo/runtimeinfo.h"

#include "properties.h"
#include "SystemInfoCommonKeys.h"

#include "ng/log.h"

#include <Poco/AtomicCounter.h>
#include <Poco/File.h>
#include <Poco/Timer.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif


class BackgroundTasks::Private
{
public:
    BackgroundTaskCallback* callback;
    const ApplicationConfig* applicationConfig;
    SystemInfoGateway* systemInfoGateway;
    NetworkGateway* networkGateway;
    DataGateway* dataGateway;
    TestRunner* testRunner;
    tfw::RuntimeInfo* runtimeInfo;
    std::unique_ptr<TestRepository> testRepository;

    std::vector<SystemInfoItem> systemInfo;
    Session session;
    std::vector<TestItem> scheduledTests;
    tfw::Descriptor currentDescriptor;
    Poco::AtomicCounter interrupted;

    int customWidth, customHeight;
    double brightness;
    bool endless;
	bool initialized;

    ListCursorContainer<TestItem> testCursors;
    ListCursorContainer<Configuration> configurationCursors;
    ListCursorContainer<Session> sessionCursors;
    ListCursorContainer<ResultItem> resultCursors;
    ListCursorContainer<CompareResult> compareCursors;
    ListCursorContainer<DuelItem> duelCursors;
    ListCursorContainer<SystemInfoItem> systemInfoCursors;
    ListCursorContainer<SystemInfoAttribute> attributeCursors;

    bool checkBattery(const std::vector<TestItem>& scheduledTests);
    void pollSynchronizationProgress(Poco::Timer&);
    void updateBattery(Poco::Timer&);
	void init()
	{
		configurationCursors	= ListCursorContainer<Configuration>();
		sessionCursors			= ListCursorContainer<Session>();
		resultCursors			= ListCursorContainer<ResultItem>();
		compareCursors			= ListCursorContainer<CompareResult>();
		duelCursors				= ListCursorContainer<DuelItem>();
		systemInfoCursors		= ListCursorContainer<SystemInfoItem>();
		attributeCursors		= ListCursorContainer<SystemInfoAttribute>();
		testCursors				= ListCursorContainer<TestItem>();

	}
};



BackgroundTasks::BackgroundTasks(
        const ApplicationConfig& applicationConfig,
        BackgroundTaskCallback& callback,
        DataGateway& dataGateway,
        NetworkGateway& networkGateway,
        tfw::RuntimeInfo& runtimeInfo,
        SystemInfoGateway& systemInfoGateway,
        TestRunner& testRunner)
:
    d(new Private)
{
	d->initialized = false;
    d->callback = &callback;
    d->applicationConfig = &applicationConfig;
    d->systemInfoGateway = &systemInfoGateway;
    d->dataGateway = &dataGateway;
    d->networkGateway = &networkGateway;
    d->testRunner = &testRunner;
    d->runtimeInfo = &runtimeInfo;
    d->testRepository.reset(new TestRepository(*d->dataGateway, *d->applicationConfig));

    d->customWidth = 0;
    d->customHeight = 0;
    d->brightness = -1.0;
    d->endless = false;
}



BackgroundTasks::~BackgroundTasks()
{}



void BackgroundTasks::stop()
{
    NGLOG_DEBUG("Stopping");
    d->testCursors.setBlockUpdate(true);
    d->configurationCursors.setBlockUpdate(true);
    d->sessionCursors.setBlockUpdate(true);
    d->resultCursors.setBlockUpdate(true);
    d->compareCursors.setBlockUpdate(true);
    d->duelCursors.setBlockUpdate(true);
    d->systemInfoCursors.setBlockUpdate(true);
    d->attributeCursors.setBlockUpdate(true);
}



void BackgroundTasks::interrupt()
{
    d->interrupted = 1;
    //d->networkGateway->cancel();
    d->testRunner->cancelTest();
}



void BackgroundTasks::queryTests(const std::shared_ptr<ListCursor<TestItem> >& cursor)
{
    cursor->setQuery([=]() -> std::vector<TestItem> {
        NGLOG_DEBUG("Getting tests");
        return d->testRepository->tests();
    });
    d->testCursors.registerCursor(cursor);
}



void BackgroundTasks::queryConfigurations(
        const std::shared_ptr<ListCursor<Configuration> >& cursor)
{
    cursor->setQuery([=]() -> std::vector<Configuration> {
        NGLOG_DEBUG("Getting configurations");
        return d->testRepository->configurations();
    });
    d->configurationCursors.registerCursor(cursor);
}



void BackgroundTasks::querySessions(const std::shared_ptr<ListCursor<Session> >& cursor)
{
    cursor->setQuery([=]() -> std::vector<Session> {
        NGLOG_DEBUG("Getting sessions");
        return d->testRepository->sessions();
    });
    d->sessionCursors.registerCursor(cursor);
}



void BackgroundTasks::queryBestResults(const std::shared_ptr<ListCursor<ResultItem> >& cursor)
{
    cursor->setQuery([=]() -> std::vector<ResultItem> {
        NGLOG_DEBUG("Getting best results for session");
        return d->testRepository->bestResults();
    });
    d->resultCursors.registerCursor(cursor);
}



void BackgroundTasks::queryResultsForSession(
        const std::shared_ptr<ListCursor<ResultItem> >& cursor,
        long long sessionId)
{
    cursor->setQuery([=]() -> std::vector<ResultItem> {
        NGLOG_DEBUG("Getting results for session");
        return d->testRepository->resultsForSession(sessionId);
    });
    d->resultCursors.registerCursor(cursor);
}



void BackgroundTasks::queryResultsDetails(
        const std::shared_ptr<ResultCursor>& cursor,
        long long resultRowId)
{
    tfw::ResultGroup resultGroup = d->dataGateway->getResultForRowId(resultRowId);
    d->callback->dispatchCursorUpdate([=]{
        cursor->setResultGroup(resultGroup);
    });
}



void BackgroundTasks::queryChart(
        const std::shared_ptr<ChartCursor>& cursor,
        long long resultRowId,
        long long chartId)
{
    tfw::ResultGroup resultGroup = d->dataGateway->getResultForRowId(resultRowId);
    long long row = chartId;
    row -= resultGroup.results().size();
    row -= 2;
    row -= resultGroup.flags().size();
    d->callback->dispatchCursorUpdate([=]{
        try {
            cursor->setChart(resultGroup.charts().at(static_cast<size_t>(row)));
        }
        catch (...) {}
    });
}



const char* BackgroundTasks::getChartJson(
        long long resultRowId,
        long long chartId)
{
    tfw::ResultGroup resultGroup = d->dataGateway->getResultForRowId(resultRowId);
    long long row = chartId;
    row -= (long long)resultGroup.results().size();
    row -= 2;
    row -= (long long)resultGroup.flags().size();

    if(row > (long long) resultGroup.charts().size() - 1) {
        row = (long long) resultGroup.charts().size() - 1;
    }
    if(row < 0) {
        row = 0;
    }

    std::string json = resultGroup.charts().at(static_cast<size_t>(row)).toJsonString();

    // Dynamically allocate memory for the returned string
    char* ptr = new char[json.length() + 1]; // +1 for terminating NULL

    // Copy source string in dynamically allocated string buffer
    strcpy(ptr, json.c_str());

    return ptr;
}



void BackgroundTasks::queryCompareResults(
        const std::shared_ptr<ListCursor<CompareResult> >& cursor,
        const std::string& resultId,
        const std::string& deviceFilter)
{
    cursor->setQuery([=]() -> std::vector<CompareResult> {
        NGLOG_DEBUG("Getting compare results");
        bool hideDesktop = d->dataGateway->loadSetting("hideDesktop", false) != 0;
        return d->testRepository->compareResults(resultId, deviceFilter, hideDesktop);
    });
    d->compareCursors.registerCursor(cursor);
}



void BackgroundTasks::queryDuelResults(
    const std::shared_ptr<ListCursor<DuelItem> >& cursor,
    const std::string& apiA,
    const std::string& deviceA,
    const std::string& apiB,
    const std::string& deviceB)
{
    cursor->setQuery([=]() -> std::vector<DuelItem> {
        NGLOG_DEBUG("Getting duel results");
        return d->testRepository->duelResults(apiA, deviceA, apiB, deviceB);
    });
    d->duelCursors.registerCursor(cursor);
}



void BackgroundTasks::querySystemInfo(const std::shared_ptr<ListCursor<SystemInfoItem> >& cursor)
{
    cursor->setQuery([=]() -> std::vector<SystemInfoItem> {
        return d->systemInfo;
    });
    d->systemInfoCursors.registerCursor(cursor);
}



void BackgroundTasks::querySystemInfoAttributes(
        const std::shared_ptr<ListCursor<SystemInfoAttribute> >& cursor,
        long long systemInfoRowId)
{
    // XXX: assumes stable indices
    SystemInfoItem item = d->systemInfo.at(static_cast<size_t>(systemInfoRowId));
    cursor->setQuery([=]() -> std::vector<SystemInfoAttribute> {

        return item.attributes();
    });
    d->attributeCursors.registerCursor(cursor);
}



void BackgroundTasks::initialize(unsigned int renderApiFlags, unsigned int computeApiFlags)
{
	sysinf::Properties properties;

	if (!d->initialized)
	{

		NGLOG_INFO("App data path: %s", d->applicationConfig->appDataPath);
		NGLOG_INFO("Synchronization path: %s", d->applicationConfig->synchronizationPath);
		NGLOG_INFO("Config path: %s", d->applicationConfig->configPath);
		NGLOG_INFO("Data path: %s", d->applicationConfig->dataPath);
		NGLOG_INFO("Plugin path: %s", d->applicationConfig->pluginPath);
		NGLOG_INFO("Test image path: %s", d->applicationConfig->imagePath);
		NGLOG_INFO("Asset image path: %s", d->applicationConfig->assetImagePath);

		NGLOG_DEBUG("Loading dictionary");
		std::shared_ptr<Dictionary> dictionary = std::make_shared<Dictionary>();
		Poco::File(d->applicationConfig->synchronizationPath + "/localization").createDirectories();
		dictionary->initializeFromResource();
		dictionary->load(d->applicationConfig->synchronizationPath + "/localization");
		dictionary->setLocale(d->applicationConfig->locale);
		d->callback->updateDictionary(dictionary);

		NGLOG_DEBUG("Collecting system info");
		d->callback->updateInitMessage("LoadingDeviceInfo");

		d->systemInfoGateway->collectSystemInfo();

		properties = d->systemInfoGateway->getProperties();

		d->systemInfo = d->systemInfoGateway->getItems(d->applicationConfig->assetImagePath);

		if (properties.getString(sysinf::DEVICE_NAME).empty()) {
			std::string renderer = properties.getString(sysinf::API_GL_RENDERER);
			std::string processor = properties.getString(sysinf::CPU_NAME);
			properties.setString(sysinf::DEVICE_NAME, "#" + renderer + "#" + processor);
		}
		properties.setBool(sysinf::CORPORATE, d->applicationConfig->isCorporateVersion);
		properties.setString("appinfo/storename", d->applicationConfig->installerName);
		properties.setString(sysinf::APPINFO_INSTALLERNAME, d->applicationConfig->installerName);
		properties.setString(sysinf::APPINFO_PACKAGE_NAME, d->applicationConfig->packageName);
		properties.setString(sysinf::APPINFO_BENCHMARK_ID, d->applicationConfig->productId);
		properties.setString(sysinf::APPINFO_VERSION, d->applicationConfig->productVersion);
		properties.setString(sysinf::APPINFO_PLATFORM, d->applicationConfig->platformId);
		properties.setString(sysinf::APPINFO_LOCALE, d->applicationConfig->locale);
		NGLOG_TRACE(properties.toJsonString(false));

		NGLOG_DEBUG("Opening result database");
		d->dataGateway->openLocalDatabase(d->applicationConfig->appDataPath);
	}
	else
	{
		properties = d->systemInfoGateway->getProperties();
	}

	std::vector<Configuration> configurations = d->systemInfoGateway->getConfigurations();
	sysinf::SystemInfo systemInfo = d->systemInfoGateway->getSystemInfo();

	d->init();


    d->testCursors.setCallback(d->callback);
    d->configurationCursors.setCallback(d->callback);
    d->sessionCursors.setCallback(d->callback);
    d->resultCursors.setCallback(d->callback);
    d->compareCursors.setCallback(d->callback);
    d->duelCursors.setCallback(d->callback);
    d->systemInfoCursors.setCallback(d->callback);
    d->attributeCursors.setCallback(d->callback);

    if (!d->applicationConfig->isCorporateVersion) { // TODO: remove these kind of tests
        NGLOG_DEBUG("Initializing network gateway");
        d->callback->updateInitMessage("ConnectingToServer");
    }
    std::string message;
    std::string messageTimestamp;
    long long bytesToSynchronize = 0;

#ifndef DISABLE_NETMAN
	if (!d->initialized)
	{
		d->networkGateway->initialize(properties,
			d->applicationConfig->getSyncFlags(systemInfo),
			message,
			messageTimestamp,
			bytesToSynchronize);
		if (!d->networkGateway->storedUsername().empty()) {
			d->callback->loggedIn(d->networkGateway->storedUsername());
		}
		if (!messageTimestamp.empty()) {
			bool found = d->dataGateway->addServerMessage(messageTimestamp, message);
			if (!found) {
				d->callback->showMessage(message);
			}
		}
	}
#endif

    d->systemInfoGateway->updateSystemInfo(properties);
    d->systemInfo = d->systemInfoGateway->getItems(d->applicationConfig->assetImagePath);

	std::string testListName = d->applicationConfig->getTestListJsonName(systemInfo);

    NGLOG_DEBUG("Initializing test repository");
    d->testRepository->loadTestsFromJsonString(d->dataGateway->getResource(testListName), renderApiFlags, computeApiFlags);
    std::vector<std::pair<std::string, bool> > selections = d->dataGateway->getTestSelections();
    for (auto i = selections.begin(); i != selections.end(); ++i) {
        try {
            d->testRepository->setTestSelection(i->first, i->second);
        } catch (const std::runtime_error&) {
            /* Ignore unknown test */
        }
    }

	// reinitialize configurations
	d->testRepository->clearConfigurations();

	for (auto i = configurations.begin(); i != configurations.end(); i++) {
		Configuration &config = *i;

#if TARGET_OS_IOS > 0
		bool configIsMetal = (config.apiFlags() & tfw::ApiDefinition::METAL) != 0;
		if (!configIsMetal)
		{
			config.setEnabled(false);
		}
#endif

		if ( ((renderApiFlags & tfw::ApiDefinition::NOT_DEFINED) != 0) || ((config.apiFlags() & (unsigned int)renderApiFlags) > 0)) {
			d->testRepository->addConfiguration(*i);
		}
	}

	std::map<std::string, std::string> unsupported =
		d->systemInfoGateway->extractUnsupportedTests(properties);
	for (auto i = unsupported.begin(); i != unsupported.end(); ++i) {
		try {
			d->testRepository->addTestIncompReason(i->first, i->second);
		}
		catch (const std::runtime_error&) {
			/* Ignore unknown test */
		}
	}

	if (bytesToSynchronize == 0) {
		if (!d->initialized)
		{
			NGLOG_DEBUG("Opening compare database");
            std::string compare_database_filename =
				d->applicationConfig->synchronizationPath + "/compare/top-results.sqlite";

            d->dataGateway->openCompareDatabase(compare_database_filename);
		}
		updateCursors();
	}

	//if (!d->initialized) {
		d->callback->initializationFinished(bytesToSynchronize);
	//}

	d->initialized = true;
}



void BackgroundTasks::updateCursors()
{
	d->testCursors.updateCursors();
	d->configurationCursors.updateCursors();
	d->sessionCursors.updateCursors();
	d->resultCursors.updateCursors();
	d->compareCursors.updateCursors();
	d->duelCursors.updateCursors();
	d->systemInfoCursors.updateCursors();
	d->attributeCursors.updateCursors();
}

void BackgroundTasks::synchronize()
{
    d->callback->updateInitMessage("Synchronization");
    Poco::Timer synchronizationTimer(0, 100);
    Poco::TimerCallback<Private> callback(*d, &Private::pollSynchronizationProgress);
    synchronizationTimer.start(callback);
    d->networkGateway->synchronize();
    synchronizationTimer.stop();

    NGLOG_DEBUG("Updating dictionary");
    std::shared_ptr<Dictionary> dictionary = std::make_shared<Dictionary>();
    dictionary->initializeFromResource();
    dictionary->load(d->applicationConfig->synchronizationPath + "/localization");
    dictionary->setLocale(d->applicationConfig->locale);
    d->callback->updateDictionary(dictionary);

    NGLOG_DEBUG("Opening compare database");
    d->dataGateway->openCompareDatabase(d->applicationConfig->synchronizationPath +
            "/compare/top-results.sqlite");
    d->testCursors.updateCursors();
    d->configurationCursors.updateCursors();
    d->sessionCursors.updateCursors();
    d->resultCursors.updateCursors();
    d->compareCursors.updateCursors();
    d->duelCursors.updateCursors();
    d->systemInfoCursors.updateCursors();
    d->attributeCursors.updateCursors();
    d->callback->synchronizationFinished();
}



void BackgroundTasks::clearResults()
{
    d->dataGateway->clear();
    d->sessionCursors.updateCursors();
    d->resultCursors.updateCursors();
    d->duelCursors.updateCursors();
}



void BackgroundTasks::login(const std::string& username, const std::string& password)
{
    d->networkGateway->login(username, password);
    d->callback->loggedIn(username);
}



void BackgroundTasks::logout()
{
    d->networkGateway->logout();
    d->callback->loggedOut();
}



void BackgroundTasks::deleteUser()
{
    d->networkGateway->deleteUser();
    d->callback->deletedUser();
}



void BackgroundTasks::signUp(
        const std::string &email,
        const std::string &username,
        const std::string &password)
{
    d->networkGateway->signUp(email, username, password);
    d->callback->signedUp();
    d->callback->loggedIn(username);
}



void BackgroundTasks::setGroupSelection(const std::string &groupId, bool isSelected)
{
    d->testRepository->setGroupSelection(groupId, isSelected);
    d->testCursors.updateCursors();
}



void BackgroundTasks::setTestSelection(const std::string &testId, bool isSelected)
{
    d->testRepository->setTestSelection(testId, isSelected);
    d->testCursors.updateCursors();
}



void BackgroundTasks::toggleAllTestSelection()
{
    d->testRepository->toggleAllTestSelection();
    d->testCursors.updateCursors();
}



void BackgroundTasks::selectConfiguration(int configurationIndex)
{
    d->testRepository->selectConfiguration(configurationIndex);
    d->testCursors.updateCursors();
    d->configurationCursors.updateCursors();
}



void BackgroundTasks::setCustomResolution(int width, int height)
{
    d->customWidth = width;
    d->customHeight = height;
}



void BackgroundTasks::setCustomBrightness(double brightness)
{
    d->brightness = brightness;
}



void BackgroundTasks::setEndlessTestRun(bool endless)
{
    d->endless = endless;
}



void BackgroundTasks::setHideDesktopDevices(bool enabled)
{
    d->dataGateway->saveSetting("hideDesktop", enabled);
    d->compareCursors.updateCursors();
}



void BackgroundTasks::pollBattery()
{
    for (auto i = d->systemInfo.begin(); i != d->systemInfo.end(); ++i) {
        if (i->name() == "Battery") {
            switch (d->runtimeInfo->batteryStatus()) {
            case tfw::RuntimeInfo::BATTERY_CHARGING:
                i->setMajor("Charging");
                break;
            case tfw::RuntimeInfo::BATTERY_DISCHARGING:
                i->setMajor("Discharging");
                break;
            case tfw::RuntimeInfo::BATTERY_FULL:
                i->setMajor("Full");
                break;
            case tfw::RuntimeInfo::BATTERY_NOT_CHARGING:
                i->setMajor("Not charging");
                break;
            case tfw::RuntimeInfo::BATTERY_UNKNOWN:
                i->setMajor("Unknown");
                break;
            default:
                assert(false);
                break;
            }
            std::ostringstream oss;
            oss << d->runtimeInfo->batteryLevelPercent() << "%";
            i->setMinor(oss.str());
            break;
        }
    }
    d->systemInfoCursors.updateCursors();
}



void BackgroundTasks::startSession(bool runAllTests)
{
    if (runAllTests) {
        d->scheduledTests = d->testRepository->allAvailableTests();
    } else {
        d->scheduledTests = d->testRepository->selectedAvailableTests();
    }

    d->session.setSessionId(Poco::Timestamp().epochMicroseconds() / 1000);
    d->session.setFinished(false);
    d->session.setConfigurationName(d->testRepository->selectedConfiguration().name());
    d->dataGateway->addSession(d->session);
    prepareNextTest();
    
    if (!d->checkBattery(d->scheduledTests)) {
        d->callback->sessionFinished();
        
        return;
    }
}



void BackgroundTasks::prepareNextTest()
{
    if (d->scheduledTests.empty()) {
        finishSession();
        return;
    }

    TestItem testItem = d->scheduledTests.front();
    d->scheduledTests.erase(d->scheduledTests.begin());

    tfw::Descriptor descriptor = d->dataGateway->getDescriptorByTestId(testItem.testId());
    descriptor.env().setReadPath(d->applicationConfig->dataPath + "/" + descriptor.dataPrefix() + "/");
    descriptor.env().setWritePath(d->applicationConfig->appDataPath + "/");
    descriptor.env().compute().setConfigIndex(d->testRepository->selectedConfigurationIndex());

	auto apis = d->testRepository->selectedConfiguration().ApiDefinitions();
    descriptor.env().graphics().setDeviceId((apis.empty()) ? "" : apis.begin()->deviceId());
    descriptor.env().graphics().setDeviceIndex((apis.empty()) ? -3 : apis.begin()->deviceIndex());

    if ((d->customWidth > 0) && (d->customHeight > 0)) {
        descriptor.setRawConfig("test_width", d->customWidth);
        descriptor.setRawConfig("test_height", d->customHeight);
        descriptor.setRawConfig("virtual_resolution", true);
    }
    if (d->brightness >= 0) {
        descriptor.setRawConfig("brightness", d->brightness);
    }
    if(d->endless) {
        descriptor.setRawConfig("endless", d->endless);
    }

    descriptor.env().graphics().setFullScreen(!d->applicationConfig->isWindowModeEnabled);
    //descriptor.env().graphics().setFullScreen(false);

    for (std::vector<SystemInfoItem>::iterator it = d->systemInfo.begin(); it != d->systemInfo.end(); ++it) {
        if(it->name() == "device") {
            descriptor.env().setDevice(it->major().append(" | ").append(it->minor()));
        }

        if(it->name() == "os") {
            descriptor.env().setOs(it->major().append(" | ").append(it->minor()));
        }
    }

    descriptor.env().setTimestamp(Poco::DateTimeFormatter::format(Poco::Timestamp(), "%Y.%m.%d. %H:%M:%S"));

    d->currentDescriptor = descriptor;
    d->callback->createContext(testItem, descriptor.env().graphics(), descriptor);
}



void BackgroundTasks::loadTest(GraphicsContext* context, int width, int height)
{
    d->interrupted = 0;
    d->currentDescriptor.env().setWidth(width);
    d->currentDescriptor.env().setHeight(height);
    d->testRunner->loadTest(d->applicationConfig->pluginPath, d->currentDescriptor, *context);

    tfw::ResultGroup testResult = d->testRunner->getResult();
    testResult.setConfiguration(d->session.configurationName());
    if (d->interrupted) {
        testResult.results().front().setStatus(tfw::Result::CANCELLED);
        d->dataGateway->addResults(d->session, testResult);
    } else if (testResult.results().empty() || testResult.results().front().status() != tfw::Result::OK) {
        d->dataGateway->addResults(d->session, testResult);
        prepareNextTest();
    } else {
        d->callback->testLoaded();
    }
}



void BackgroundTasks::runTest()
{
    if (d->interrupted) {
        return;
    }
    d->testRunner->runTest();
    tfw::ResultGroup testResult = d->testRunner->getResult();
    testResult.setConfiguration(d->session.configurationName());
    d->dataGateway->addResults(d->session, testResult);
    if (!d->interrupted) {
        prepareNextTest();
    }
}



void BackgroundTasks::finishSession()
{
    d->scheduledTests.clear();
    d->session.setFinished(true);
    d->dataGateway->addSession(d->session);
    d->sessionCursors.updateCursors();
    d->resultCursors.updateCursors();
    d->duelCursors.updateCursors();
    d->callback->sessionFinished();
    d->callback->showResults();
}



bool BackgroundTasks::Private::checkBattery(const std::vector<TestItem> &scheduledTests)
{
	for (auto i = scheduledTests.begin(); i != scheduledTests.end(); ++i) {
        if (i->requires("battery") &&
                ((runtimeInfo->batteryStatus() == tfw::RuntimeInfo::BATTERY_CHARGING) ||
                (runtimeInfo->batteryStatus() == tfw::RuntimeInfo::BATTERY_FULL)))
        {
            this->
            callback->reportError(BenchmarkException(BenchmarkException::BATTERY_CONNECTED, ""));
            return false;
        }
    }
    return true;
}



void BackgroundTasks::Private::pollSynchronizationProgress(Poco::Timer&)
{
    float progress = 0.0f;
    long long bytesNeeded = 0;
    long long bytesWritten = 0;
    networkGateway->pollSyncProgress(progress, bytesNeeded, bytesWritten);
    callback->updateSyncProgress(progress, bytesNeeded, bytesWritten);
}
