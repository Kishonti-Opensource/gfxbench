/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Charts/THChart.h"

@interface THGridChart : THChart

/// Represents the color of the axis. Defaults to: LineChart_BorderColor
@property (nonatomic, strong) UIColor *chartBorderColor;
/// Color of the grid lines. Defaults to: LineChart_LineColor
@property (nonatomic, strong) UIColor *lineColor;
/// Color of the text. Defaults to: LineChart_TextColor
@property (nonatomic, strong) UIColor *textColor;

/// Font of the text on the axis, and the legend layers. Defaults to: TextSFont
@property (nonatomic, strong) UIFont *textFont;

/// Width of the line drawn as axis. Defaults to: LineChart_BorderWidth
@property (nonatomic, assign) float chartBorderWidth;
/// Width of the grid lines. Defaults to: LineChart_LineWidth
@property (nonatomic, assign) float lineWidth;
/// Overall padding value. Defaults to: DefaultPadding
@property (nonatomic, assign) float padding;
/// Size of the legend color marks. Defaults to: LineChart_LegendColorWidth
@property (nonatomic, assign) float legendColorWidth;

/// Display the grid on the chart. Defaults to: true
@property (nonatomic, assign) BOOL displayGrid;
/// Display the legend on the chart. Defaults to: true
@property (nonatomic, assign) BOOL displayLegend;
/// Display the axis on the chart. Defaults to: true
@property (nonatomic, assign) BOOL displayAxis;

@property (nonatomic, assign) CGAffineTransform viewTransform;
@property (nonatomic, assign) CGAffineTransform baseTransform;
@property (nonatomic, assign) CGAffineTransform gridTransform;

@property (nonatomic, assign) float dMin;
@property (nonatomic, assign) float dMax;
@property (nonatomic, assign) float rMin;
@property (nonatomic, assign) float rMax;

- (CGRect)getChartRect;
- (CGAffineTransform)getGridTransform;
- (void)modifyViewMatrix;

@end
