/*
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 
 View Controller for Metal Sample Code. Maintains a CADisplayLink timer that runs on the main thread and triggers rendering in METLView. Provides update callbacks to its delegate on the timer, prior to triggering rendering.
 
 */

#import "METLViewController.h"
#import "NSFileManagerLocations.h"
#import "ng/log.h"
#import "testfw.h"
#import "schemas/descriptors.h"
#import <QuartzCore/CAMetalLayer.h>


@implementation METALView

+ (Class)layerClass
{
    return [CAMetalLayer class];
}

@end

@implementation METLViewController

- (void)initContext
{
	// the graphics context should be initialized in testbase by device name
    ctx_ = new MetalGraphicsContextImp();
}

- (void)setupMainViewWithContext:(GraphicsContext *)context
{
#if !(TARGET_IPHONE_SIMULATOR)
	if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
	{
		self.metalview.contentScaleFactor = [[UIScreen mainScreen] nativeScale];
	}
	else if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
	{
		self.metalview.contentScaleFactor = [[UIScreen mainScreen] scale];
	}
	((MetalGraphicsContext *)ctx_)->setMetalLayer((CAMetalLayer*)self.metalview.layer);
#endif
    [self.metalview setOpaque:true] ;
    [self.metalview setBackgroundColor:[UIColor blackColor]] ;
}

- (void)setupEnv:(tfw::Descriptor *)des
{
    [super setupEnv:des];
    des->env().setHeight(self.metalview.bounds.size.height * self.metalview.contentScaleFactor);
    des->env().setWidth(self.metalview.bounds.size.width * self.metalview.contentScaleFactor);
}

- (void)dealloc {
    delete ctx_;
    [_metalview release];
    [super dealloc];
}

- (void)viewDidUnload {
    [self setMetalview:nil];
    [super viewDidUnload];
}
@end
