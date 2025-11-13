/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#import "Controls/NUILoadingBar.h"
#import "Common/THTheme.h"
#import "Common/NUIAppData.h"

@implementation NUILoadingBar

- (void)setupWithFrame:(CGRect)frame withTestId:(NSString *)test Name:(NSString *)name
{
    self.frame = frame;

    self.LeftText.text = [test rangeOfString:@"_off"].location != NSNotFound ?
    [NSString stringWithFormat:[NUIAppData getLocalized:@"TestLoading"], [[NUIAppData getLocalized:name] UTF8String]] :
        @"";
    self.TestTitle.text = [NUIAppData getLocalized:name];
	self.isLoadingText.text = [NSString stringWithFormat: @"%@ is now loading:", [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDisplayName"]];

    int x = self.frame.size.width;
    int y = self.frame.size.height;
    if(self.frame.size.width < self.frame.size.height)
    {
        x = self.frame.size.height;
        y = self.frame.size.width;
    }

    int strip_height = 120;
    int progress_height = 4;
    int y_padding = 4;
    int x_padding = 10;

    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone)
    {
        strip_height = 75;
    }

    self.BackImage.frame = CGRectMake(0, 0, x, y);
    self.LoadingStrip.frame = CGRectMake(0, (int)(y*2.0f/3.0f - (strip_height + progress_height)/2.0f), x, strip_height + progress_height);
    self.TextStrip.frame = CGRectMake(0, 0, x, strip_height);
    self.ProgressStrip.frame = CGRectMake(0, strip_height, 0, progress_height);
    self.LeftTextView.frame = CGRectMake(0, 0, (int)(x*2.0f/5.0f), strip_height);
    self.Separator.frame = CGRectMake(x*2.0f/5.0f, y_padding, 1, strip_height - y_padding*2);
    self.RightTextView.frame = CGRectMake(x*2.0f/5.0f + 1, 0, x - (x*2.0f/5.0f + 1), strip_height);

    self.LoadingStrip.backgroundColor = [THTheme getColorNamed:@"TransparentColor"];
    self.TextStrip.backgroundColor = [THTheme getColorNamed:@"BackColor"];
    self.ProgressStrip.backgroundColor = [THTheme getColorNamed:@"MainColor"];
    self.LeftTextView.backgroundColor = [THTheme getColorNamed:@"BackColor"];
    self.RightTextView.backgroundColor = [THTheme getColorNamed:@"BackColor"];
    self.Separator.backgroundColor = [THTheme getColorNamed:@"SeparatorColor"];

    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone)
    {
        self.isLoadingText.font = [THTheme getFontNamed:@"FontLoadingScreenMain"];
        self.TestTitle.font = [THTheme getFontNamed:@"FontLoadingScreenTitle"];
        self.LeftText.font = [THTheme getFontNamed:@"FontLoadingScreenMain"];
    }
    else
    {
        self.isLoadingText.font = [THTheme getFontNamed:@"FontPadLoadingScreenMain"];
        self.TestTitle.font = [THTheme getFontNamed:@"FontPadLoadingScreenTitle"];
        self.LeftText.font = [THTheme getFontNamed:@"FontPadLoadingScreenMain"];
    }

    self.isLoadingText.textColor = [THTheme getColorNamed:@"TextLoadingScreenColor"];
    self.TestTitle.textColor = [THTheme getColorNamed:@"TextLoadingScreenTitleColor"];
    self.LeftText.textColor = [THTheme getColorNamed:@"TextLoadingScreenColor"];





    self.LeftText.frame = CGRectMake(x_padding, y_padding, self.LeftTextView.frame.size.width - 2*x_padding, self.LeftTextView.frame.size.height - 2*y_padding);

    [self.isLoadingText sizeToFit];
    [self.TestTitle sizeToFit];
    float titleWidth = self.TestTitle.frame.size.width > self.RightTextView.frame.size.width - 2*x_padding ? self.RightTextView.frame.size.width - 2*x_padding : self.TestTitle.frame.size.width;
    self.isLoadingText.frame = CGRectMake(self.RightTextView.frame.size.width - x_padding - self.isLoadingText.frame.size.width,
                                          self.RightTextView.frame.size.height/2.0f - (self.isLoadingText.frame.size.height + self.TestTitle.frame.size.height)/2.0f,
                                          self.isLoadingText.frame.size.width,
                                          self.isLoadingText.frame.size.height);
    self.TestTitle.frame = CGRectMake(self.RightTextView.frame.size.width - x_padding - titleWidth,
                                          self.isLoadingText.frame.origin.y + self.isLoadingText.frame.size.height,
                                          titleWidth,
                                          self.TestTitle.frame.size.height);

    UIImage *testImage = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[test stringByAppendingString:@"_loading"] ofType:@"png"]];
    if(!testImage)
    {
        testImage = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"test_no_image_loading" ofType:@"png"]];
    }
    self.BackImage.image = testImage;
}


- (void)setProgress:(NSNumber *)progress
{
    int strip_height = 120;
    int progress_height = 4;

    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone)
    {
        strip_height = 75;
    }

    self.ProgressStrip.frame = CGRectMake(0, strip_height, self.LoadingStrip.frame.size.width * [progress floatValue], progress_height);
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
