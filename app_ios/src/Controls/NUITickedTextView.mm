/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUITickedTextView.m
//  GFXBench
//
//  Created by Kishonti Kft on 29/10/2013.
//
//

#import "NUITickedTextView.h"
#import "Common/THTheme.h"
#import "Common/NUIAppData.h"

@implementation NUITickedTextView

//- (id)initWithFrame:(CGRect)frame
//{
//    self = [super initWithFrame:frame];
//    if (self) {
//        self.Label.font = [THTheme getFontNamed:@"FontInfoCellMinor"];
//        self.Label.textColor = [THTheme getColorNamed:@"TextInfoCellMinorColor"];
//        self.Label.text = @"";
//        self.TickImage.image = [THTheme imageNamed:@"DevInfoTickNo" withTintColorName:@"TickNegativeColor"];
//    }
//    return self;
//}


- (void)setText:(NSString *)text
{
    self.Label.font = [THTheme getFontNamed:@"FontInfoCellMinor"];
    self.Label.textColor = [THTheme getColorNamed:@"TextInfoCellMinorColor"];
    self.TickImage.image = [THTheme imageNamed:@"DevInfoTickNo" withTintColorName:@"TickNegativeColor"];
    
    self.Label.text = text;
//    
//    [self.Label setNumberOfLines:1];
//    
//    CGSize maximumLabelSize = CGSizeMake(9999,self.frame.size.height);
////    CGSize expectedLabelSize = [text sizeWithFont:self.Label.font
////                                       constrainedToSize:maximumLabelSize
////                                           lineBreakMode:NSLineBreakByClipping];
//    
//    CGRect expectedLabelSize = [text boundingRectWithSize:maximumLabelSize options:0 attributes:@{
//                                                                                                    NSFontAttributeName : self.Label.font
//                                                                                                    } context:nil];
//    
//    self.Label.frame = CGRectMake(0, 0, ceil(expectedLabelSize.size.width) + 2, self.frame.size.height);
//    self.TickImage.frame = CGRectMake(self.Label.frame.size.width, (self.Label.frame.size.height/2)-3, 10, 10);
//    self.frame = CGRectMake(self.frame.origin.x, self.frame.origin.y, self.Label.frame.size.width + self.TickImage.frame.size.width + 4, self.frame.size.height);
}


- (void)setTicked:(BOOL)ticked
{
    if(ticked)
        self.TickImage.image = [THTheme imageNamed:@"DevInfoTickYes" withTintColorName:@"TickPositiveColor"];
    else
        self.TickImage.image = [THTheme imageNamed:@"DevInfoTickNo" withTintColorName:@"TickNegativeColor"];
}

- (CGSize)preferredLayoutSizeFittingSize:(CGSize)targetSize {
    // save original frame and preferredMaxLayoutWidth
    CGRect originalFrame = self.frame;
    CGFloat originalPreferredMaxLayoutWidth = self.Label.preferredMaxLayoutWidth;
    
    // assert: targetSize.width has the required width of the cell
    
    // step1: set the cell.frame to use that width
    CGRect frame = self.frame;
    frame.size = targetSize;
    self.frame = frame;
    
    // step2: layout the cell
    [self setNeedsLayout];
    [self layoutIfNeeded];
    
    self.Label.preferredMaxLayoutWidth = self.Label.bounds.size.width;
    
    // assert: the label's bounds and preferredMaxLayoutWidth are set to the width required by the cell's width
    
    // step3: compute how tall the cell needs to be
    
    // this causes the cell to compute the height it needs, which it does by asking the
    // label what height it needs to wrap within its current bounds (which we just set).
    CGSize computedSize = [self systemLayoutSizeFittingSize:UILayoutFittingCompressedSize];
    
    // assert: computedSize has the needed height for the cell
    
    // Apple: "Only consider the height for cells, because the contentView isn't anchored correctly sometimes."
    CGSize newSize = CGSizeMake(computedSize.width, computedSize.height);
    
    // restore old frame and preferredMaxLayoutWidth
    self.frame = originalFrame;
    self.Label.preferredMaxLayoutWidth = originalPreferredMaxLayoutWidth;
    
    return newSize;
}

//- (UICollectionViewLayoutAttributes *)preferredLayoutAttributesFittingAttributes:(UICollectionViewLayoutAttributes *)layoutAttributes {
//    UICollectionViewLayoutAttributes *attr = [layoutAttributes copy];
//    
//    CGRect newFrame = attr.frame;
//    self.frame = newFrame;
//    
//    [self setNeedsLayout];
//    [self layoutIfNeeded];
//    
//    CGSize desiredSize = [self.contentView systemLayoutSizeFittingSize:UILayoutFittingCompressedSize];
//    attr.frame.size = CGSizeMake(200, 200);
//    return attr;
//}

@end
