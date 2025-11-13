/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#import <UIKit/UIKit.h>

/**
 * @brief Basic chart class.
 *
 * @discussion Superclass for all charts. Has minor functionalities only.
 * Stores the data passed to it and sorts the value lists in first with
 * less avg manner.
 *
 * It calls the following functions in specific situations:
 *
 *  On init - setup
 *
 *  On frame change - resizeLayers
 *
 *  On setDataJsonString - setupTransformation, fillData
 */
@interface THChart : UIView

- (id) initWithJsonString:(NSString *)jsonString;
- (id) initWithFrame:(CGRect)frame WithJsonString:(NSString *)jsonString;

/// Data displayed in the chart.
@property (strong, nonatomic) NSDictionary *data;

/// Avg sorted data displayed in the chart.
@property (strong, nonatomic) NSArray *sortedValues;

/// Animation time of the appear animations. Defaults to: LineChart_AnimationTime
@property (nonatomic, assign) float animationTime;
/// Delay between the animation of the separate value layers. Defaults to: LineChart_AnimationDelay
@property (nonatomic, assign) float animationDelay;
/// Delay until the first animation of the separate value layers. Defaults to: LineChart_StartAnimationDelay
@property (nonatomic, assign) float startAnimationDelay;

/// Empty function. Called init time.
- (void)setup;
/// Empty function. Called init time. Sets the json data for this chart.
- (void)setupWithJsonString:(NSString *)jsonString;
/// Empty function. Called init time, after setup and data assign. Create data transformations here.
- (void)setupTransformation;
/// Empty function. Called init time, after setup and data assign. Use data here first.
- (void)fillData;
/// Empty function. Called on layer cleanup, nil layers here.
- (void)clearLayers;

/// Sets the json data for this chart.
- (void)setDataJsonString:(NSString *)jsonString;

/// Indicates whether the diagram has data assigned to it.
- (BOOL)hasData;

/// Draws layers.
- (void)displayLayers;
/// Animate layers.
- (void)animateLayers;

///// Currently active animations.
//@property (strong, nonatomic) NSMutableArray *activeAnimations;
//
///**
// * @brief Returns whether the current chart has running animations.
// * @return True if the chart has animations running, False otherwise.
// */
//- (BOOL)hasRunningAnimations;

@end
