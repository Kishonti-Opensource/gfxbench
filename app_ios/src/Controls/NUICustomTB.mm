/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Controls/NUICustomTB.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUIMessageKeys.h"
#import "Common/NUIAppData.h"

@implementation NUICustomTB

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
    }
    return self;
}


-(void)setup
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(disable) name:NUI_Request_DisableTabbar object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(enable) name:NUI_Request_EnableTabbar object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reloadData) name:NUI_Notification_LocalizationChanged object:nil];


    [self defaultColors];
    [self.HomeBack setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.ColoredLine setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.ColoredLineShadow setBackgroundColor:[THTheme getColorNamed:@"TabBarShadowColor"]];
    [self.HomeAddition setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    self.HomeImage.image = [THTheme imageNamed:@"Home" withTintColorName:@"TabBarColor"];
    self.HomeLabel.textColor = [THTheme getColorNamed:@"TextTabBarHighlightColor"];

    if(self.LeftPadding != nil)
    {
        self.LeftPadding.backgroundColor = [THTheme getColorNamed:@"TabBarSeparatorColor"];
        self.RightPadding.backgroundColor = [THTheme getColorNamed:@"TabBarSeparatorColor"];
        self.LeftPaddingFront.backgroundColor = [THTheme getColorNamed:@"TabBarColor"];
        self.RightPaddingFront.backgroundColor = [THTheme getColorNamed:@"TabBarColor"];
    }

    self.SelectedItem = 0;

    self.HomeLabel.text = [NUIAppData getLocalized:@"TabHome"];
    self.ResultLabel.text = [NUIAppData getLocalized:@"TabResults"];
    self.CompareLabel.text = [NUIAppData getLocalized:@"TabCompare"];
    self.InfoLabel.text = [NUIAppData getLocalized:@"TabInfo"];
    self.SignInLabel.text = [NUIAppData getLocalized:@"TabSettings"];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reloadData) name:NUI_Request_UIReload object:nil];
}

- (void)reloadData
{
    self.HomeLabel.text = [NUIAppData getLocalized:@"TabHome"];
    self.ResultLabel.text = [NUIAppData getLocalized:@"TabResults"];
    self.CompareLabel.text = [NUIAppData getLocalized:@"TabCompare"];
    self.InfoLabel.text = [NUIAppData getLocalized:@"TabInfo"];
    self.SignInLabel.text = [NUIAppData getLocalized:@"TabSettings"];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NUI_Request_UIReload object:nil];
}


-(void)defaultColors
{
    UIColor* normalColor = [THTheme getColorNamed:@"TabBarColor"];
    UIColor* placeholderColor = [THTheme getColorNamed:@"TabBarSeparatorColor"];

    [self.HomeBack setBackgroundColor:normalColor];
    [self.ResultBack setBackgroundColor:normalColor];
    [self.CompareBack setBackgroundColor:normalColor];
    [self.InfoBack setBackgroundColor:normalColor];
    [self.SignInBack setBackgroundColor:normalColor];

    self.HomeImage.image = [THTheme imageNamed:@"Home" withTintColorName:@"BackColor"];
    self.ResultsImage.image = [THTheme imageNamed:@"Results" withTintColorName:@"BackColor"];
    self.CompareImage.image = [THTheme imageNamed:@"Compare" withTintColorName:@"BackColor"];
    self.InfoImage.image = [THTheme imageNamed:@"Info" withTintColorName:@"BackColor"];
    self.SignInImage.image = [THTheme imageNamed:@"Profile" withTintColorName:@"BackColor"];

    self.HomeLabel.textColor = [THTheme getColorNamed:@"TextTabBarNormalColor"];
    self.ResultLabel.textColor = [THTheme getColorNamed:@"TextTabBarNormalColor"];
    self.CompareLabel.textColor = [THTheme getColorNamed:@"TextTabBarNormalColor"];
    self.InfoLabel.textColor = [THTheme getColorNamed:@"TextTabBarNormalColor"];
    self.SignInLabel.textColor = [THTheme getColorNamed:@"TextTabBarNormalColor"];

    [self.HomeAddition setBackgroundColor:placeholderColor];
    [self.ResultsAddition setBackgroundColor:placeholderColor];
    [self.CompareAddition setBackgroundColor:placeholderColor];
    [self.InfoAddition setBackgroundColor:placeholderColor];
    [self.SignInAddition setBackgroundColor:placeholderColor];

    [self.ColoredLine setBackgroundColor:normalColor];
}


- (IBAction)BtnSelected:(UIButton*)sender
{
    if([self.delegate respondsToSelector:@selector(hideKeyboardOnPage:)])
    {
        [self.delegate performSelector:@selector(hideKeyboardOnPage:) withObject:@(self.SelectedItem)];
    }
    self.SelectedItem = sender.tag;
    if([self.delegate respondsToSelector:@selector(Tabbar:didChooseTab:)])
    {
        [self.delegate Tabbar:self didChooseTab:self.SelectedItem];
    }
}


- (void) selectBtn: (NSInteger)itemnumber
{
    [self setColorsWithIndex:itemnumber];
    self.SelectedItem = itemnumber;
}


- (void) setColorsWithIndex:(NSInteger)index
{
    UIColor* selectedColor;

    [self defaultColors];

    switch (index) {
        case 0:
            selectedColor = [THTheme getColorNamed:@"MainColor"];
            [self.HomeBack setBackgroundColor:selectedColor];
            [self.ColoredLine setBackgroundColor:selectedColor];
            [self.HomeAddition setBackgroundColor:selectedColor];
            self.HomeImage.image = [THTheme imageNamed:@"Home" withTintColorName:@"TabBarColor"];
            self.HomeLabel.textColor = [THTheme getColorNamed:@"TextTabBarHighlightColor"];
            break;
        case 1:
            selectedColor = [THTheme getColorNamed:@"MainColor"];
            [self.ResultBack setBackgroundColor:selectedColor];
            [self.ColoredLine setBackgroundColor:selectedColor];
            [self.ResultsAddition setBackgroundColor:selectedColor];
            self.ResultsImage.image = [THTheme imageNamed:@"Results" withTintColorName:@"TabBarColor"];
            self.ResultLabel.textColor = [THTheme getColorNamed:@"TextTabBarHighlightColor"];
            break;
        case 2:
            selectedColor = [THTheme getColorNamed:@"MainColor"];
            [self.CompareBack setBackgroundColor:selectedColor];
            [self.ColoredLine setBackgroundColor:selectedColor];
            [self.CompareAddition setBackgroundColor:selectedColor];
            self.CompareImage.image = [THTheme imageNamed:@"Compare" withTintColorName:@"TabBarColor"];
            self.CompareLabel.textColor = [THTheme getColorNamed:@"TextTabBarHighlightColor"];
            break;
        case 3:
            selectedColor = [THTheme getColorNamed:@"MainColor"];
            [self.InfoBack setBackgroundColor:selectedColor];
            [self.ColoredLine setBackgroundColor:selectedColor];
            [self.InfoAddition setBackgroundColor:selectedColor];
            self.InfoImage.image = [THTheme imageNamed:@"Info" withTintColorName:@"TabBarColor"];
            self.InfoLabel.textColor = [THTheme getColorNamed:@"TextTabBarHighlightColor"];
            break;
        case 4:
            selectedColor = [THTheme getColorNamed:@"MainColor"];
            [self.SignInBack setBackgroundColor:selectedColor];
            [self.ColoredLine setBackgroundColor:selectedColor];
            [self.SignInAddition setBackgroundColor:selectedColor];
            self.SignInImage.image = [THTheme imageNamed:@"Profile" withTintColorName:@"TabBarColor"];
            self.SignInLabel.textColor = [THTheme getColorNamed:@"TextTabBarHighlightColor"];
            break;

        default:
            break;
    }
}

- (void)disable
{
    self.HomeBtn.enabled = false;
    self.ResultsBtn.enabled = false;
    self.CompareBtn.enabled = false;
    self.InfoBtn.enabled = false;
    self.SignInBtn.enabled = false;
}

- (void)enable
{
    self.HomeBtn.enabled = true;
    self.ResultsBtn.enabled = true;
    self.CompareBtn.enabled = true;
    self.InfoBtn.enabled = true;
    self.SignInBtn.enabled = true;
}

@end
