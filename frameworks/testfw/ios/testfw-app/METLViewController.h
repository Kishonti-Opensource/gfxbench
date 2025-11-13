/*
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 
 View Controller for Metal Sample Code. Maintains a CADisplayLink timer that runs on the main thread and triggers rendering in METLView. Provides update callbacks to its delegate on the timer, prior to triggering rendering.
 
 */

#import <UIKit/UIKit.h>
#import "ViewController.h"
#import "graphics/metalgraphicscontext.h"

@interface METALView : UIView

@end

@interface METLViewController : ViewController

@property (retain, nonatomic) IBOutlet METALView *metalview;

@end
