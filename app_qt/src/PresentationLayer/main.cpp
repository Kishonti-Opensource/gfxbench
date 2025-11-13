/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "Dictionary.h"
#include "MainWindow.h"
#include "ApiSelector.h"
#include "MessageBox.h"

#include "benchmarkservice.h"
#include "deviceinfo/platformruntimeinfo.h"
#include "ng/log.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QProxyStyle>
#include <QSharedMemory>
#include <QStandardPaths>

#include <cstdlib>
#include <memory>
#include <QSettings>
#include <QDateTime>



extern const char* const organizationName;
extern const char* const productName;
extern const char* const productId;
extern const char* const productVersion;
extern const char* const crashReporterToken;
extern const char* const contactDomain;
extern const char* const packageName;



class QFileSink: public ng::log::Sink
{
public:
    void open(const QString &path) {
        file.setFileName(path);
        file.open(QFile::WriteOnly | QFile::Truncate);
    }
    void doLog(const ng::log::Message& lm) {
        QByteArray message = lm.full();
        message += '\n';
        file.write(message);
        file.flush();
    }
private:
    QFile file;
};



class CustomLogger: public ng::log::Logger
{
public:
    CustomLogger() {
#ifdef NDEBUG
        mConsoleSink.setMinSeverity(ng::log::Severity::info);
        mFileSink.setMinSeverity(ng::log::Severity::info);
#else
        mConsoleSink.setMinSeverity(ng::log::Severity::any);
        mFileSink.setMinSeverity(ng::log::Severity::any);
    #ifdef Q_OS_WIN
        addSink(&mWindowsSink);
    #endif
#endif
        addSink(&mConsoleSink);
    }
    void openFileSink(const QString &path) {
        mFileSink.open(path);
        addSink(&mFileSink);
    }
private:
    ng::LogSinkPrintf mConsoleSink;
    QFileSink mFileSink;
#if defined(Q_OS_WIN) && !defined(NDEBUG)
    ng::LogSinkWindowsDebug mWindowsSink;
#endif
};



/**
 * No orange highlights on Ubuntu.
 */
class CustomStyle: public QProxyStyle
{
public:
    void drawPrimitive(
            PrimitiveElement element,
            const QStyleOption* option,
            QPainter* painter,
            const QWidget* widget) const
    {
        if (element == QStyle::PE_FrameFocusRect)
            return;
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
};



void messageHandler(QtMsgType type, const QMessageLogContext&, const QString &msg)
{
    /*
     * Workaround for crash when closing terminal on windows:
     * A probable bug in Qt triggers a warning message in QTimer's destructor when the application
     * console is closed. The logging then triggers a crash in Microsoft's mutex implentation.
     * We simply swallow the problematic warning message to avoid the crash.
     */
    if (msg.contains("cannot be stopped from another thread")) {
        return;
    }

    switch (type) {
    case QtDebugMsg:
        NGLOG_DEBUG("%s", msg.toStdString());
        break;
    case QtWarningMsg:
        NGLOG_WARN("%s", msg.toStdString());
        break;
    case QtCriticalMsg:
        NGLOG_ERROR("%s", msg.toStdString());
        break;
    case QtFatalMsg:
        NGLOG_ERROR("%s", msg.toStdString());
        break;
    default:
        break;
    }
}



void initializeQt()
{
    QApplication::setOrganizationName(::organizationName);
    QApplication::setOrganizationDomain(::contactDomain);
    QApplication::setApplicationName(::productName);
    QApplication::setApplicationVersion(::productVersion);
    qRegisterMetaType<std::string>("std::string");
    qInstallMessageHandler(messageHandler);
    qApp->setStyle(new CustomStyle);
}


void copyPath(const QString& src, const QString& dst)
{
    NGLOG_INFO("Extract files from %s to %s", src.toStdString(), dst.toStdString());
    QDir dir(src);
    if (! dir.exists()) {
        return;
     }
    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dst + QDir::separator() + d;
        dir.mkpath(dst_path);
        copyPath(src + QDir::separator() + d, dst_path);
    }
    
    foreach (QString f, dir.entryList(QDir::Files)) {
        QFile si(src + QDir::separator() + f);
        QFile di(dst + QDir::separator() + f);
        if(si.size() != di.size()) {
            QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
        }
    }
}

void setConfigs(int argc, char *argv[], BenchmarkService& service)
{
    QStringList arguments;
    for (int i = 0; i < argc; ++i) {
        QString arg = QString(argv[i]);
        if(arg.startsWith("-NS") || arg.startsWith("YES") ||arg.startsWith("NO")) {
           continue;
        }
        arguments.append(argv[i]);
    }

    QCommandLineParser parser;
    QCommandLineOption basePathOption(QStringList() << "b" << "basepath");
    basePathOption.setValueName("Base path");
    parser.addOption(basePathOption);
    QCommandLineOption windowOption(QStringList() << "w" << "window");
    parser.addOption(windowOption);
    parser.parse(arguments);

    service.setConfig(BenchmarkService::PRODUCT_ID, ::productId);
    service.setConfig(BenchmarkService::PRODUCT_VERSION, ::productVersion);
    service.setConfig(BenchmarkService::PACKAGE_NAME, ::packageName);
    service.setConfig(BenchmarkService::LOCALE, QLocale::system().name().toUtf8());
#if defined(Q_OS_WIN)
    service.setConfig(BenchmarkService::PLATFORM_ID, "windows");
    service.setConfig(BenchmarkService::INSTALLER_NAME, "com.microsoft.windowsstore");
#elif defined(Q_OS_OSX)
    service.setConfig(BenchmarkService::PLATFORM_ID, "macosx");
    service.setConfig(BenchmarkService::INSTALLER_NAME, "com.apple.macappstore");
#elif defined(Q_OS_LINUX)
    service.setConfig(BenchmarkService::PLATFORM_ID, "linux");
    service.setConfig(BenchmarkService::INSTALLER_NAME, "linux distribution");
#else
    service.setConfig(BenchmarkService::PLATFORM_ID, QGuiApplication::platformName().toUtf8());
    service.setConfig(BenchmarkService::INSTALLER_NAME, "unknown");
#endif

    QString basePath = QCoreApplication::applicationDirPath();
    if (QCoreApplication::applicationDirPath().endsWith("/bin")) {
        basePath += "/..";
    }
    if (QCoreApplication::applicationDirPath().endsWith("MacOS")) {
        basePath += "/../Resources";
    }
    if (parser.isSet(basePathOption)) {
        basePath = QDir::cleanPath(parser.value(basePathOption));
    }

    
#ifdef Q_OS_OSX
#define IS_COMMUNITY 1 // Forced to set up path properly for corporate version.
#endif
    

#if IS_COMMUNITY
    service.setConfig(BenchmarkService::NEEDS_SYNC_BASED_ON_DATE, "true");

#ifdef Q_OS_OSX
    copyPath(basePath, (QDir::fromNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/synchronized").toUtf8()));
#endif //Q_OS_OSX
    service.setConfig(BenchmarkService::APPDATA_PATH,
            QDir::fromNativeSeparators(QStandardPaths::writableLocation(
            QStandardPaths::DataLocation)).toUtf8());
    
    service.setConfig(BenchmarkService::SYNCHRONIZATION_PATH,
            (QDir::fromNativeSeparators(QStandardPaths::writableLocation(
            QStandardPaths::CacheLocation) + "/synchronized").toUtf8()));
#ifdef Q_OS_OSX
    service.setConfig(BenchmarkService::DATA_PATH,
            (QDir::fromNativeSeparators(
            QCoreApplication::applicationDirPath()+"/../resources/data" )).toUtf8());
#else
    service.setConfig(BenchmarkService::DATA_PATH,
        (QDir::fromNativeSeparators(QStandardPaths::writableLocation(
            QStandardPaths::CacheLocation) + "/synchronized/data").toUtf8()));
#endif

    service.setConfig(BenchmarkService::WINDOW_MODE_ENABLED, "false");
#else //IS_COMMUNITY
    service.setConfig(BenchmarkService::APPDATA_PATH, basePath.toUtf8());
    
    service.setConfig(BenchmarkService::SYNCHRONIZATION_PATH,
            (basePath  + "/synchronized").toUtf8());
    
    service.setConfig(BenchmarkService::DATA_PATH, (basePath + "/data").toUtf8());
    
    service.setConfig(BenchmarkService::WINDOW_MODE_ENABLED,
            parser.isSet(windowOption) ? "true" : "false");
#endif //IS_COMMUNITY
#ifdef Q_OS_OSX
    service.setConfig(BenchmarkService::APPDATA_PATH,
            QDir::fromNativeSeparators(QStandardPaths::writableLocation(
            QStandardPaths::DataLocation)).toUtf8());
#endif
    service.setConfig(BenchmarkService::CONFIG_PATH, (basePath + "/config").toUtf8());
    service.setConfig(BenchmarkService::PLUGIN_PATH, (basePath + "/plugins").toUtf8());
    service.setConfig(BenchmarkService::ASSET_IMAGE_PATH, ":");
    service.setConfig(BenchmarkService::TEST_IMAGE_PATH, (basePath + "/images").toUtf8());
}


int main(int argc, char *argv[])
{
    // This is intentionally leaked to avoid destruction order issues during exit.
    CustomLogger* logger = new CustomLogger;
    ng::Logger::setTheGlobal(logger);

    int resultCode;
    try {
        QApplication application(argc, argv);
        initializeQt();
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

        /* Handle decimal points in ngrtl correctly. */
        setlocale(LC_NUMERIC, "C");

        QtUI::Dictionary::instance()->load(":/localization"); // Load defaults
        QtUI::Dictionary::instance()->load(QStandardPaths::writableLocation(
                QStandardPaths::DataLocation) + "/synchronized/localization");
        NGLOG_INFO("Dictionary loaded");
/*
        QSharedMemory instanceGuard(::productId);
        if (!instanceGuard.create(1) && (instanceGuard.error() == QSharedMemory::AlreadyExists)) {
            QtUI::popupMsgBox("AlreadyRunningTitle", QtUI::dict("AlreadyRunningBody").arg(
                    ::productName), "Ok");
            return EXIT_FAILURE; // A process is already running 
        }
*/

        QtUI::MainWindow mainWindow;
        tfw::PlatformRuntimeInfo runtimeInfo;
        std::shared_ptr<BenchmarkService> benchmarkService =
                BenchmarkService::create(&mainWindow, &runtimeInfo);
        setConfigs(argc, argv, *benchmarkService);
        logger->openFileSink(QString::fromStdString(benchmarkService->getConfig(
                BenchmarkService::APPDATA_PATH) + "/" + ::productId + ".log"));
        NGLOG_INFO("Application started");

		unsigned int renderApiFlags = tfw::ApiDefinition::NOT_DEFINED;
		unsigned int computeApiFlags= tfw::ApiDefinition::NOT_DEFINED;

#if defined(GFXBENCH_GL)
		renderApiFlags = tfw::ApiDefinition::GL;
#elif defined(GFXBENCH_DX)
		renderApiFlags = tfw::ApiDefinition::DX | tfw::ApiDefinition::DX12;
#elif defined(GFXBENCH_VULKAN)
		renderApiFlags = tfw::ApiDefinition::VULKAN | tfw::ApiDefinition::GL;
#elif defined(GFXBENCH_METAL)
		renderApiFlags = tfw::ApiDefinition::METAL;
#elif defined(COMPUBENCH_CL)
		renderApiFlags = tfw::ApiDefinition::GL;
		computeApiFlags = tfw::ApiDefinition::CL;
#elif defined(COMPUBENCH_CU)
		renderApiFlags = tfw::ApiDefinition::GL;
		computeApiFlags = tfw::ApiDefinition::CUDA;
#elif defined(COMPUBENCH_METAL)
		renderApiFlags = tfw::ApiDefinition::METAL;
#endif
        benchmarkService->start(renderApiFlags,computeApiFlags);

        mainWindow.setBenchmarkService(benchmarkService.get());
        mainWindow.setWindowTitle(::productName);

        mainWindow.showSplash();
        resultCode = application.exec();
    } catch (const std::exception& e) {
        NGLOG_ERROR("%s", e.what());
        resultCode = EXIT_FAILURE;
    }
    return resultCode;
}
