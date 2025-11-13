/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "PickerViewController.h"
#import "ViewController.h"
#import "METLViewController.h"
#import "AppDelegate.h"
#import "schemas/result.h"


using namespace tfw;


@interface PickerViewController ()
@property (strong) NSArray *configUrls;
@property NSInteger selectedRow;
@end

@implementation PickerViewController


- (void)viewDidLoad
{
    [super viewDidLoad];

	NSString *configPath = [AppDelegate getConfigPath];
	NSString *errorMsg = nil;
	if (configPath) {
		NSError *error = nil;
		NSArray *configUrls = [[NSFileManager defaultManager]
							   contentsOfDirectoryAtURL:[NSURL fileURLWithPath:configPath]
							   includingPropertiesForKeys:nil
							   options:0
							   error:&error];
		if (configUrls == nil) {
			errorMsg = [NSString stringWithFormat:@"Can not list config files in %@: %@", configPath, [error localizedDescription]];
		}
		self.configUrls = configUrls;
		self.selectedRow = 0;
	} else {
		errorMsg = @"Can not find \"Documents/config\" directory, or \"<app_bundle>/config\". Please copy config and data directories to the device.";
	}

	if (errorMsg) {
        [self.webview loadHTMLString:errorMsg baseURL:nil];
		[self.startButton setEnabled:NO];
    } else {
        [self.webview loadHTMLString:@"No results yet." baseURL:nil];
	}

    [self prefersStatusBarHidden];
    [self performSelector:@selector(setNeedsStatusBarAppearanceUpdate)];

    self.testPicker.delegate = self;
    self.testPicker.dataSource = self;
}

- (void) viewDidAppear:(BOOL)animated
{
    if(currentTest) {
		tfw::ResultGroup g;
		g.fromJsonString(currentTest->result());
		NSString *result = [NSString stringWithUTF8String:currentTest->result().c_str()];
		NSString *flags = @"";
		for(size_t i = 0; i < g.flags().size(); ++i) {
			flags = [flags stringByAppendingFormat:@"%s ", g.flags().at(i).c_str()];
		}
		[self.webview setResultJsonString:result andResultName:@"" withflaggedReason:flags];
		currentTest->terminate();
        delete currentTest;
        currentTest = 0;
    }
}

- (BOOL)prefersStatusBarHidden
{
    return YES;
}


- (IBAction)startPressed:(id)sender {

	NSURL *configUrl = self.configUrls[self.selectedRow];
	NSError *error = nil;
	NSString *json = [NSString stringWithContentsOfURL:configUrl encoding:NSUTF8StringEncoding error:&error];
    if (json == nil) {
        [self.webview loadHTMLString:[NSString stringWithFormat:@"Can not load content of config url %@: %@", configUrl, [error localizedDescription]] baseURL:nil];
		return;
	}
	Descriptor d;
    d.fromJsonString([json UTF8String]);

    TestFactory factory = TestFactory::test_factory(d.factoryMethod().c_str());
    if (factory.valid()) {
		currentTest = factory.create_test();
		currentTest->setConfig(d.toJsonString());

		typedef std::vector<ApiDefinition> ApiDefinitions;
		const ApiDefinitions &apis = d.env().graphics().versions();
		for (ApiDefinitions::const_iterator api = apis.begin(); api != apis.end(); ++api) {
			if(api->type() == ApiDefinition::METAL) {
				if (&MTLCreateSystemDefaultDevice != nil) {
					[self performSegueWithIdentifier:@"ToMetal" sender:self];
				} else {
                    [self.webview loadHTMLString:@"Metal runtime not supported on the device" baseURL:nil];
				}
				break;
			} else if (api->type() == ApiDefinition::ES) {
				[self performSegueWithIdentifier:@"ToGl" sender:self];
				break;
			}
		}
	}
}


#pragma mark - Navigation

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    ViewController *vc = (ViewController*)[segue destinationViewController];
	[vc setTest:currentTest];
}

#pragma mark - PickerDataSource

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return self.configUrls.count;
}

#pragma mark - PickerDelegate

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
	NSURL *config = self.configUrls[row];
    return [[[config path] lastPathComponent] stringByDeletingPathExtension];
}

- (CGFloat)pickerView:(UIPickerView *)pickerView rowHeightForComponent:(NSInteger)component
{
    return 40;
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
	self.selectedRow = row;
}

@end
