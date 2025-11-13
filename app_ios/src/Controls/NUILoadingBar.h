/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>

@interface NUILoadingBar : UIView

@property (weak, nonatomic) IBOutlet UIImageView *BackImage;
@property (weak, nonatomic) IBOutlet UIView *LoadingStrip;
@property (weak, nonatomic) IBOutlet UIView *TextStrip;
@property (weak, nonatomic) IBOutlet UIView *ProgressStrip;
@property (weak, nonatomic) IBOutlet UIView *LeftTextView;
@property (weak, nonatomic) IBOutlet UIView *Separator;
@property (weak, nonatomic) IBOutlet UIView *RightTextView;
@property (weak, nonatomic) IBOutlet UILabel *LeftText;
@property (weak, nonatomic) IBOutlet UILabel *isLoadingText;
@property (weak, nonatomic) IBOutlet UILabel *TestTitle;

- (void)setupWithFrame:(CGRect)frame withTestId:(NSString *)test Name:(NSString *)name;
- (void)setProgress:(NSNumber *)progress;

@end
