/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#import "NSFileManagerLocations.h"
#import "NUIResultDetailView.h"
#import "ng/log.h"
#import "testfw.h"
#import "schemas/descriptors.h"

@interface PickerViewController : UIViewController <UIPickerViewDataSource, UIPickerViewDelegate>
{
    tfw::TestBase* currentTest;
}

@property (weak, nonatomic) IBOutlet UIPickerView *testPicker;
@property (weak, nonatomic) IBOutlet UIButton *startButton;
@property (weak, nonatomic) IBOutlet NUIResultDetailView *webview;

- (IBAction)startPressed:(id)sender;

@end
