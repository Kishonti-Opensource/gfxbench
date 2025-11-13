/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPResultDetailChartCell.m
//  app_ios
//
//  Created by Balazs Hajagos on 21/10/2015.
//
//

#import "APPResultDetailChartCell.h"

@implementation APPResultDetailChartCell

- (void)layoutSubviews {
    [super layoutSubviews];
    if(self.chart != nil) {
        CGRect r = self.bounds;
        [self.chart setFrame:r];
    }
}

@end
