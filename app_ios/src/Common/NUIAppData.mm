/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUIAppData.m
//  GFXBench
//
//  Created by Kishonti Kft on 14/02/2014.
//
//

#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUIUtilities.h"

#import "Utils/NSFileManagerLocations.h"
#import "Utils/UIResponder_KeyboardCacher.h"

#include <fstream>
#include <Poco/File.h>
#include <Poco/Path.h>

@implementation APPModalMessageAction

+ (instancetype)actionWithTitle:(NSString *)title style:(UIAlertActionStyle)style handler:(void (^)(APPModalMessageAction *action))handler {
    APPModalMessageAction *ret = [[APPModalMessageAction alloc] init];
    [ret setTitle:title];
    [ret setStyle:style];
    ret.handler = handler;
    
    return ret;
}

@end

@implementation APPModalMessageObject

+ (instancetype) messageWithTitle:(NSString *)title message:(NSString *)message isFatal:(BOOL)isFatal {
    
    APPModalMessageObject *mObject = [[APPModalMessageObject alloc] init];
    [mObject setTitle:title];
    [mObject setMessage:message];
    [mObject setIsFatal:isFatal];
    
    return mObject;
}

- (void)addChoice:(APPModalMessageAction *)action {
    NSMutableArray *a = [[NSMutableArray alloc] initWithArray:self.choices];
    [a addObject:action];
    self.choices = a;
}

@end

namespace {
    class IosBenchmarkServiceCallback : public BenchmarkServiceCallback {
        
        void BENCHMARK_SERVICE_API localizationChanged() {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_LocalizationChanged
                                                                object:[NUIAppData sharedNUIAppData]];
        }
        
        void BENCHMARK_SERVICE_API reportError(
                                               const char* errorId,
                                               const char* errorMessage,
                                               bool isFatal) {
            APPModalMessageObject *messageObject = [APPModalMessageObject messageWithTitle:[NSString stringWithUTF8String:errorId] message:[NSString stringWithUTF8String:errorMessage] isFatal:isFatal];
            
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowModalMessage
                                                                object:[NUIAppData sharedNUIAppData]
                                                              userInfo:@{
                                                                         @"messageObj":messageObject
                                                                         }
             ];
        }
        
        void BENCHMARK_SERVICE_API showMessage(const char* message) {
            APPModalMessageObject *messageObject = [APPModalMessageObject messageWithTitle:@"NetRequirementDialogTitle" message:[NSString stringWithUTF8String:message] isFatal:false];
            
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowModalMessage
                                                                object:[NUIAppData sharedNUIAppData]
                                                              userInfo:@{
                                                                         @"messageObj":messageObject
                                                                         }
             ];
        }
        
        void BENCHMARK_SERVICE_API updateInitMessage(const char* message) {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_UpdateInitMessage
                                                                object:[NUIAppData sharedNUIAppData]
                                                              userInfo:@{
                                                                         @"message":[NSString stringWithUTF8String:message]
                                                                         }
             ];
            
        }
        
        void BENCHMARK_SERVICE_API askSynchronization(long long bytesToSynchronize) {
            NSString *message = [NSString stringWithFormat:[NUIAppData getLocalized:@"SyncLotDialogBody"], [[NSString stringWithFormat:@"%@ MB", [[NSNumber numberWithLongLong:bytesToSynchronize/1024/1024] stringValue]] UTF8String]];
            NSString *syncTitle = [NUIAppData getLocalized:@"SyncLotDialogTitle"];
            
            
            APPModalMessageObject *messageObject = [APPModalMessageObject messageWithTitle:syncTitle message:message isFatal:false];
            [messageObject addChoice:[APPModalMessageAction actionWithTitle:@"Ok" style:UIAlertActionStyleDefault handler:^(APPModalMessageAction *action) {
                [NUIAppData getService]->acceptSynchronization();
                [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowModalMessage object:[NUIAppData sharedNUIAppData]];
                [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_LoadingForeverStopped
                                                                    object:[NUIAppData sharedNUIAppData]];
            }]];
            [messageObject addChoice:[APPModalMessageAction actionWithTitle:@"No" style:UIAlertActionStyleDefault handler:^(APPModalMessageAction *action) {
                [[NUIAppData sharedNUIAppData] Quit];
            }]];
            
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowModalMessage
                                                                object:[NUIAppData sharedNUIAppData]
                                                              userInfo:@{
                                                                         @"messageObj":messageObject
                                                                         }
             ];
        }
        
        void BENCHMARK_SERVICE_API updateSyncProgress(
                                                      double progress,
                                                      long long bytesNeeded,
                                                      long long bytesWritten) {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_UpdateSyncProgress
                                                                object:[NUIAppData sharedNUIAppData]
                                                              userInfo:@{
                                                                         @"progress":[NSNumber numberWithDouble:progress],
                                                                         @"bytesNeeded":[NSNumber numberWithLongLong:bytesNeeded],
                                                                         @"bytesWritten":[NSNumber numberWithLongLong:bytesWritten],
                                                                         }
             ];
        }
        
        void BENCHMARK_SERVICE_API initializationFinished() {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_InitializationFinished
                                                                object:[NUIAppData sharedNUIAppData]];
            
            [[NSUserDefaults standardUserDefaults] setObject:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"] forKey:@"LastVersionSync"];
            [[NSUserDefaults standardUserDefaults] synchronize];
            //[NUIAppData addSkipBackupToDirectory:[NSURL fileURLWithPath:[NUIAppData SyncPath] isDirectory:YES]];
        }
        
        void BENCHMARK_SERVICE_API loggedIn(const char* username) {
            [NUIAppData sharedNUIAppData].loginName = [NSString stringWithUTF8String:username];
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_LoggedIn
                                                                object:[NUIAppData sharedNUIAppData]
                                                              userInfo:@{
                                                                         @"username":[NSString stringWithUTF8String:username]
                                                                         }
             ];
        }

        void BENCHMARK_SERVICE_API deletedUser() {
            [NUIAppData sharedNUIAppData].loginName = nil;
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_DeletedUser
                                                                object:[NUIAppData sharedNUIAppData]];
        }
        
        void BENCHMARK_SERVICE_API loggedOut() {
            [NUIAppData sharedNUIAppData].loginName = nil;
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_LoggedOut
                                                                object:[NUIAppData sharedNUIAppData]];
        }
        
        void BENCHMARK_SERVICE_API signedUp() {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_SignedUp
                                                                object:[NUIAppData sharedNUIAppData]];
        }
        
        void BENCHMARK_SERVICE_API createContext(
                                                 const char* testId,
                                                 const char* loadingImage,
                                                 const tfw::Graphics& graphics,
                                                 const tfw::Descriptor& desc) {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CreateContext
                                                                object:[NUIAppData sharedNUIAppData]
                                                              userInfo:@{
                                                                         @"testId":[NSString stringWithUTF8String:testId],
                                                                         @"loadingImage":[NSString stringWithUTF8String:loadingImage],
                                                                         @"graphics":[NSValue valueWithPointer:&graphics],
                                                                         @"desc":[NSValue valueWithPointer:&desc]
                                                                         }
             ];
        }
        
        void BENCHMARK_SERVICE_API testLoaded() {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_TestLoaded
                                                                object:[NUIAppData sharedNUIAppData]];
        }
        
        void BENCHMARK_SERVICE_API showResults() {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowResults
                                                                object:[NUIAppData sharedNUIAppData]];
        }
        
        void BENCHMARK_SERVICE_API eventReceived() {
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_BenchmarkServiceEventRecieved
                                                                object:[NUIAppData sharedNUIAppData]];
        }
        
    };
}


@interface NUIAppData()

@property (assign, nonatomic) BenchmarkService *service;
@property (assign, nonatomic) tfw::RuntimeInfo *runtimeInfo;
@property (assign, nonatomic) IosBenchmarkServiceCallback *callback;

@property (strong, nonatomic) NUIReachability *reachability;

@property (strong, nonatomic) NSMutableArray* alerts;

@property (assign, nonatomic) BOOL needsLatest;

@end


@implementation NUIAppData

#pragma mark - static getters

+ (BenchmarkService *) getService {
    if([NUIAppData sharedNUIAppData].service == nil) {
        [NUIAppData sharedNUIAppData].service = createBenchmarkService([NUIAppData getCallback], [NUIAppData getRuntimeInfo]);
    }
    return [NUIAppData sharedNUIAppData].service;
}

+ (tfw::RuntimeInfo *) getRuntimeInfo {
    if([NUIAppData sharedNUIAppData].runtimeInfo == nil) {
        [NUIAppData sharedNUIAppData].runtimeInfo = new tfw::PlatformRuntimeInfo();
    }
    return [NUIAppData sharedNUIAppData].runtimeInfo;
}

+ (BenchmarkServiceCallback *) getCallback {
    if([NUIAppData sharedNUIAppData].callback == nil) {
        [NUIAppData sharedNUIAppData].callback = new IosBenchmarkServiceCallback();
    }
    return [NUIAppData sharedNUIAppData].callback;
}

+ (NSString *)ReadDataPath
{
    return [[NSBundle mainBundle] resourcePath];
}

+ (NSString *)RwDataPath
{
#if !IS_CORPORATE
    return [[NSFileManager defaultManager] applicationSupportDirectory];
#else
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if([paths count] > 0)
        return [paths objectAtIndex:0];
    else
        return [[NSFileManager defaultManager] applicationSupportDirectory];
#endif
}

+ (NSString *)SyncPath
{
    return [self ReadDataPath];
}

+ (void)addSkipBackupToDirectory:(NSURL *)URL
{
    // Create a local file manager instance
    NSFileManager *localFileManager=[[NSFileManager alloc] init];
    
    // Enumerate the directory (specified elsewhere in your code)
    // Request the two properties the method uses, name and isDirectory
    NSDirectoryEnumerator *dirEnumerator = [localFileManager enumeratorAtURL:URL
                                                  includingPropertiesForKeys:[NSArray arrayWithObjects:NSURLNameKey,NSURLIsDirectoryKey,nil]
                                                                     options:0
                                                                errorHandler:nil];
    
    // Enumerate the dirEnumerator results, each value is stored in allURLs
    for (NSURL *theURL in dirEnumerator) {
        
        // Retrieve the file name. From NSURLNameKey, cached during the enumeration.
        NSString *fileName;
        [theURL getResourceValue:&fileName forKey:NSURLNameKey error:NULL];
        
        // Retrieve whether a directory. From NSURLIsDirectoryKey, also
        // cached during the enumeration.
        NSNumber *isDirectory;
        [theURL getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:NULL];
        
        // addSkipBackupAttribute for each file
        if ([isDirectory boolValue]==NO) {
            [NUIAppData addSkipBackupAttributeToItemAtURL:theURL];
        }
    }
}

+ (BOOL)addSkipBackupAttributeToItemAtURL:(NSURL *)URL
{
    assert([[NSFileManager defaultManager] fileExistsAtPath: [URL path]]);
    
    NSError *error = nil;
    BOOL success = [URL setResourceValue: [NSNumber numberWithBool: YES]
                                  forKey: NSURLIsExcludedFromBackupKey error: &error];
    if(!success){
        NSLog(@"Error excluding %@ from backup %@", [URL lastPathComponent], error);
    }
    return success;
}

+ (NSString *)ConfigPath
{
#if IS_CORPORATE
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *documentsPath = [[[fileManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject] path];
    NSString *testDataPath = [NSString stringWithFormat:@"%@/config", documentsPath];
    BOOL isDir = NO;
    BOOL exists = [fileManager fileExistsAtPath:testDataPath isDirectory:&isDir];
    if (exists && isDir)
    {
        return testDataPath;
    }
    else
    {
        return [NSString stringWithFormat:@"%@/config", [NSBundle mainBundle].bundlePath];
    }
#else
    return [NSString stringWithFormat:@"%@/config", [NSBundle mainBundle].bundlePath];
#endif
}

+ (NSString *)DataPathForTest:(const tfw::Descriptor *)desc
{
	const std::string &prefix = desc->dataPrefix();
#if IS_CORPORATE
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *documentsPath = [[[fileManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject] path];
	NSString *testDataPath = [NSString stringWithFormat:@"%@/data/%s", documentsPath, prefix.c_str()];
	BOOL isDir = NO;
	BOOL exists = [fileManager fileExistsAtPath:testDataPath isDirectory:&isDir];
	if (exists && isDir)
	{
		return testDataPath;
	}
	else
	{
		return [NSString stringWithFormat:@"%@/data/%s", [NSBundle mainBundle].bundlePath, prefix.c_str()];
	}
#else
    return [NSString stringWithFormat:@"%@/syncronized/data/%s", [[NSFileManager defaultManager] applicationSupportDirectory], prefix.c_str()];
#endif
}

+ (NSString *)DataPath
{
#if IS_CORPORATE
    return [NSString stringWithFormat:@"%@/data", [NSBundle mainBundle].bundlePath];
#else
    return [[self SyncPath] stringByAppendingPathComponent:@"data"];
#endif
}

+ (NSString *)ImagePath
{
#if IS_CORPORATE
    return [NSString stringWithFormat:@"%@/app_data/image", [[NSBundle mainBundle] resourcePath]];
#else
    return [NSString stringWithFormat:@"%@/syncronized/image", [[NSFileManager defaultManager] applicationSupportDirectory]];
#endif
}

//TODO get information from descriptor or testlist instead
+ (NSString *)GraphicsVersionStringFromMajor:(int)major Minor:(int)minor TestId:(NSString *)tid
{
    if([tid rangeOfString:@"metal"].location != NSNotFound)
    {
        return [NSString stringWithFormat:@"METAL %d.%d", major, minor];
    }
    else
    {
        return [NSString stringWithFormat:@"OpenGL ES %d.%d", major, minor];
    }
}

+ (NSDictionary *)getObscuredVersion {
    NSString *version = [[NSBundle mainBundle] objectForInfoDictionaryKey:(NSString*) @"BUIVersion"];
    NSArray *bits = [version componentsSeparatedByString:@"."];
    //            NSNumber *VersionMajor = [NSNumber numberWithInteger:[bits[0] integerValue]];
    NSNumber *VersionMinor = [NSNumber numberWithInteger:[bits[1] integerValue]];
    NSNumber *VersionPatch = [NSNumber numberWithInteger:MAX([bits[2] integerValue], 38)];
    
    return @{@"Major":[NSNumber numberWithInt:3], @"Minor":VersionMinor, @"Patch":VersionPatch};
}

+ (NSString *) getLocalized:(NSString *)s {
    return [NSString stringWithUTF8String:[NUIAppData getService]->getLocalizedString([s UTF8String]).c_str()];
}


+ (void)SetNeedsLatestResults:(BOOL)b {
    [NUIAppData sharedNUIAppData].needsLatest = b;
}

+ (BOOL)NeedsLatestResults {
    return [NUIAppData sharedNUIAppData].needsLatest;
}



#pragma mark - BenchmarkService

- (void) callEventOnMainThread {
    dispatch_async(dispatch_get_main_queue(),
                   ^{
                       [NUIAppData getService]->processEvents();
                   });
}

- (void) initFinished {

    [UIApplication sharedApplication].idleTimerDisabled = NO;

    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_HideLoading object:self];
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setObject:[NSDate dateWithTimeIntervalSinceNow:0] forKey:@"LastSuccessfulSync"];
    [userDefaults synchronize];
}

- (void)addNewAlert:(UIAlertController *)alert {
    if(self.alerts == nil) {
        self.alerts = [[NSMutableArray alloc] init];
    }
    
    [self.alerts addObject:alert];
    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowModalMessage
                                                        object:[NUIAppData sharedNUIAppData]
     ];
}

- (UIAlertController *)getAlert {
    if(self.alerts == nil) return nil;
    if(self.alerts.count <= 0) return nil;
    
    UIAlertController *alert = [self.alerts firstObject];
    [self.alerts removeObject:alert];
    
    return alert;
}


#pragma mark - Lifetime functions
- (void)Quit
{
    exit(0);
}

- (void)Startup
{
    dispatch_async(dispatch_get_main_queue(),
                   ^{
                       [UIApplication sharedApplication].idleTimerDisabled = YES;
                   });

    [UIResponder cacheKeyboard:YES];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowLoadingScreen object:self];
    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_LoadingForeverStarted
                                                        object:self];
    
    NSString *preferredLocale = [[NSUserDefaults standardUserDefaults] stringForKey:@"Locale"];
    if(preferredLocale == nil || preferredLocale.length == 0)
        preferredLocale = [NSLocale preferredLanguages].firstObject;
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    BOOL showDesktop = [userDefaults boolForKey:@"ShowDesktop"];
    
    
    BOOL needsSync = true;
    NSString *currentVersion = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    NSString *previousVersion = [userDefaults stringForKey:@"LastVersionSync"];
    if(previousVersion == nil) {
        previousVersion = @"";
    }
    NSDate *savedSyncDate = [userDefaults objectForKey:@"LastSuccessfulSync"];
    NSDate *lastSync = [NSDate distantPast];
    NSDate *now = [NSDate dateWithTimeIntervalSinceNow:0];
    if(savedSyncDate != nil) {
        lastSync = savedSyncDate;
    }
    
    NSCalendar *calendar = [NSCalendar currentCalendar];
    
    [calendar rangeOfUnit:NSCalendarUnitDay startDate:&now
                 interval:NULL forDate:now];
    [calendar rangeOfUnit:NSCalendarUnitDay startDate:&lastSync
                 interval:NULL forDate:lastSync];
    
    NSDateComponents *difference = [calendar components:NSCalendarUnitDay
                                               fromDate:lastSync toDate:now options:0];
    if(difference.day <= 6 && [previousVersion isEqualToString:currentVersion]) {
        needsSync = false;
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_LoadingForeverStarted
                                                        object:self];
#define UNCOMPRESS_APP_DATA 0
#if UNCOMPRESS_APP_DATA
    {
    NSBundle *mainBundle = [NSBundle mainBundle];
    std::string fn = "app_data/dat_files.txt";
    NSString *source_file_name = [mainBundle pathForResource:[NSString stringWithCString:fn.c_str()
                                                                            encoding:[NSString defaultCStringEncoding]]
                                                  ofType:nil];
    std::string source_file_name_std = std::string([source_file_name UTF8String]);
    

    std::string line;
    std::ifstream infile(source_file_name_std); // read file lists
    while (std::getline(infile, line)) // iterate over files
    {
        NSString *source_file_name = [mainBundle pathForResource:[NSString stringWithCString:line.c_str()
                                                                                encoding:[NSString defaultCStringEncoding]]
                                                      ofType:nil];
        if(source_file_name == nil)
        {
            printf("%s\n", line.c_str());
            continue;
        }
        std::string source_file_name_std = std::string([source_file_name UTF8String]);
        
        Poco::Path pp(source_file_name_std);
        
        Poco::Path pp_line(line); // try to remove prefix, to be able create target path
        try {
            pp_line.popFrontDirectory();
            pp_line.popFrontDirectory();
        }
        catch(...)
        {
            printf("invalid file, skip uncompressing %s\n", line.c_str());
            continue;
        }
        NSString *newfilename = [NSString stringWithCString: (std::string("/" + pp_line.toString())).c_str()
                                                   encoding:[NSString defaultCStringEncoding]];
                                 
        NSString *target_file_name = [[NUIAppData SyncPath] stringByAppendingString: newfilename];
        std::string target_file_name_std = std::string([target_file_name UTF8String]);
        
        Poco::File pf(source_file_name_std);
        {
          // create folder structure for target
          std::string base_filename = target_file_name_std.substr(0, target_file_name_std.find_last_of("/"));
          Poco::File target_dir(base_filename);
          target_dir.createDirectories();
        }
        if(pf.exists()) // file exist in bundle
        {
            Poco::File target_fi(target_file_name_std);
            if(!target_fi.exists()) // file is not uncompressed
            {
                pf.copyTo(target_file_name_std);
            }
        }
    }
    }
#endif
    [NUIAppData getService]->setConfig(BenchmarkService::PRODUCT_ID, [[[NSBundle mainBundle] objectForInfoDictionaryKey:(NSString*) @"BUIProdctId"] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::PRODUCT_VERSION, ((NSString *)[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"]).UTF8String);
    [NUIAppData getService]->setConfig(BenchmarkService::PLATFORM_ID, "apple");
    [NUIAppData getService]->setConfig(BenchmarkService::INSTALLER_NAME, "appstore");
    [NUIAppData getService]->setConfig(BenchmarkService::PACKAGE_NAME, [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::LOCALE, [preferredLocale UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::WINDOW_MODE_ENABLED, "false");
    [NUIAppData getService]->setConfig(BenchmarkService::CONFIG_PATH, [[NUIAppData ConfigPath] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::DATA_PATH, [[NUIAppData DataPath] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::PLUGIN_PATH, "");
    [NUIAppData getService]->setConfig(BenchmarkService::APPDATA_PATH, [[NUIAppData RwDataPath] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::SYNCHRONIZATION_PATH, [[NUIAppData SyncPath] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::ASSET_IMAGE_PATH, [[[NSBundle mainBundle] resourcePath] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::TEST_IMAGE_PATH, [[[NSBundle mainBundle] resourcePath] UTF8String]);
    [NUIAppData getService]->setConfig(BenchmarkService::NEEDS_SYNC_BASED_ON_DATE, needsSync ? "true" : "false");
    [NUIAppData getService]->start();
    
    [NUIAppData getService]->setHideDesktopDevices(!showDesktop);
}

#pragma mark - Networking
- (void)StopNetNotifier
{
    [self.reachability stopNotifier];
}


- (void)StartNetNotifier
{
    [self.reachability startNotifier];
}


- (BOOL)isNetAvailable
{
    return [self.reachability currentReachabilityStatus] != NotReachable;
}


#pragma mark - Singleton
+ (id)sharedNUIAppData
{
    static NUIAppData *sharedMyNUIAppData = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedMyNUIAppData = [[self alloc] init];
    });
    return sharedMyNUIAppData;
}

- (id)init {
    if (self = [super init]) {
        NSLog(@"AppData init !!");
        
        // Load the theme
        [THTheme sharedTheme];
        
        self.loginName = nil;
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(callEventOnMainThread) name:NUI_Request_BenchmarkServiceEventRecieved object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(initFinished) name:NUI_Notification_InitializationFinished object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(NetStatusChanged:) name:kReachabilityChangedNotification object:nil];
        self.reachability = [NUIReachability reachabilityForInternetConnection];
        [self.reachability startNotifier];
    }
    return self;
}

- (void)dealloc
{
    [NUIAppData getService]->stop();
    [NUIAppData getService]->destroy();
    
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NUI_Request_ReloadData object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NUI_Request_BenchmarkServiceEventRecieved object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NUI_Notification_InitializationFinished object:nil];
    [self.reachability stopNotifier];
}

@end
