/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Controls/THIconicLabel.h"
#import "Common/THTheme.h"

@interface THIconicLabel()

@property (strong, nonatomic) UIImageView *mIcon;
@property (strong, nonatomic) UILabel *mMainLabel;
@property (strong, nonatomic) UILabel *mSecondaryLabel;

@end

@implementation THIconicLabel

- (id) init {
    self = [super init];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (CGSize)getTextBoundingRectWithSize:(CGSize)size Secondary:(BOOL)secondary {
    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    textStyle.alignment = NSTextAlignmentLeft;
    textStyle.lineBreakMode = NSLineBreakByWordWrapping;
    NSDictionary *textFontAttributes = @{
                                         NSFontAttributeName: secondary ? self.mSecondaryLabel.font : self.mMainLabel.font,
                                         NSParagraphStyleAttributeName: textStyle
                                         };

    CGRect textRect = [self.mainString boundingRectWithSize:CGSizeMake(size.width, size.height)
                                                     options:NSStringDrawingUsesLineFragmentOrigin
                                                  attributes:textFontAttributes
                                                     context:nil];

    return textRect.size;
}

- (CGSize)sizeThatFits:(CGSize)size {
    CGSize ret = size;
    float defaultSize = [THTheme getFloatNamed:@"DefaultTappableSize"];

    if(self.mIcon != nil) {
        ret = CGSizeMake(MAX(ret.width, MIN(self.iconSizeCap, defaultSize + 2*self.padding)),
                         MAX(ret.height, MIN(self.iconSizeCap, defaultSize + 2*self.padding)));
    }
    if(self.mMainLabel != nil && self.mSecondaryLabel != nil) {
        float textMaxWidth = ret.width - ret.height - 2*self.padding - self.iconTextDist;
        CGSize mainLabelFit = [self.mMainLabel sizeThatFits:CGSizeMake(textMaxWidth, size.height)];
        CGSize secondaryLabelFit = CGSizeMake(0, 0);
        if(![self.secondaryString isEqualToString:@""])
            secondaryLabelFit = [self.mSecondaryLabel sizeThatFits:CGSizeMake(textMaxWidth, size.height)];
        ret = CGSizeMake(ret.width, MAX(ret.height,
                                        mainLabelFit.height + secondaryLabelFit.height + 2*self.padding + self.textTextDist));
    }

    return ret;
}

- (void)sizeToFit {
    CGSize s = [self sizeThatFits:self.frame.size];
    [self setFrame:CGRectMake(self.frame.origin.x, self.frame.origin.y, s.width, s.height)];
}

- (void) setupWiTHTheme {
    [self setBackgroundColor:[THTheme getColorNamed:@"BackgroundColor"]];
    self.iconSizeCap = FLT_MAX;
    self.iconColorName = @"MainColor";
    self.iconName = @"";
    self.mainString = @"MainString";
    self.secondaryString = @"";

    self.mainTextFont = [THTheme getFontNamed:@"TitleMFont"];
    self.secondaryTextFont = [THTheme getFontNamed:@"TextMFont"];
    self.mainTextColor = [THTheme getColorNamed:@"TextColor"];
    self.secondaryTextColor = [THTheme getColorNamed:@"TextColor"];

    self.padding = [THTheme getFloatNamed:@"DefaultPadding"];
    self.iconTextDist = [THTheme getFloatNamed:@"IconicLabel_IconTextDist"];
    self.textTextDist = [THTheme getFloatNamed:@"IconicLabel_TextTextDist"];

    if(self.mMainLabel == nil) {
        self.mMainLabel = [[UILabel alloc] init];
        [self addSubview:self.mMainLabel];
        [self.mMainLabel setText:self.mainString];
        [self.mMainLabel setTextColor:self.mainTextColor];
        [self.mMainLabel setFont:self.mainTextFont];
        [self.mMainLabel setNumberOfLines:2];
    }

    if(self.mSecondaryLabel == nil) {
        self.mSecondaryLabel = [[UILabel alloc] init];
        [self addSubview:self.mSecondaryLabel];
        [self.mSecondaryLabel setText:self.secondaryString];
        [self.mSecondaryLabel setTextColor:self.secondaryTextColor];
        [self.mSecondaryLabel setFont:self.secondaryTextFont];
        [self.mSecondaryLabel setNumberOfLines:2];
    }

    if(self.mIcon == nil) {
        self.mIcon = [[UIImageView alloc] init];
        [self addSubview:self.mIcon];
    }

    [self setFrame:self.frame];
}

- (void) setFrame:(CGRect)frame {
    [super setFrame:frame];

    float iconSize = 0;

    if(self.mIcon != nil) {
        iconSize = MIN(self.iconSizeCap, frame.size.height - 2*self.padding);

        [self.mIcon setFrame:CGRectMake(self.padding,
                                        MAX(self.padding, (frame.size.height-iconSize)*0.5f),
                                        iconSize,
                                        iconSize)];
    }
    if(self.mMainLabel != nil && self.mSecondaryLabel != nil) {
        float textMaxWidth = frame.size.width - frame.size.height - 2*self.padding - self.iconTextDist;
        CGSize mainLabelFit = [self.mMainLabel sizeThatFits:CGSizeMake(textMaxWidth, frame.size.height)];

        if([self.secondaryString isEqualToString:@""]) {
            [self.mMainLabel setFrame:CGRectMake(iconSize > 0 ? iconSize + self.padding + self.iconTextDist : self.padding,
                                                 MAX(self.padding, (frame.size.height - mainLabelFit.height) * 0.5f),
                                                 mainLabelFit.width,
                                                 mainLabelFit.height)];


        } else {
            CGSize secondaryLabelFit = [self.mSecondaryLabel sizeThatFits:CGSizeMake(textMaxWidth, frame.size.height)];
            [self.mMainLabel setFrame:CGRectMake(iconSize > 0 ? iconSize + self.padding + self.iconTextDist : self.padding,
                                                 self.padding,
                                                 mainLabelFit.width,
                                                 mainLabelFit.height)];
            [self.mSecondaryLabel setFrame:CGRectMake(iconSize > 0 ? iconSize + self.padding + self.iconTextDist : self.padding,
                                                      mainLabelFit.height + self.padding + self.textTextDist,
                                                      secondaryLabelFit.width,
                                                      secondaryLabelFit.height)];
        }

    }
}

- (void)setIconSizeCap:(float)size {
    _iconSizeCap = size;
    [self setFrame:self.frame];
}

- (void)setMainString:(NSString *)mainString {
    _mainString = mainString;
    [self.mMainLabel setText:self.mainString];
    [self setFrame:self.frame];
}

- (void)setSecondaryString:(NSString *)secondaryString {
    _secondaryString = secondaryString;
    [self.mSecondaryLabel setText:self.secondaryString];
    [self setFrame:self.frame];
}

- (void)setIconName:(NSString *)iconName {
    _iconName = iconName;
    UIImage *image = [THTheme imageNamed:self.iconName withTintColorName:self.iconColorName];
    if(image == nil) image = [THTheme imageWithColor:[THTheme getColorNamed:self.iconColorName]];
    [self.mIcon setImage:image];
    [self setFrame:self.frame];
}

- (void)setIconColorName:(NSString *)iconColorName {
    _iconColorName = iconColorName;
    [self.mIcon setImage:[THTheme imageNamed:self.iconName withTintColorName:self.iconColorName]];
}

- (void)setMainTextColor:(UIColor *)mainTextColor {
    _mainTextColor = mainTextColor;
    self.mMainLabel.textColor = mainTextColor;
}

- (void)setSecondaryTextColor:(UIColor *)secondaryTextColor {
    _secondaryTextColor = secondaryTextColor;
    self.mSecondaryLabel.textColor = secondaryTextColor;
}

- (void)setPadding:(float)padding {
    _padding = padding;
    [self setFrame:self.frame];
}

- (void)setIconTextDist:(float)iconTextDist {
    _iconTextDist = iconTextDist;
    [self setFrame:self.frame];
}

- (void)setTextTextDist:(float)textTextDist {
    _textTextDist = textTextDist;
    [self setFrame:self.frame];
}

- (void)setMainTextFont:(UIFont *)mainTextFont {
    _mainTextFont = mainTextFont;
    [self.mMainLabel setFont:mainTextFont];
}

- (void)setSecondaryTextFont:(UIFont *)secondaryTextFont {
    _secondaryTextFont = secondaryTextFont;
    [self.mSecondaryLabel setFont:secondaryTextFont];
}

@end
