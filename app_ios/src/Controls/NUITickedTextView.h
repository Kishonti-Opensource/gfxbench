/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUITickedTextView.h
//  GFXBench
//
//  Created by Kishonti Kft on 29/10/2013.
//
//

#import <UIKit/UIKit.h>

@interface NUITickedTextView : UICollectionViewCell

@property (weak, nonatomic) IBOutlet UILabel *Label;
@property (weak, nonatomic) IBOutlet UIImageView *TickImage;

- (void)setText:(NSString *)text;
- (void)setTicked:(BOOL)ticked;
- (CGSize)preferredLayoutSizeFittingSize:(CGSize)targetSize;

@end
