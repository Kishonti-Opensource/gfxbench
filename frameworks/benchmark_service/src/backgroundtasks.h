/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "benchmarkexception.h"
#include "chartcursor.h"
#include "duelitem.h"
#include "compareresult.h"
#include "dictionary.h"
#include "listcursor.h"
#include "resultcursor.h"
#include "resultitem.h"
#include "session.h"
#include "systeminfoitem.h"
#include "testitem.h"

#include <memory>
#include <string>



class ApplicationConfig;
class Configuration;
class DataGateway;
class NetworkGateway;
class SystemInfoGateway;
class TestRunner;
class ListUpdate;
class GraphicsContext;
namespace sysinf
{
    class Properties;
}
namespace tfw {
    class Descriptor;
    class ResultGroup;
    class RuntimeInfo;
}
namespace Poco { class Timer; }



class BackgroundTaskCallback: public CursorUpdateCallback
{
public:
    virtual ~BackgroundTaskCallback() {}
    virtual void reportError(const BenchmarkException& error) = 0;
    virtual void initializationFinished(long long bytesToSync) = 0;
    virtual void synchronizationFinished() = 0;
    virtual void updateInitMessage(const std::string& message) = 0;
    virtual void updateSyncProgress(
            float progress,
            long long bytesNeeded,
            long long bytesWritten) = 0;
    virtual void loggedIn(const std::string& username) = 0;
    virtual void loggedOut() = 0;
    virtual void signedUp() = 0;
    virtual void deletedUser() = 0;
    virtual void showMessage(const std::string& message) = 0;
    virtual void showResults() = 0;
    virtual void sessionFinished() = 0;
    virtual void createContext(const TestItem& testItem, const tfw::Graphics& graphics, const tfw::Descriptor& desc) = 0;
    virtual void testLoaded() = 0;
    virtual void updateDictionary(const std::shared_ptr<Dictionary>& newDictionary) = 0;
};



class BackgroundTasks
{
public:
    BackgroundTasks(
            const ApplicationConfig& applicationConfig,
            BackgroundTaskCallback& callback,
            DataGateway& dataGateway,
            NetworkGateway& networkGateway,
            tfw::RuntimeInfo& runtimeInfo,
            SystemInfoGateway& systemInfoGateway,
            TestRunner& testRunner);
    ~BackgroundTasks();
    void stop();
    void interrupt();

    void queryTests(const std::shared_ptr<ListCursor<TestItem> >& cursor);
    void queryConfigurations(const std::shared_ptr<ListCursor<Configuration> >& cursor);
    void querySessions(const std::shared_ptr<ListCursor<Session> >& cursor);
    void queryBestResults(const std::shared_ptr<ListCursor<ResultItem> >& cursor);
    void queryResultsForSession(
            const std::shared_ptr<ListCursor<ResultItem> >& cursor,
            long long sessionId);
    void queryResultsDetails(const std::shared_ptr<ResultCursor>& cursor, long long resultRowId);
    void queryChart(
            const std::shared_ptr<ChartCursor>& cursor,
            long long resultRowId,
            long long chartId);
    void queryCompareResults(
            const std::shared_ptr<ListCursor<CompareResult> >& cursor,
            const std::string &resultId,
            const std::string &deviceFilter);
    void queryDuelResults(
            const std::shared_ptr<ListCursor<DuelItem> >& cursor,
            const std::string& apiA,
            const std::string& deviceA,
            const std::string& apiB,
            const std::string& deviceB);
    void querySystemInfo(const std::shared_ptr<ListCursor<SystemInfoItem> >& cursor);
    void querySystemInfoAttributes(
            const std::shared_ptr<ListCursor<SystemInfoAttribute> >& cursor,
            long long systemInfoRowId);
    
    const char* getChartJson(
            long long resultRowId,
            long long chartId);

    void initialize(unsigned int rebnderApiFlags, unsigned int computeApiFlags);
	void updateCursors();
    void synchronize();
    void setGroupSelection(const std::string &groupId, bool isSelected);
    void selectConfiguration(int configurationIndex);
    void setTestSelection(const std::string &testId, bool isSelected);
    void toggleAllTestSelection();
    void clearResults();
    void login(const std::string& username, const std::string& password);
    void logout();
    void deleteUser();
    void signUp(
            const std::string &email,
            const std::string &username,
            const std::string &password);
    void setCustomResolution(int width, int height);
    void setCustomBrightness(double brightness);
    void setEndlessTestRun(bool endless);
    void setHideDesktopDevices(bool enabled);
    void pollBattery();

    void startSession(bool runAll);
    void prepareNextTest();
    void loadTest(GraphicsContext* context, int width, int height);
    void runTest();
    void finishSession();
private:
    class Private;
    std::unique_ptr<Private> d;

    BackgroundTasks(const BackgroundTasks&); // No copy
    BackgroundTasks& operator=(const BackgroundTasks&); // No copy
};
