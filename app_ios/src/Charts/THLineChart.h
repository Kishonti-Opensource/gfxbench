/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */



#import "Charts/THGridChart.h"

/**
 * @discussion A chart class that draws the given values on separate line layers.
 * All layers are colored differently, may be filled with gradient. May include
 * legend, gird, borders, and axises are drawed on demand.
 */
@interface THLineChart : THGridChart

/// Width of the lines which indicates the values. Defaults to: LineChart_ValueLineWidth
@property (nonatomic, assign) float valueLineWidth;

/// Display the gradients for the values. Defaults to: false
@property (nonatomic, assign) BOOL displayGradients;

@end
