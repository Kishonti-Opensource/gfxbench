/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>

@protocol NUICustomTabbarDelegate;

@interface NUICustomTB : UIView

@property (nonatomic, weak) id<NUICustomTabbarDelegate> delegate;

@property (weak, nonatomic) IBOutlet UIView *ColoredLine;
@property (weak, nonatomic) IBOutlet UIView *ColoredLineShadow;
@property (weak, nonatomic) IBOutlet UIButton *HomeBtn;
@property (weak, nonatomic) IBOutlet UIButton *ResultsBtn;
@property (weak, nonatomic) IBOutlet UIButton *CompareBtn;
@property (weak, nonatomic) IBOutlet UIButton *InfoBtn;
@property (weak, nonatomic) IBOutlet UIButton *SignInBtn;

@property (weak, nonatomic) IBOutlet UIView *HomeBack;
@property (weak, nonatomic) IBOutlet UIView *ResultBack;
@property (weak, nonatomic) IBOutlet UIView *CompareBack;
@property (weak, nonatomic) IBOutlet UIView *InfoBack;
@property (weak, nonatomic) IBOutlet UIView *SignInBack;

@property (weak, nonatomic) IBOutlet UILabel *HomeLabel;
@property (weak, nonatomic) IBOutlet UILabel *ResultLabel;
@property (weak, nonatomic) IBOutlet UILabel *CompareLabel;
@property (weak, nonatomic) IBOutlet UILabel *InfoLabel;
@property (weak, nonatomic) IBOutlet UILabel *SignInLabel;

@property (weak, nonatomic) IBOutlet UIImageView *HomeImage;
@property (weak, nonatomic) IBOutlet UIImageView *ResultsImage;
@property (weak, nonatomic) IBOutlet UIImageView *CompareImage;
@property (weak, nonatomic) IBOutlet UIImageView *InfoImage;
@property (weak, nonatomic) IBOutlet UIImageView *SignInImage;

@property (weak, nonatomic) IBOutlet UIView *HomeAddition;
@property (weak, nonatomic) IBOutlet UIView *ResultsAddition;
@property (weak, nonatomic) IBOutlet UIView *CompareAddition;
@property (weak, nonatomic) IBOutlet UIView *InfoAddition;
@property (weak, nonatomic) IBOutlet UIView *SignInAddition;

@property (weak, nonatomic) IBOutlet UIView *LeftPadding;
@property (weak, nonatomic) IBOutlet UIView *RightPadding;
@property (weak, nonatomic) IBOutlet UIView *LeftPaddingFront;
@property (weak, nonatomic) IBOutlet UIView *RightPaddingFront;

@property (assign, nonatomic) NSInteger SelectedItem;

- (IBAction)BtnSelected:(UIButton *)sender;
- (void) selectBtn: (NSInteger)itemnumber;
- (void)setup;

@end


@protocol NUICustomTabbarDelegate <NSObject>

- (void)Tabbar:(NUICustomTB*)viewController didChooseTab:(NSInteger)index;

@end
