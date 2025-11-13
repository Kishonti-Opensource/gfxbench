/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Pages/NUITestViewController.h"
#import "Utils/NUIUtilities.h"
#import "Common/NUIAppData.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NSFileManagerLocations.h"
#import "ng/log.h"
#import "testfw.h"
#import "jsonutils.h"
#import "graphics/eaglgraphicscontext.h"
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

#define LOADING_BAR 1


void _CHECK_GLERROR ()
{
	int err = glGetError();
	if (err)
		NGLOG_TRACE("GLError: %s", err);
}


void _CHECK_GLFBOERROR ()
{
	int err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (err != GL_FRAMEBUFFER_COMPLETE)
	{
		NGLOG_TRACE("Status: %s", err);
	}
}
#define CHECK_GLERROR _CHECK_GLERROR();
#define CHECK_GLFBOERROR _CHECK_GLFBOERROR();


#pragma mark -

@interface NUITestViewController ()

@property(strong, nonatomic) NSString *savepath;
@property(strong, nonatomic) NSString *date;
@property(strong, nonatomic) NSNumber *currentSession;

@property(strong, nonatomic) UISwipeGestureRecognizer *leftSwipeRecognizer;
@property(strong, nonatomic) UISwipeGestureRecognizer *rightSwipeRecognizer;
@property(strong, nonatomic) UISwipeGestureRecognizer *upSwipeRecognizer;
@property(strong, nonatomic) UISwipeGestureRecognizer *downSwipeRecognizer;

@end

@implementation NUITestViewController
{
    tfw::TestBase *currentTest;
    float progress;
    bool log_progress;
    bool net_went_off;
    dispatch_queue_t runner_queue;
    float old_brightness;
    float batterylevel;

    int currentTestIndex;
}

- (BOOL)prefersHomeIndicatorAutoHidden
{
    return YES;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(createContext:) name:NUI_Request_CreateContext object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleTestLoaded) name:NUI_Notification_TestLoaded object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleShowResults) name:NUI_Request_ShowResults object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleTestEnd:) name:NUI_Notification_TestEnd object:nil];

    _contextProvider = [[TestContextProvider alloc] init];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    [[NUIAppData sharedNUIAppData] StopNetNotifier];


    //Add swipe recognizers to cancel tests.
    self.leftSwipeRecognizer = [[[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(respondToSwipeGesture:)] autorelease];
    self.leftSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionLeft;
    [self.view addGestureRecognizer:self.leftSwipeRecognizer];
    self.rightSwipeRecognizer = [[[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(respondToSwipeGesture:)] autorelease];
    self.rightSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionRight;
    [self.view addGestureRecognizer:self.rightSwipeRecognizer];
    self.upSwipeRecognizer = [[[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(respondToSwipeGesture:)] autorelease];
    self.upSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionUp;
    [self.view addGestureRecognizer:self.upSwipeRecognizer];
    self.downSwipeRecognizer = [[[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(respondToSwipeGesture:)] autorelease];
    self.downSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionDown;
    [self.view addGestureRecognizer:self.downSwipeRecognizer];



    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(batteryChanged) name:UIDeviceBatteryLevelDidChangeNotification object:nil];
    batterylevel = 1.0;
}

- (void) createContext:(NSNotification *)notification {
    if(notification.userInfo == nil ||
       [notification.userInfo objectForKey:@"testId"] == nil ||
       [notification.userInfo objectForKey:@"loadingImage"] == nil ||
       [notification.userInfo objectForKey:@"graphics"] == nil ||
       [notification.userInfo objectForKey:@"desc"] == nil) {
        NSLog(@"create context notification is invalid");
        return;
    }

    NSString *testId = [notification.userInfo objectForKey:@"testId"];

    [self displayLoadingScreenWithId:testId WithName:[NUIAppData getLocalized:testId]];

    tfw::Graphics *graphics = (tfw::Graphics *)[[notification.userInfo objectForKey:@"graphics"] pointerValue];
    tfw::Descriptor *desc = (tfw::Descriptor *)[[notification.userInfo objectForKey:@"desc"] pointerValue];

    if(desc->rawConfigHasKey("brightness")) {
        double new_brightness = desc->rawConfign("brightness");
        if(new_brightness >= 0)
            [[UIScreen mainScreen] setBrightness:new_brightness];
    }


    [self.view setNeedsUpdateConstraints];
    [self.view updateConstraintsIfNeeded];
    [self.view setNeedsLayout];
    [self.view layoutIfNeeded];

    [_contextProvider setVersion:graphics withFrame:self.view.frame];
    self->ctx_ = [_contextProvider getGraphicsContext];
    self.glView = [_contextProvider getTestView];
    [self.view addSubview:self.glView];
    [self.view bringSubviewToFront:self.loadingBar];

    [NUIAppData getService]->loadTest(self->ctx_,
                                      self.glView.frame.size.width * self.glView.contentScaleFactor,
                                      self.glView.frame.size.height * self.glView.contentScaleFactor);
}

- (void) handleTestLoaded {
    [self removeLoadingScreen];
    [NUIAppData getService]->runTest();
}

- (void) handleShowResults {
    [UIApplication sharedApplication].idleTimerDisabled = NO;
    if(old_brightness != [[UIScreen mainScreen] brightness])
    {
        [[UIScreen mainScreen] setBrightness:old_brightness];
    }


    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_UIStyle object:self];
}

- (IBAction)respondToSwipeGesture:(UISwipeGestureRecognizer *)recognizer {
    [self CancelTests];
}


- (void)batteryChanged
{
    batterylevel = [[UIDevice currentDevice] batteryLevel]*100;
}


- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];

    [self startSession];
}

- (void)startSession
{

    using namespace tfw;
    NGLOG_INFO("starting tests");
    old_brightness = [[UIScreen mainScreen] brightness];

    NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
    [dateFormatter setDateFormat:@"y_MM_d_HH_mm_ss"];
    self.date = [dateFormatter stringFromDate: [NSDate date]];
    NSString *path = [[NSFileManager defaultManager] applicationSupportDirectory];
    path = [path stringByAppendingString:@"/results/"];
    path = [path stringByAppendingString:self.date];
    [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil];
    self.savepath = path;

    [UIApplication sharedApplication].idleTimerDisabled = YES;

    if(self.CommandLine != nil)
    {
        [self startCmdTest];
    }

}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

- (void)pollProgress
{
    while(log_progress)
    {
        if(self->currentTest == NULL)
            break;

        progress = self->currentTest->progress();

#if LOADING_BAR
        [self.loadingBar performSelectorOnMainThread:@selector(setProgress:) withObject:[NSNumber numberWithFloat:progress] waitUntilDone:NO];
#endif
        usleep(100 * 1000);
    }
    return;
}

- (IBAction)buttonPressed:(id)sender {
    self.isCancelled = TRUE;
    if(self->currentTest != nullptr)
        self->currentTest->cancel();
}


- (void)viewWillDisappear:(BOOL)animated
{
    [context_ release];
    self.glView = nil;
    [self setGlView:nil];
    if(self->ctx_ != NULL)
        delete self->ctx_;

	[super viewWillDisappear:animated];
}


- (void)saveResult:(std::string)jsonString fileName:(NSString *)name ToPath:(NSString*)path
{
    NSString *filePath = path;
    [[NSFileManager defaultManager] createDirectoryAtPath:filePath withIntermediateDirectories:YES attributes:nil error:nil];
    NSString *fileName = [NSString stringWithFormat:@"%@.json", name];
    NSString *fileAtPath = [filePath stringByAppendingPathComponent:@"/"];
    fileAtPath = [fileAtPath stringByAppendingPathComponent:fileName];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fileAtPath]) {
        [[NSFileManager defaultManager] createFileAtPath:fileAtPath contents:nil attributes:nil];
    }
    [[[NSString stringWithUTF8String:jsonString.c_str()] dataUsingEncoding:NSUTF8StringEncoding] writeToFile:fileAtPath atomically:NO];
}


- (void)saveResult:(std::string)jsonString fileName:(NSString *)name
{
    NSString *filePath = [[NUIAppData RwDataPath] stringByAppendingString:@"/results"];
    NSString *fileName = [NSString stringWithFormat:@"%@.json", name];
    NSString *fileAtPath = [filePath stringByAppendingPathComponent:@"/"];
    fileAtPath = [fileAtPath stringByAppendingPathComponent:fileName];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fileAtPath]) {
        [[NSFileManager defaultManager] createFileAtPath:fileAtPath contents:nil attributes:nil];
    }
    [[[NSString stringWithUTF8String:jsonString.c_str()] dataUsingEncoding:NSUTF8StringEncoding] writeToFile:fileAtPath atomically:NO];
}


- (NUILoadingBar*)loadCustomLoadBar
{
    NSArray* topLevelObjects;
    topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"LoadingBar" owner:self options:Nil];
    NUILoadingBar* CustomLB = [topLevelObjects objectAtIndex:0];

    return CustomLB;
}

- (void)displayLoadingScreenWithId:(NSString *)test_id WithName:(NSString *)name
{
#if LOADING_BAR
	// hack: remove accidentaly existing loading screen
	[self removeLoadingScreen];

    dispatch_queue_t main_queue = dispatch_get_main_queue();
    dispatch_async(main_queue, ^(void)
      {
          self.loadingBar = [self loadCustomLoadBar];
          if([UIDevice currentDevice].userInterfaceIdiom != UIUserInterfaceIdiomPad)
          {
              [self.loadingBar setTransform:CGAffineTransformMakeRotation(-M_PI_2)];
              [self.loadingBar setupWithFrame:self.view.frame withTestId:test_id Name:name];
          }
          else
          {
              [self.loadingBar setupWithFrame:self.view.frame withTestId:test_id Name:name];
          }

          [self.view addSubview:self.loadingBar];
      });
#endif
}

- (void)removeLoadingScreen
{
#if LOADING_BAR
    dispatch_queue_t main_queue = dispatch_get_main_queue();
    dispatch_async(main_queue, ^(void)
    {
        [self.loadingBar performSelectorOnMainThread:@selector(removeFromSuperview) withObject:self.loadingBar waitUntilDone:YES];
        self.loadingBar = nil;
    });
#endif
}

- (void)CancelTests
{
    self.isCancelled = true;
    if(self->currentTest != NULL)
    {
        self->currentTest->cancel();
    }
}

- (void)LoadingDelayWithDate:(NSDate *)target_end
{
    NSTimeInterval remainingTime = [target_end timeIntervalSinceDate:[NSDate date]];
    if(remainingTime > 0) {
        usleep(remainingTime * 1000000);
    }
}

- (void) dealloc {
    if(self.leftSwipeRecognizer != nil) {
        [self.view removeGestureRecognizer:self.leftSwipeRecognizer];
    }
    if(self.rightSwipeRecognizer != nil) {
        [self.view removeGestureRecognizer:self.rightSwipeRecognizer];
    }
    if(self.upSwipeRecognizer != nil) {
        [self.view removeGestureRecognizer:self.upSwipeRecognizer];
    }
    if(self.downSwipeRecognizer != nil) {
        [self.view removeGestureRecognizer:self.downSwipeRecognizer];
    }

    if(_contextProvider != nil) {
        [_contextProvider release];
    }

    [super dealloc];
}


//--------------------------------------------------------------------------------------------------------------------
#pragma mark - Command Line

- (void)startCmdTest
{
    using namespace tfw;

    NSLog(@"test NO.: %d", currentTestIndex);

    tfw::Descriptor desc = [self.CommandLine getDescriptors].descriptors().at(currentTestIndex);
    [self displayLoadingScreenWithId:[NSString stringWithUTF8String:desc.testId().c_str()] WithName:[NSString stringWithUTF8String:desc.testId().c_str()]];

    TestBase *test = 0;

    std::string testName = desc.factoryMethod();
    TestFactory factory = TestFactory::test_factory(desc.factoryMethod().c_str());
    if (factory.valid())
    {
        test = factory.create_test();

        // Set the environment from json.

        NSString *testId = [NSString stringWithUTF8String:desc.testId().c_str()];

        tfw::Environment env = desc.env();
        tfw::Graphics graphics = env.graphics();

        [_contextProvider setVersion:&graphics withFrame:self.view.frame];
        self->ctx_ = [_contextProvider getGraphicsContext];

        if(self->ctx_ != nullptr)
        {
            if([testId rangeOfString:@"battery"].location != NSNotFound)
            {
                if(old_brightness < 0)
                    old_brightness = [[UIScreen mainScreen] brightness];
                [[UIScreen mainScreen] setBrightness:0.5];
            }

            self.glView = [_contextProvider getTestView];


            env.setHeight(self.glView.frame.size.height * self.glView.contentScaleFactor);
            env.setWidth(self.glView.frame.size.width * self.glView.contentScaleFactor);

            [self.view addSubview:self.glView];
            [self.view bringSubviewToFront:self.loadingBar];

            NSString *envReadPath = [self.CommandLine getInputPath];
            NSString *dataPrefix =  [NSString stringWithUTF8String:desc.dataPrefix().c_str()];
            envReadPath = [envReadPath stringByAppendingString:@"/data/"];
            envReadPath = [envReadPath stringByAppendingString:dataPrefix];
            envReadPath = [envReadPath stringByAppendingString:@"/"];
            NSString *envWritePath = [self.CommandLine getOutputPathWithDefault:self.savepath];
            env.setReadPath([envReadPath UTF8String]);
            env.setWritePath([envWritePath UTF8String]);
            desc.setEnv(env);
            test->setConfig(desc.toJsonString());
            test->setGraphicsContext(self->ctx_);

            self->currentTest = test;
            progress = 0;
            log_progress = true;

            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, (unsigned long)NULL), ^(void)
                           {
                               [self pollProgress];
                           });


            runner_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, (unsigned long)NULL);
            dispatch_async(runner_queue, ^{

                NSDate *target_end = [NSDate dateWithTimeIntervalSinceNow:2];
                if (self->currentTest->init())
                {
                    log_progress = false;
                    [self LoadingDelayWithDate:target_end];
                    [self removeLoadingScreen];

                    NSLog(@"test started");
                    self->currentTest->run();
                }
                else
                {
                    NGLOG_ERROR("failed to init test: %s", desc.testId());
                }

                dispatch_queue_t main_queue = dispatch_get_main_queue();
                dispatch_async(main_queue, ^{
                    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_TestEnd object:self];
                });
            });

        }
        else
        {
            NSLog(@"No supported version. (required ES or METAL)");
            NGLOG_ERROR("no supported version (required ES or METAL)");
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_TestEnd object:self userInfo:@{@"error": @"no supported version (required ES or METAL)"}];
        }
    }
    else
    {
        NGLOG_ERROR("failed to load test: %s", desc.testId());
        [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Notification_TestEnd object:self userInfo:@{@"error": @"TestFactory: failed to create test."}];
    }
}

- (void)handleTestEnd:(NSNotification *)e
{
    if(self.CommandLine != nil)
    {
        NSString *error = nil;
        if(e.userInfo != nil)
        {
            error = e.userInfo[@"error"];
        }

        if(self.glView != nil)
        {
            [self.glView removeFromSuperview];
            self.glView = nil;
        }

        tfw::Descriptor desc;
        tfw::TestInfo testinfo;
        desc = [self.CommandLine getDescriptors].descriptors().at(currentTestIndex);

        NSString* testId = [NSString stringWithUTF8String:desc.testId().c_str()];
        if([testId rangeOfString:@"battery"].location != NSNotFound)
        {
            [[UIScreen mainScreen] setBrightness:old_brightness];
        }

        std::string resultString = "";
        if(error == nil)
        {
            resultString = self->currentTest->result();
        }
        else
        {
            tfw::Result r0;
            r0.setResultId("");
            r0.setTestId([testId UTF8String]);
            r0.setStatus(tfw::Result::FAILED);
            r0.setErrorString([error UTF8String]);
            tfw::ResultGroup r;
            r.addResult(r0);
            resultString = r.toJsonString();
        }

        NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
        [dateFormatter setDateFormat:@"y_MM_d_HH_mm_ss"];
        NSString *DateString = [dateFormatter stringFromDate: [NSDate date]];
        NSString *SaveName = [NSString stringWithFormat:@"%@_%@", [NSString stringWithUTF8String:desc.testId().c_str()], DateString];

        NSString *path = [self.CommandLine getOutputPathWithDefault:self.savepath];
        path = [path stringByAppendingString:self.date];
        path = [path stringByAppendingString:@"/"];

        [self saveResult:resultString fileName:SaveName ToPath:path];

        if(self->currentTest != nullptr) {
            self->currentTest->terminate();
            delete self->currentTest;
            self->currentTest = nullptr;
        }

        currentTestIndex++;

        size_t max;
        if(self.CommandLine != nil)
        {
            max = [self.CommandLine getDescriptors].descriptors().size();
        }
        else
        {
            max = self.testNames.count;
        }


        if(currentTestIndex >= max || self.isCancelled)
        {
            exit(0);
        }
        else
        {
            [self startCmdTest];
        }
    }
}


@end
