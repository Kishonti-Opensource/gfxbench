/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef BENCHMARKSERVICE_H
#define BENCHMARKSERVICE_H

#include "benchmarkservice_export.h"
#include "cursor.h"

#include "graphics/graphicscontext.h"
#include "schemas/graphics.h"
#include "schemas/descriptors.h"
#include "deviceinfo/runtimeinfo.h"

#include <memory>



class BenchmarkService;
class BenchmarkServiceCallback;
extern "C" BENCHMARK_SERVICE_EXPORT BenchmarkService* BENCHMARK_SERVICE_API createBenchmarkService(
        BenchmarkServiceCallback* callback,
        tfw::RuntimeInfo* runtimeInfo);



/**
 * Notifies the application about events in the service layer. All methods except eventReceived
 * are dispatched on the thread calling BenchmarkService::processEvents.
 * @see BenchmarkService::processEvents
 */
class BenchmarkServiceCallback
{
public:
    virtual ~BenchmarkServiceCallback() {}

    /**
     * Called when the selected changes or the dictionaries are updated (e.g. after
     * synchronization). The localized strings on the UI should be refreshed.
     * @see BenchmarkService::getLocalizedString
     */
    virtual void BENCHMARK_SERVICE_API localizationChanged() = 0;

    /**
     * Called when an error occurs during a transaction.
     * @param error exception object detailing the causes of the error.
     */
    virtual void BENCHMARK_SERVICE_API reportError(
            const char* errorId,
            const char* errorMessage,
            bool isFatal) = 0;

    /**
     * Called when a text message should be presented to the user (in a dialog).
     * @param message the unlocalized text (string key) of the message.
     */
    virtual void BENCHMARK_SERVICE_API showMessage(const char* message) = 0;

    /**
     * Called when a message about the initialization progress should be presented to the user
     * (on the splash screen).
     * @param message the unlocalized text (string key) of the message.
     */
    virtual void BENCHMARK_SERVICE_API updateInitMessage(const char* message) = 0;

    /**
     * Called when the service is waiting for user confirmation of the synchronization process
     * that involves significant network traffic.
     * @param bytesToSynchronize the amount of data waiting to be downloaded in bytes.
     */
    virtual void BENCHMARK_SERVICE_API askSynchronization(long long bytesToSynchronize) = 0;

    /**
     * Called periodically during synchronization.
     * @param progress ratio of the downloaded and total data. It is in the [0, 1] range.
     * @param bytesNeeded total amount of data being downloaded
     * @param bytesWritten amount of data already downloaded and written to storage.
     */
    virtual void BENCHMARK_SERVICE_API updateSyncProgress(
            double progress,
            long long bytesNeeded,
            long long bytesWritten) = 0;

    /**
     * Called when the service is fully loaded and ready to run tests.
     */
    virtual void BENCHMARK_SERVICE_API initializationFinished() = 0;

    /**
     * Called when a BenchmarkService::login or BenchmarkService::signUp transaction finishes
     * successfully, or when the user is automatically logged in on start up.
     * @param username name of the user that logged in.
     */
    virtual void BENCHMARK_SERVICE_API loggedIn(const char* username) = 0;

    /**
     * Called when a BenchmarkService::logout transaction finishes successfully.
     */
    virtual void BENCHMARK_SERVICE_API loggedOut() = 0;

    /**
    * Called when a BenchmarkService::deleteUser transaction finishes successfully.
    */
    virtual void BENCHMARK_SERVICE_API deletedUser() = 0;

    /**
     * Called when BenchmarkService::signUp transaction finishes successfully.
     */
    virtual void BENCHMARK_SERVICE_API signedUp() = 0;

    /**
     * Called before loading a test.The implementation must create an appropriate context.
     * BenchmarkService::loadTest should be called when the context becomes usable.
     * @param testId identifier of the test to create the context for.
     * @param loadingImage path of image to show on the loading screen.
     * @param graphics parameters for the context creations.
     * @param config of the test to get special setup requirements (eg.: brightness).
     */
    virtual void BENCHMARK_SERVICE_API createContext(
            const char* testId,
            const char* loadingImage,
            const tfw::Graphics& graphics,
            const tfw::Descriptor& config) = 0;

    /**
     * Called when a test is loaded and ready to run.The implementation must call
     * BenchService::runTest.
     */
    virtual void BENCHMARK_SERVICE_API testLoaded() = 0;

    /**
     * Called when a test session finishes and the test results should be presented to the user.
     */
    virtual void BENCHMARK_SERVICE_API showResults() = 0;

    /**
     * Called when the service generates new events that should be handled by the UI thread, and
     * BenchmarkService::handleEvents should be called on the main thread.
     * @note THIS IS NOT CALLED ON THE MAIN THREAD!
     */
    virtual void BENCHMARK_SERVICE_API eventReceived() = 0;
};



/**
 * The collection of all queries and commands related to running tests. Intended to be used
 * directly from the platform specific UI layers. All methods return instantly so they are safe
 * to call on the UI thread. The service sends messages to the application via
 * BenchmarkServiceCallback.
 * This class is designed to have a COM-like (but not true COM) interface, hence the inline
 * wrapper methods and primitive types in the signatures. This helps in cleanly compiling it into
 * a .dll and simplifies generating Java and C# wrappers for it. To understand the motivations and
 * implementation choices see https://msdn.microsoft.com/en-us/library/ms809983.aspx .
 * @see BenchmarkServiceCallback
 */
class BenchmarkService
{
public:
    enum ConfigKey {
        PRODUCT_ID,
        PRODUCT_VERSION,
        PLATFORM_ID,
        INSTALLER_NAME,
        PACKAGE_NAME,
        LOCALE,
        WINDOW_MODE_ENABLED,
        CORPORATE_VERSION,
        CONFIG_PATH,
        DATA_PATH,
        PLUGIN_PATH,
        APPDATA_PATH,
        SYNCHRONIZATION_PATH,
        ASSET_IMAGE_PATH,
        TEST_IMAGE_PATH,
        NEEDS_SYNC_BASED_ON_DATE
    };
    
    enum ResultStatus {
        NOT_AVAILABLE = 0,
        OK = 1,
        FAILED = 2,
        CANCELLED = 3
    };

    /**
     * Constructor.
     * @param callback a callback through which the application is notified about changes in the
     * service layer.
     * @param runtimeInfo source of platform specific information for the service.
     */
    static std::shared_ptr<BenchmarkService> create(
            BenchmarkServiceCallback* callback,
            tfw::RuntimeInfo* runtimeInfo)
    {
        return std::shared_ptr<BenchmarkService>(createBenchmarkService(callback, runtimeInfo),
                &BenchmarkService::release);
    }

    /**
     * Destroys the object at pointer if it is not null.
     * @param pointer
     */
    static void release(BenchmarkService* pointer) { if (pointer) pointer->destroy(); }

    /**
     * Destroys the object and frees the memory it occupies. Implicitly calls stop.
     * @see stop
     */
    virtual void BENCHMARK_SERVICE_API destroy() = 0;

    /**
     * Queries the list of tests packaged with the application.
     * @returns a cursor with the following columns: _id, title, icon, description,
     * incompatibilityText, isEnabled, isChecked, group, variantOf, variantName, isRunalone.
     */
    std::shared_ptr<Cursor> getTests() {
        return std::shared_ptr<Cursor>(getTestsPointer(), &Cursor::release);
    }

    /**
     * Queries the list OpenCL configurations on which the tests can run on.
     * @returns a cursor with the following columns: _id, title, icon, type, error, isEnabled,
     * isChecked.
     */
    std::shared_ptr<Cursor> getConfigurations() {
        return std::shared_ptr<Cursor>(getConfigurationsPointer(), &Cursor::release);
    }

    /**
     * Queries the list of past test sessions.
     * @returns a cursor with the following columns: _id, title, configuration.
     */
    std::shared_ptr<Cursor> getSessions() {
        return std::shared_ptr<Cursor>(getSessionsPointer(), &Cursor::release);
    }

    /**
     * Queries the best results of each type.
     * @returns a cursor with the following columns: _id, title, description, major, minor, icon,
     * testId, group, variantOf, variantName, status, primaryScore, primaryUnit, secondaryScore,
     * secondaryUnit.
     */
    std::shared_ptr<Cursor> getBestResults() {
        return std::shared_ptr<Cursor>(getBestResultsPointer(), &Cursor::release);
    }

    /**
     * Queries test results from the session identified by sessionId.
     * @param sessionId
     * @returns a cursor with the following columns: _id, title, description, major, minor, icon,
     * testId, group, variantOf, variantName, status, primaryScore, primaryUnit, secondaryScore,
     * secondaryUnit.
     */
    std::shared_ptr<Cursor> getResultForSession(long long sessionId) {
        return std::shared_ptr<Cursor>(getResultForSessionPointer(sessionId), &Cursor::release);
    }

    /**
     * Queries details of the result identified by its row id. The returned cursor contains two
     * types of items. If column major is not null, the item is textual. If it is null, the
     * item is a chart, and can be queried by getChart.
     * @param resultRowId
     * @returns a cursor with the following columns: _id, title, major, minor.
     * @see getChart
     */
    std::shared_ptr<Cursor> getResultDetails(long long resultRowId) {
        return std::shared_ptr<Cursor>(getResultDetailsPointer(resultRowId), &Cursor::release);
    }

    /**
     * @param resultRowId
     * @param chartRowId
     * @returns datasets for a chart. The datasets have the same number of elements. The first
     * dataset represents the domain of the measurements (x coordinates), the others represent the
     * measurements themselves (y coordinates). Columns: _id, title, data, metric, unit.
     */
    std::shared_ptr<Cursor> getChart(long long resultRowId, long long chartRowId) {
        return std::shared_ptr<Cursor>(getChartPointer(resultRowId, chartRowId), &Cursor::release);
    }

    /**
     * Queries results of other devices from the compare database.
     * @param resultId
     * @param filter return results for devices whose name contains this as a substring.
     * @returns a cursor with the following columns: _id, title, icon, api, testId, primaryScore,
     * primaryUnit, secondaryScore, secondaryUnit. The title column column contains the name of the
     * device.
     */
    std::shared_ptr<Cursor> getCompareResults(const char* resultId, const char* filter) {
        return std::shared_ptr<Cursor>(
            getCompareResultsPointer(resultId, filter), &Cursor::release);
    }

    /**
    * Queries comparisons of two devices from the compare database
    * @param api identifies the graphics or compute api (e.g. "gl" or "dx") to return comparisons
    * for.
    * @param deviceA identifies a device to return comparisons for. Pass "own" to get results
    * from the local device.
    * @param deviceB same as deviceA.
    * @returns a cursor with the following columns: _id, title, icon, unit, scoreA, scoreB. scoreA
    * and scoreB can be null, if no valid result was found.
    */
    std::shared_ptr<Cursor> getDuelResults(
            const char* apiA,
            const char* deviceA,
            const char* apiB,
            const char* deviceB)
    {
        return std::shared_ptr<Cursor>(getDuelResultsPointer(apiA, deviceA, apiB, deviceB),
                &Cursor::release);
    }

    /**
     * Queries system information. E.g.: hardware and OS details, graphics and
     * compute api information. Attributes can be retrieved synchronously as a JSON string in the
     * column "attributesJson" or asynchronously by calling getSystemInfoAttributes.
     * @see getSystemInfoAttributes
     * @returns a cursor with the following columns: _id, title, icon, major, minor, isEnabled,
     * attributesJson.
     */
    std::shared_ptr<Cursor> getSystemInfo() {
        return std::shared_ptr<Cursor>(getSystemInfoPointer(), &Cursor::release);
    }

    /**
     * Queries the list of attributes for the system info item identified by systemInfoRowId.
     * @param systemInfoRowId
     * @returns a cursor with the following columns: _id, title, value.
     */
    std::shared_ptr<Cursor> getSystemInfoAttributes(long long systemInfoRowId) {
        return std::shared_ptr<Cursor>(
                getSystemInfoAttributesPointer(systemInfoRowId), &Cursor::release);
    }

    /**
     * Queries the list of language codes for which localization is available.
     * @returns a cursor with the following columns: title.
     */
    std::shared_ptr<Cursor> getAvailableLanguages() {
        return std::shared_ptr<Cursor>(getAvailableLanguagesPointer(), &Cursor::release);
    }

    /**
     * Translates a string.
     * @see setConfig
     * @see BenchmarkServiceCallback::localizationChanged
     * @param key identifies the string to get.
     * @returns a UTF8 encoded string translated to the language selected by the LOCALE config.
     */
    std::string getLocalizedString(const char* key) {
        return getLocalizedStringPointer(key);
    }

    /**
     * @returns a configuration setting set by setConfig.
     */
    std::string getConfig(ConfigKey key) {
        return getConfigPointer(key);
    }
    
    /**
     * Gets a chart json.
     * @param resultRowId
     * @param chartRowId
     * @see getChart
     * @returns datasets for a chart. Unlike the getChart method this method returns the whole
     * dataset as a single json formatted string.
     */
    virtual std::string getChartJson(long long resultRowId, long long chartRowId) = 0;

    /**
     * Sets platform or application dependent configuration strings. All keys (except
     * CORPORATE_VERSION) should be set before calling start, and should not be changed afterwards.
     * @param key identifies the parameter to set.
     * @param value new value of the parameter.
     */
    virtual void BENCHMARK_SERVICE_API setConfig(ConfigKey key, const char* value) = 0;

    /**
     * Processes pending events. Call this on the UI thread when
     * BenchServiceCallback::processEvents is received.
     */
    virtual void BENCHMARK_SERVICE_API processEvents() = 0;

    /**
     * Starts the background thread and the initialization process. All configurations must be set
     * before calling this.
     * @see setConfig
     */
    virtual void BENCHMARK_SERVICE_API start(
		unsigned int renderApiFlags	 = (unsigned int)tfw::ApiDefinition::NOT_DEFINED, 
		unsigned int computeApiFlags = (unsigned int)tfw::ApiDefinition::NOT_DEFINED) = 0;

    /**
     * Waits for running tasks to finish and stops the background thread. Might take a long time
     * to finish.
     */
    virtual void BENCHMARK_SERVICE_API stop() = 0;

    /**
     * Updates the selection state of a test group and all tests in it. If the group contains a
     * run-alone test and isSelected is true, all other tests and groups are deselected.
     * Updates the affected cursors.
     * @param groupId identifier of the group to select/deselect
     * @param isSelected true to select the group, false to deselect it.
     */
    virtual void BENCHMARK_SERVICE_API setGroupSelection(const char* groupId, bool isSelected) = 0;

    /**
     * Updates the selection state of a test. If the test is a run-alone test and isSelected is
     * true, all other tests are deselected. Group selections are automatically updated to reflect
     * changes in the test selections. Updates affected cursors.
     * @param groupId identifier of the group to select/deselect
     * @param isSelected true to select the group, false to deselect it.
     */
    virtual void BENCHMARK_SERVICE_API setTestSelection(const char* testId, bool isSelected) = 0;

    /**
     * If there are unselected non-run-alone tests, selects them. Otherwise deselects all tests.
     * Run-alone tests are always deselected. Updates affected cursors.
     */
    virtual void BENCHMARK_SERVICE_API toggleAllTestSelection() = 0;

    /**
     * Selects an OpenCL device configuration to use in the OpenCL benchmarks.
     * @param configurationIndex index of the configuration to select in the last
     * ListUpdate::configurations.
     */
    virtual void BENCHMARK_SERVICE_API selectConfiguration(int configurationIndex) = 0;

    /**
     * Sets the virtual resolution of benchmarks. Pass negative values to run tests in native
     * resolution. Has no effect in community builds.
     * @param width horizontal resolution in pixels.
     * @param height vertical resolution in pixels.
     */
    virtual void BENCHMARK_SERVICE_API setCustomResolution(int width, int height) = 0;

    /**
     * Sets the birghtness of the screen during benchmarking. Pass negative values to turn off.
     * Valid brightness values are between [0, 1].
     * @param brightness Brightness percent of the screen scaled between [0, 1].
     */
    virtual void BENCHMARK_SERVICE_API setCustomBrightness(double brightness) = 0;
    
    /**
     * Sets the test running to endless mode which results in no interruption during testing
     * and running until the app is exited or the battery is drained.
     * @param endless True or false.
     */
    virtual void BENCHMARK_SERVICE_API setEndlessTestRun(bool endless) = 0;

    /**
     * Starts a session running all available tests except the run-alone ones.
     * @see BenchmarkServiceCallback::createContext
     */
    virtual void BENCHMARK_SERVICE_API runAllTests() = 0;

    /**
     * Starts a session running selected tests.
     * @see BenchmarkServiceCallback::createContext
     */
    virtual void BENCHMARK_SERVICE_API runSelectedTests() = 0;

    /**
     * Initializes the next test to run. Call this once after the appropriate context is created
     * for the test.
     * @see BenchmarkServiceCallback::createContext
     * @see BenchmarkServiceCallback::testLoaded
     * @param graphicsContext context for running the test. Must be readily usable. (Window is
     * exposed, etc.)
     * @param width horizontal size of the window showing the test in pixels.
     * @param height vertical size of the window showing the test in pixels.
     */
    virtual void BENCHMARK_SERVICE_API loadTest(
            GraphicsContext* graphicsContext,
            int width,
            int height) = 0;

    /**
     * Starts the previously loaded test. Call this once after receiving a testLoaded callback.
     * @see BenchmarkServiceCallback::testLoaded
     */
    virtual void BENCHMARK_SERVICE_API runTest() = 0;

    /**
     * Cancels the currently running test and finishes the session.
     */
    virtual void BENCHMARK_SERVICE_API stopTests() = 0;

    /**
     * Cancels the currently running test and starts the next one in the session. Finishes the
     * session if the current test is the last one.
     */
    virtual void BENCHMARK_SERVICE_API skipTest() = 0;

    /**
     * Allow synchronization. Has an effect only after receiving
     * BenchmarkServiceCallback::askSynchronization. Call stop to reject the request and exit.
     */
    virtual void BENCHMARK_SERVICE_API acceptSynchronization() = 0;

    /**
     * Deletes all results and sessions from the local database. Updates cursors accordingly.
     */
    virtual void BENCHMARK_SERVICE_API clearResults() = 0;

    /**
     * Logs in to the community server. The user stays logged in even if the application exits
     * unless logout is called or setLogoutOnClose is set to true. Has no effect in corporate
     * builds.
     * @see BenchmarkServiceCallback::loggedIn
     * @param username
     * @param password
     */
    virtual void BENCHMARK_SERVICE_API login(const char* username, const char* password) = 0;

    /**
     * Logs out of the community server. Has no effect in corporate builds.
     * @see BenchmarkServiceCallback::loggedOut
     */
    virtual void BENCHMARK_SERVICE_API logout() = 0;


    /**
    * Delete user account of the community server. Has no effect in corporate builds.
    * @see BenchmarkServiceCallback::deletedUser
    */
    virtual void BENCHMARK_SERVICE_API deleteUser() = 0;

    /**
     * Registers a user to the community server. Implicitly logs in if the registration is
     * successful. Has no effect in corporate builds.
     * @see login
     * @see BenchmarkServiceCallback::signUp
     * @see BenchmarkServiceCallback::loggedIn
     * @param email
     * @param username
     * @param password
     */
    virtual void BENCHMARK_SERVICE_API signUp(
            const char* email,
            const char* username,
            const char* password) = 0;

    /**
     * Set to true to log the out when calling stop.
     * @param enabled true to logout when stopping, false to stay logged in.
     */
    virtual void BENCHMARK_SERVICE_API setLogoutOnClose(bool enabled) = 0;

    /**
     * Set to true to filter desktop devices in getCompareResults.
     * @param enabled true to filter desktop devices, false to return all results.
     */
    virtual void BENCHMARK_SERVICE_API setHideDesktopDevices(bool enabled) = 0;
protected:
    virtual ~BenchmarkService() {}
    virtual Cursor* BENCHMARK_SERVICE_API getTestsPointer() = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getConfigurationsPointer() = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getSessionsPointer() = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getBestResultsPointer() = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getResultForSessionPointer(long long sessionId) = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getResultDetailsPointer(long long resultRowId) = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getChartPointer(
            long long resultRowId,
            long long chartRowId) = 0;
    virtual const char* BENCHMARK_SERVICE_API getChartJsonString(
            long long resultRowId,
            long long chartRowId) = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getCompareResultsPointer(
            const char* resultId,
            const char* filter) = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getDuelResultsPointer(
            const char* apiA,
            const char* deviceA,
            const char* apiB,
            const char* deviceB) = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getSystemInfoPointer() = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getSystemInfoAttributesPointer(
            long long systemInfoRowID) = 0;
    virtual Cursor* BENCHMARK_SERVICE_API getAvailableLanguagesPointer() = 0;
    virtual const char* BENCHMARK_SERVICE_API getLocalizedStringPointer(const char* key) = 0;
    virtual const char* BENCHMARK_SERVICE_API getConfigPointer(ConfigKey key) = 0;
};



#endif
