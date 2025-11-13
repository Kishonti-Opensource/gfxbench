/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import "Charts/THChart.h"

/**
 * @discussion A chart that show data added to it in a special pie chart like thing.
 * Display one pie section for each value, fills it proportional to the value,
 * and displays it with its name along the pie axis. Has several parameters that
 * are initialized based on the current theme.
 */
@interface THPolarAreaChart : THChart

/// Defines the width of the separator between the pie slices. Defaults to: PolarAreaChart_SeparatorWidth.
@property (nonatomic, assign) float separatorWidth;

/// Defines the distance of the Labels based on the slice radius. Defaults to: PolarAreaChart_LabelDistance.
@property (nonatomic, assign) float textDistance;

@end
