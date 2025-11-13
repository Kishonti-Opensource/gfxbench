/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPResultDetailChartCell.h
//  app_ios
//
//  Created by Balazs Hajagos on 21/10/2015.
//
//

#import <UIKit/UIKit.h>
#import "Charts/THLineChart.h"

@interface APPResultDetailChartCell : UITableViewCell

@property (weak, nonatomic) IBOutlet UIView *chartPlaceholder;
@property (strong, nonatomic) IBOutlet THLineChart *chart;

@end
