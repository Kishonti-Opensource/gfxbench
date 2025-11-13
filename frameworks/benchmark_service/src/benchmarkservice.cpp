/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "benchmarkservice.h"

#include "applicationconfig.h"
#include "backgroundtasks.h"
#include "benchmarkexception.h"
#include "datagateway.h"
#include "dictionary.h"
#include "executor.h"
#include "listcursor.h"
#include "resultcursor.h"
#include "chartcursor.h"
#include "networkgateway.h"
#include "systeminfogateway.h"
#include "testrunner.h"

#ifdef IS_COMMUNITY
    #include "communitydatagateway.h"
    #include "communitynetworkgateway.h"
#else
    #include "corporatedatagateway.h"
#endif

#include "schemas/descriptors.h"

#include "ng/log.h"

#include <Poco/Timer.h>
#include <Poco/Timestamp.h>



const long long MINIMUM_LOADING_us = 2000000;
const long BATTERY_POLL_INTERVAL_ms = 5000;



enum State {
    Stopped,
    Initializing,
    WaitingForSynchronization,
    Synchronizing,
    Running,
    WaitingForServer,
    LoadingTest,
    RunningTest
};



class BenchmarkServiceImpl: public BenchmarkService, public BackgroundTaskCallback
{
public:
    BenchmarkServiceImpl(BenchmarkServiceCallback* callback, tfw::RuntimeInfo* runtimeInfo);
    ~BenchmarkServiceImpl();

    /* BenchmarkService */
    void BENCHMARK_SERVICE_API destroy() override;
    Cursor* BENCHMARK_SERVICE_API getTestsPointer() override;
    Cursor* BENCHMARK_SERVICE_API getConfigurationsPointer() override;
    Cursor* BENCHMARK_SERVICE_API getSessionsPointer() override;
    Cursor* BENCHMARK_SERVICE_API getBestResultsPointer() override;
    Cursor* BENCHMARK_SERVICE_API getResultForSessionPointer(long long sessionId) override;
    Cursor* BENCHMARK_SERVICE_API getResultDetailsPointer(long long resultRowId) override;
    Cursor* BENCHMARK_SERVICE_API getChartPointer(
            long long resultRowId,
            long long chartRowId) override;
    const char* BENCHMARK_SERVICE_API getChartJsonString(
            long long resultRowId,
            long long chartRowId) override;
    Cursor* BENCHMARK_SERVICE_API getCompareResultsPointer(
            const char* resultId,
            const char* filter) override;
    Cursor* BENCHMARK_SERVICE_API getDuelResultsPointer(
            const char* apiA,
            const char* deviceA,
            const char* apiB,
            const char* deviceB) override;
    Cursor* BENCHMARK_SERVICE_API getSystemInfoPointer() override;
    Cursor* BENCHMARK_SERVICE_API getSystemInfoAttributesPointer(
            long long systemInfoRowId) override;
    Cursor* BENCHMARK_SERVICE_API getAvailableLanguagesPointer() override;
    const char* BENCHMARK_SERVICE_API getLocalizedStringPointer(const char* key) override;
    const char* BENCHMARK_SERVICE_API getConfigPointer(ConfigKey key) override;
    void BENCHMARK_SERVICE_API setConfig(ConfigKey key, const char* value) override;
    void BENCHMARK_SERVICE_API processEvents() override;
    void BENCHMARK_SERVICE_API start(	unsigned int renderApiFlags = (unsigned int)tfw::ApiDefinition::NOT_DEFINED,
										unsigned int computeApiFlags = (unsigned int)tfw::ApiDefinition::NOT_DEFINED) override;
    void BENCHMARK_SERVICE_API stop() override;
    void BENCHMARK_SERVICE_API setGroupSelection(const char* groupId, bool isSelected) override;
    void BENCHMARK_SERVICE_API setTestSelection(const char* testId, bool isSelected) override;
    void BENCHMARK_SERVICE_API toggleAllTestSelection() override;
    void BENCHMARK_SERVICE_API selectConfiguration(int configurationIndex) override;
    void BENCHMARK_SERVICE_API setCustomResolution(int width, int height) override;
    void BENCHMARK_SERVICE_API setCustomBrightness(double brightness) override;
    void BENCHMARK_SERVICE_API setEndlessTestRun(bool endless) override;
    void BENCHMARK_SERVICE_API runAllTests() override;
    void BENCHMARK_SERVICE_API runSelectedTests() override;
    void BENCHMARK_SERVICE_API loadTest(
            GraphicsContext* graphicsContext,
            int width,
            int height) override;
    void BENCHMARK_SERVICE_API runTest() override;
    void BENCHMARK_SERVICE_API stopTests() override;
    void BENCHMARK_SERVICE_API skipTest() override;
    void BENCHMARK_SERVICE_API acceptSynchronization() override;
    void BENCHMARK_SERVICE_API clearResults() override;
    void BENCHMARK_SERVICE_API login(const char* username, const char* password) override;
    void BENCHMARK_SERVICE_API logout() override;
    void BENCHMARK_SERVICE_API deleteUser() override;
    void BENCHMARK_SERVICE_API signUp(
            const char* email,
            const char* username,
            const char* password) override;
    void BENCHMARK_SERVICE_API setLogoutOnClose(bool enabled) override;
    void BENCHMARK_SERVICE_API setHideDesktopDevices(bool enabled) override;
    
    std::string getChartJson(long long resultRowId, long long chartRowId) override;

    /* BackgroundTaskCallback */
    void reportError(const BenchmarkException &error) override;
    void initializationFinished(long long bytesToSync) override;
    void synchronizationFinished() override;
    void updateInitMessage(const std::string &message) override;
    void updateSyncProgress(
            float progress,
            long long bytesNeeded,
            long long bytesWritten) override;
    void loggedIn(const std::string &username) override;
    void loggedOut() override;
    void deletedUser() override;
    void signedUp() override;
    void showMessage(const std::string &message) override;
    void showResults() override;
    void sessionFinished() override;
    void createContext(const TestItem& testItem, const tfw::Graphics& graphics, const tfw::Descriptor& desc) override;
    void testLoaded() override;
    void dispatchCursorUpdate(const std::function<void()>& task) override;
    void updateDictionary(const std::shared_ptr<Dictionary>& newDictionary) override;
private:
    BenchmarkServiceCallback* mCallback;
    Executor mBackgroundThread;
    Executor mMainThread;
    bool mLogoutOnClose;
    State mState;
    Poco::Timer mLoadingTimer;
    Poco::TimerCallback<BenchmarkServiceImpl> mLoadingTimerCallback;
    Poco::Timer mBatteryTimer;
    Poco::TimerCallback<BenchmarkServiceImpl> mBatteryTimerCallback;
    long long mLoadStartTime;
    ApplicationConfig mApplicationConfig;
    tfw::RuntimeInfo* mRuntimeInfo;
    std::unique_ptr<DataGateway> mDataGateway;
    std::unique_ptr<NetworkGateway> mNetworkGateway;
    std::unique_ptr<SystemInfoGateway> mSystemInfoGateway;
    std::unique_ptr<TestRunner> mTestRunner;
    std::unique_ptr<BackgroundTasks> mBackgroundTasks;
    std::shared_ptr<Dictionary> mDictionary;
    ListCursorContainer<Language> mLanguageCursors;

    void pollBattery(Poco::Timer& timer);
    void dispatchTestLoaded(Poco::Timer&);
    void runInBackground(const std::string& taskName, const std::function<void()>& task);
    void runOnMainThread(const std::function<void()>& task);
    BenchmarkServiceImpl(const BenchmarkServiceImpl&); // No copy
    BenchmarkServiceImpl& operator=(const BenchmarkServiceImpl&); // No copy

	unsigned int mRenderApiFlags;
	unsigned int mComputeApiFlags;
};



extern "C" BENCHMARK_SERVICE_EXPORT BenchmarkService* BENCHMARK_SERVICE_API createBenchmarkService(
        BenchmarkServiceCallback* callback,
        tfw::RuntimeInfo* runtimeInfo)
{
    return new BenchmarkServiceImpl(callback, runtimeInfo);
}



BenchmarkServiceImpl::BenchmarkServiceImpl(
        BenchmarkServiceCallback* callback,
        tfw::RuntimeInfo* runtimeInfo)
:
    mLoadingTimerCallback(*this, &BenchmarkServiceImpl::dispatchTestLoaded),
    mBatteryTimerCallback(*this, &BenchmarkServiceImpl::pollBattery)
{
    assert(callback != nullptr);
    assert(runtimeInfo != nullptr);

    mLogoutOnClose = false;
    mState = Stopped;
    mRuntimeInfo = runtimeInfo;
    mCallback = callback;
    mLanguageCursors.setCallback(this);

#ifdef IS_COMMUNITY
    mApplicationConfig.isCorporateVersion = false;
    mDataGateway.reset(new CommunityDataGateway());
    mNetworkGateway.reset(new CommunityNetworkGateway(mApplicationConfig));
#else
    mApplicationConfig.isCorporateVersion = true;
    mDataGateway.reset(new CorporateDataGateway(mApplicationConfig));
    mNetworkGateway.reset(new NullNetworkGateway());
#endif
    mSystemInfoGateway.reset(new SystemInfoGateway());
    mTestRunner.reset(new TestRunner());
    mBackgroundTasks.reset(new BackgroundTasks(
            mApplicationConfig,
            *this,
            *mDataGateway,
            *mNetworkGateway,
            *mRuntimeInfo,
            *mSystemInfoGateway,
            *mTestRunner));
    mDictionary.reset(new Dictionary());
}



BenchmarkServiceImpl::~BenchmarkServiceImpl()
{
    stop();
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::destroy()
{
    delete this;
}

std::string BenchmarkServiceImpl::getChartJson(long long resultRowId, long long chartRowId) {
    const char* jsonChars = getChartJsonString(resultRowId, chartRowId);
    std::string jsonString = jsonChars;
    delete[](jsonChars);
    return jsonString;
}



Cursor* BenchmarkServiceImpl::getTestsPointer()
{
    auto cursor = std::make_shared<ListCursor<TestItem> >();
    runInBackground("getTests", [=](){
        mBackgroundTasks->queryTests(cursor);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getConfigurationsPointer()
{
    auto cursor = std::make_shared<ListCursor<Configuration> >();
    runInBackground("getConfigurations", [=](){
        mBackgroundTasks->queryConfigurations(cursor);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getSessionsPointer()
{
    auto cursor = std::make_shared<ListCursor<Session> >();
    runInBackground("getSessions", [=](){
        mBackgroundTasks->querySessions(cursor);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getBestResultsPointer()
{
    auto cursor = std::make_shared<ListCursor<ResultItem> >();
    runInBackground("getBestResults", [=](){
        mBackgroundTasks->queryBestResults(cursor);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getResultForSessionPointer(long long sessionId)
{
    auto cursor = std::make_shared<ListCursor<ResultItem> >();
    runInBackground("getResultForSession", [=](){
        mBackgroundTasks->queryResultsForSession(cursor, sessionId);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getResultDetailsPointer(long long resultRowId)
{
    auto cursor = std::make_shared<ResultCursor>();
    runInBackground("getResultDetails", [=](){
        mBackgroundTasks->queryResultsDetails(cursor, resultRowId);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getChartPointer(
        long long resultRowId,
        long long chartRowId)
{
    auto cursor = std::make_shared<ChartCursor>();
    runInBackground("getChart", [=](){
        mBackgroundTasks->queryChart(cursor, resultRowId, chartRowId);
    });
    return cursor->lock();
}



const char* BENCHMARK_SERVICE_API BenchmarkServiceImpl::getChartJsonString(
        long long resultRowId,
        long long chartRowId)
{
    return mBackgroundTasks->getChartJson(resultRowId, chartRowId);
}



Cursor* BenchmarkServiceImpl::getCompareResultsPointer(const char* resultId, const char* filter)
{
    auto cursor = std::make_shared<ListCursor<CompareResult> >();
    std::string resultIdString = resultId;
    std::string filterString = filter;
    runInBackground("getCompareResults", [=](){
        mBackgroundTasks->queryCompareResults(cursor, resultIdString, filterString);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getDuelResultsPointer(
        const char* apiA,
        const char* deviceA,
        const char* apiB,
        const char* deviceB)
{
    auto cursor = std::make_shared<ListCursor<DuelItem> >();
    std::string apiAString = apiA;
    std::string apiBString = apiB;
    std::string deviceAString = deviceA;
    std::string deviceBString = deviceB;

    runInBackground("getDuelResults", [=](){
        mBackgroundTasks->queryDuelResults(cursor, apiAString, deviceAString, apiBString, deviceBString);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getSystemInfoPointer()
{
    auto cursor = std::make_shared<ListCursor<SystemInfoItem> >();
    runInBackground("getSystemInfoItems", [=](){
        mBackgroundTasks->querySystemInfo(cursor);
    });
    return cursor->lock();
}



Cursor* BenchmarkServiceImpl::getSystemInfoAttributesPointer(long long systemInfoRowId)
{
    auto cursor = std::make_shared<ListCursor<SystemInfoAttribute> >();
    runInBackground("", [=](){
        mBackgroundTasks->querySystemInfoAttributes(cursor, systemInfoRowId);
    });
    return cursor->lock();
}



Cursor* BENCHMARK_SERVICE_API BenchmarkServiceImpl::getAvailableLanguagesPointer()
{
    auto cursor = std::make_shared<ListCursor<Language> >();
    cursor->setQuery([this](){ return mDictionary->getAvailableLanguages(); });
    mLanguageCursors.registerCursor(cursor);
    return cursor->lock();
}



const char* BENCHMARK_SERVICE_API BenchmarkServiceImpl::getLocalizedStringPointer(const char* key)
{
    return mDictionary->getLocalizedString(key);
}



const char* BENCHMARK_SERVICE_API BenchmarkServiceImpl::getConfigPointer(ConfigKey key)
{
    switch (key) {
    case PRODUCT_ID:
        return mApplicationConfig.productId.c_str();
    case PRODUCT_VERSION:
        return mApplicationConfig.productVersion.c_str();
    case PLATFORM_ID:
        return mApplicationConfig.platformId.c_str();
    case INSTALLER_NAME:
        return mApplicationConfig.installerName.c_str();
    case PACKAGE_NAME:
        return mApplicationConfig.packageName.c_str();
    case LOCALE:
        return mApplicationConfig.locale.c_str();
    case WINDOW_MODE_ENABLED:
        return mApplicationConfig.isWindowModeEnabled ? "true" : "false";
    case CORPORATE_VERSION:
        return mApplicationConfig.isCorporateVersion ? "true" : "false";
    case CONFIG_PATH:
        return mApplicationConfig.configPath.c_str();
    case DATA_PATH:
        return mApplicationConfig.dataPath.c_str();
    case PLUGIN_PATH:
        return mApplicationConfig.pluginPath.c_str();
    case APPDATA_PATH:
        return mApplicationConfig.appDataPath.c_str();
    case SYNCHRONIZATION_PATH:
        return mApplicationConfig.synchronizationPath.c_str();
    case ASSET_IMAGE_PATH:
        return mApplicationConfig.assetImagePath.c_str();
    case TEST_IMAGE_PATH:
        return mApplicationConfig.imagePath.c_str();
    case NEEDS_SYNC_BASED_ON_DATE:
        return mApplicationConfig.needsSyncBasedOnDate ? "true" : "false";
    default:
        assert(false);
        return "";
    }
}




void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setConfig(ConfigKey key, const char* value)
{
    switch (key) {
    case PRODUCT_ID:
        mApplicationConfig.productId = value;
        break;
    case PRODUCT_VERSION:
        mApplicationConfig.productVersion = value;
        break;
    case PLATFORM_ID:
        mApplicationConfig.platformId = value;
        break;
    case INSTALLER_NAME:
        mApplicationConfig.installerName = value;
        break;
    case PACKAGE_NAME:
        mApplicationConfig.packageName = value;
        break;
    case LOCALE:
        mApplicationConfig.locale = value;
        mDictionary->setLocale(value);
        mCallback->localizationChanged();
        break;
    case WINDOW_MODE_ENABLED:
        mApplicationConfig.isWindowModeEnabled = (value == std::string("true"));
        break;
    case CONFIG_PATH:
        mApplicationConfig.configPath = value;
        break;
    case DATA_PATH:
        mApplicationConfig.dataPath = value;
        break;
    case PLUGIN_PATH:
        mApplicationConfig.pluginPath = value;
        break;
    case APPDATA_PATH:
        mApplicationConfig.appDataPath = value;
        break;
    case SYNCHRONIZATION_PATH:
        mApplicationConfig.synchronizationPath = value;
        break;
    case ASSET_IMAGE_PATH:
        mApplicationConfig.assetImagePath = value;
        break;
    case TEST_IMAGE_PATH:
        mApplicationConfig.imagePath = value;
        break;
    case NEEDS_SYNC_BASED_ON_DATE:
        mApplicationConfig.needsSyncBasedOnDate = (value == std::string("true"));
        break;
    default:
        assert(false);
        break;
    }
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::processEvents()
{
    mMainThread.executeWaiting();
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::start(unsigned int renderApiFlags,unsigned int computeApiFlags)
{
    NGLOG_INFO("BenchmarkService Start");

    mCallback->updateInitMessage("Loading");
	mRenderApiFlags = renderApiFlags;
	mComputeApiFlags = computeApiFlags;

	try
	{
		mBackgroundThread.start();
		runInBackground("initialize", [=]() {
			mBackgroundTasks->initialize(renderApiFlags, computeApiFlags);
		});
	}
	catch (std::exception& e)
	{
		NGLOG_ERROR("BenchmarkService Abandoning Start. A query is already occupying background thread.");
		throw e;
	}
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::stop()
{
    if (mState == Stopped) {
        return;
    }
    NGLOG_INFO("BenchmarkService Stop");

    mBackgroundTasks->interrupt();
    runInBackground("stop", [=](){
        if (mLogoutOnClose) {
            mBackgroundTasks->logout();
        }
        mBackgroundTasks->stop();
    });

    mBackgroundThread.stop();
    mLoadingTimer.stop();
    mBatteryTimer.stop();
    mState = Stopped;
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setGroupSelection(
        const char* groupId,
        bool isSelected)
{
    std::string groupIdString = groupId;
    runInBackground("setGroupSelection", [=](){
        mBackgroundTasks->setGroupSelection(groupIdString, isSelected);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setTestSelection(
        const char* testId,
        bool isSelected)
{
    std::string testIdString = testId;
    runInBackground("setTestSelection", [=](){
        mBackgroundTasks->setTestSelection(testIdString, isSelected);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::toggleAllTestSelection()
{
    runInBackground("setTestSelection", [=](){
        mBackgroundTasks->toggleAllTestSelection();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::selectConfiguration(int configurationIndex)
{
    runInBackground("selectConfiguration", [=](){
        mBackgroundTasks->selectConfiguration(configurationIndex);
		mBackgroundTasks->updateCursors();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setCustomResolution(int width, int height)
{
    runInBackground("setCustomResolution", [=](){
        mBackgroundTasks->setCustomResolution(width, height);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setCustomBrightness(double brightness)
{
    runInBackground("setCustomBrightness", [=](){
        mBackgroundTasks->setCustomBrightness(brightness);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setEndlessTestRun(bool endless)
{
    runInBackground("setEndlessTestRun", [=](){
        mBackgroundTasks->setEndlessTestRun(endless);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::runAllTests()
{
    runInBackground("runAllTests", [=](){
        mBackgroundTasks->startSession(true);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::runSelectedTests()
{
    runInBackground("runSelectedTests", [=](){
        mBackgroundTasks->startSession(false);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::loadTest(
        GraphicsContext* context,
        int width,
        int height)
{
    mLoadStartTime = Poco::Timestamp().epochMicroseconds();
    runInBackground("loadTest", [=](){
        mBackgroundTasks->loadTest(context, width, height);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::runTest()
{
    runInBackground("runTest", [=](){
        mBackgroundTasks->runTest();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::stopTests()
{
    if ((mState != LoadingTest) && (mState != RunningTest)) {
        NGLOG_WARN("Unexpected stopTests");
        return;
    }

    mBackgroundTasks->interrupt();
    mLoadingTimer.stop();
    runInBackground("prepareNextTest", [=](){
        mBackgroundTasks->finishSession();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::skipTest()
{
    if ((mState != LoadingTest) && (mState != RunningTest)) {
        NGLOG_WARN("Unexpected skipTest");
        return;
    }

    mBackgroundTasks->interrupt();
    mLoadingTimer.stop();
    runInBackground("prepareNextTest", [=](){
        mBackgroundTasks->prepareNextTest();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::acceptSynchronization()
{
    if (mState != WaitingForSynchronization) {
        NGLOG_WARN("Unexpected acceptSynchronization");
        return;
    }
    mState = Synchronizing;
    runInBackground("acceptSynchronization", [=](){
        mBackgroundTasks->synchronize();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::clearResults()
{
    runInBackground("clearResults", [=](){
        mBackgroundTasks->clearResults();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::login(const char* username, const char* password)
{
    if (mState != Running) return;
    std::string usernameString = username;
    std::string passwordString = password;
    mState = WaitingForServer;
    runInBackground("login", [=](){
        mBackgroundTasks->login(usernameString, passwordString);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::logout()
{
    if (mState != Running) return;
    mState = WaitingForServer;
    runInBackground("logout", [=](){
        mBackgroundTasks->logout();
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::deleteUser()
{
    if (mState != Running) return;
    mState = WaitingForServer;
    runInBackground("deleteUser", [=]() {
        mBackgroundTasks->deleteUser();
        });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::signUp(
        const char* email,
        const char* username,
        const char* password)
{
    if (mState != Running) return;
    std::string emailString = email;
    std::string usernameString = username;
    std::string passwordString = password;
    mState = WaitingForServer;
    runInBackground("signUp", [=](){
        mBackgroundTasks->signUp(emailString, usernameString, passwordString);
    });
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setLogoutOnClose(bool enabled)
{
    NGLOG_INFO("setLogoutOnClose: %s", (int)enabled);
    mLogoutOnClose = enabled;
}



void BENCHMARK_SERVICE_API BenchmarkServiceImpl::setHideDesktopDevices(bool enabled)
{
    NGLOG_INFO("setHideDesktopDevices: %s", (int)enabled);
    runInBackground("setHideDesktopDevices", [=](){
        mBackgroundTasks->setHideDesktopDevices(enabled);
    });
}



void BenchmarkServiceImpl::reportError(const BenchmarkException &error)
{
    std::shared_ptr<BenchmarkException> errorCopy(error.clone());
    runOnMainThread([=](){
        if (mState == WaitingForServer) {
            mState = Running;
        }
        mCallback->reportError(errorCopy->errorId(), errorCopy->what(), errorCopy->isFatal());
        if (error.isFatal()) {
            stop();
        }
    });
}



void BenchmarkServiceImpl::initializationFinished(long long bytesToSynchronize)
{
    runOnMainThread([=](){
        if (bytesToSynchronize > 0) {
            NGLOG_INFO("askSync");
            mState = WaitingForSynchronization;
            mCallback->askSynchronization(bytesToSynchronize);
        } else {
            NGLOG_INFO("finishInitialization");
            mState = Running;
            mCallback->initializationFinished();
            mBatteryTimer.stop();
            mBatteryTimer.setPeriodicInterval(BATTERY_POLL_INTERVAL_ms);
            mBatteryTimer.start(mBatteryTimerCallback);
        }
    });
}



void BenchmarkServiceImpl::synchronizationFinished()
{
    runOnMainThread([=](){
        NGLOG_INFO("finishSynchronization");
        mState = Running;
        mCallback->initializationFinished();
        mBatteryTimer.stop();
        mBatteryTimer.setPeriodicInterval(BATTERY_POLL_INTERVAL_ms);
        mBatteryTimer.start(mBatteryTimerCallback);
    });
}



void BenchmarkServiceImpl::updateInitMessage(const std::string &message)
{
    runOnMainThread([=](){
        mCallback->updateInitMessage(message.c_str());
    });
}



void BenchmarkServiceImpl::updateSyncProgress(
        float progress,
        long long bytesNeeded,
        long long bytesWritten)
{
    runOnMainThread([=](){
        mCallback->updateSyncProgress(progress, bytesNeeded, bytesWritten);
    });
}



void BenchmarkServiceImpl::loggedIn(const std::string &username)
{
    runOnMainThread([=](){
        NGLOG_INFO("loggedIn");
        if (mState == WaitingForServer) {
            mState = Running;
        }
        mCallback->loggedIn(username.c_str());
    });
}



void BenchmarkServiceImpl::loggedOut()
{
    runOnMainThread([=](){
        NGLOG_INFO("loggedOut");
        if (mState == WaitingForServer) {
            mState = Running;
        }
        mCallback->loggedOut();
    });
}



void BenchmarkServiceImpl::deletedUser()
{
    runOnMainThread([=]() {
        NGLOG_INFO("deletedUser");
        if (mState == WaitingForServer) {
            mState = Running;
        }
        mCallback->deletedUser();
        });
}



void BenchmarkServiceImpl::signedUp()
{
    runOnMainThread([=](){
        NGLOG_INFO("signedUp");
        if (mState == WaitingForServer) {
            mState = Running;
        }
        mCallback->signedUp();
    });
}



void BenchmarkServiceImpl::showMessage(const std::string &message)
{
    runOnMainThread([=](){
        mCallback->showMessage(message.c_str());
    });
}



void BenchmarkServiceImpl::showResults()
{
    runOnMainThread([=](){
        mCallback->showResults();
    });
}



void BenchmarkServiceImpl::sessionFinished()
{
    runOnMainThread([=](){
        NGLOG_INFO("finishSession");
        mState = Running;
        mBatteryTimer.stop();
        mBatteryTimer.setPeriodicInterval(BATTERY_POLL_INTERVAL_ms);
        mBatteryTimer.start(mBatteryTimerCallback);
    });
}



void BenchmarkServiceImpl::createContext(
        const TestItem& testItem,
        const tfw::Graphics& graphics,
        const tfw::Descriptor& desc)
{
    runOnMainThread([=](){
        mState = LoadingTest;
        mBatteryTimer.stop();
        mCallback->createContext(testItem.testId().c_str(), (mApplicationConfig.imagePath + '/' +
                testItem.variantOf() + "_loading.png").c_str(), graphics, desc);
    });
}



void BenchmarkServiceImpl::testLoaded()
{
    runOnMainThread([=](){
        long long loadEndTime = Poco::Timestamp().epochMicroseconds();
        long long delayus = MINIMUM_LOADING_us - (loadEndTime - mLoadStartTime);
        if (delayus > 0) {
            mLoadingTimer.stop();
            mLoadingTimer.setStartInterval(static_cast<long>(delayus / 1000));
            mLoadingTimer.start(mLoadingTimerCallback);
        } else {
            mCallback->testLoaded();
        }
    });
}



void BenchmarkServiceImpl::dispatchCursorUpdate(const std::function<void()>& task)
{
    runOnMainThread(task);
}



void BenchmarkServiceImpl::updateDictionary(const std::shared_ptr<Dictionary>& newDictionary)
{
    runOnMainThread([=](){
        mDictionary = newDictionary;
        mLanguageCursors.updateCursors();
        mCallback->localizationChanged();
    });
}



void BenchmarkServiceImpl::pollBattery(Poco::Timer& timer)
{
    runInBackground(std::string(), [=](){
        mBackgroundTasks->pollBattery();
    });
}



void BenchmarkServiceImpl::dispatchTestLoaded(Poco::Timer&)
{
    runOnMainThread([=](){
        mState = RunningTest;
        mLoadingTimer.stop();
        mCallback->testLoaded();
    });
}



void BenchmarkServiceImpl::runInBackground(
        const std::string& taskName,
        const std::function<void()>& task)
{
    if (!taskName.empty()) {
        NGLOG_INFO("Dispatching %s", taskName);
    }
    mBackgroundThread.enqueueTask([=](){
        if (!taskName.empty()) {
            NGLOG_INFO("Executing %s", taskName);
        }
        try {
            task();
        } catch (const BenchmarkException& e) {
            NGLOG_ERROR("error: %s: %s", e.errorId(), e.what());
            reportError(e);
        } catch (const std::exception& e) {
            NGLOG_ERROR("error: %s: %s", BenchmarkException::INTERNAL_ERROR, e.what());
            reportError(FatalBenchmarkException(BenchmarkException::INTERNAL_ERROR, e.what()));
        }
    });
}



void BenchmarkServiceImpl::runOnMainThread(const std::function<void()>& task)
{
    mMainThread.enqueueTask(task);
    mCallback->eventReceived();
}
