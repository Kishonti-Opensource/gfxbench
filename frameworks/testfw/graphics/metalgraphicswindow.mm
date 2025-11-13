/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "metalgraphicswindow.h"

#include "graphics/metalgraphicscontext.h"
#include "metalmessagequeue.h"

#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_11)

#include <Cocoa/Cocoa.h>
#include <Poco/Exception.h>


//===================================================================
//
//  MetalView Interface
//
//===================================================================


#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#import <Cocoa/Cocoa.h>

@protocol MetalViewDelegate;

@interface MetalView : NSView
@property (nonatomic, weak) id <MetalViewDelegate> delegate;


@property (nonatomic, readonly) CAMetalLayer* metalLayer;


@end

@protocol MetalViewDelegate <NSObject>

@required

- (void)reshape:(MetalView *)view;
- (void)render:(MetalView *)view;

@end


//===================================================================
//
//  MetalView Implementation
//
//===================================================================


@implementation MetalView
{
@public
    CAMetalLayer *_metalLayer;
@private
    
    
    BOOL _layerSizeDidUpdate;
}

- (void)commonInit
{
    _metalLayer = [CAMetalLayer new];
    self.layer = _metalLayer;
    self.wantsLayer = YES;
    
    _metalLayer.opaque          = YES;
    _metalLayer.backgroundColor = CGColorCreateGenericRGB(1.0, 0.0, 0.0, 1.0);
    
    _metalLayer.pixelFormat     = MTLPixelFormatBGRA8Unorm;
    
    _metalLayer.framebufferOnly = YES;
}

- (instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        [self commonInit];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self commonInit];
    }
    return self;
}


@end


//===================================================================
//
//  MTLGraphicsWindow Implementation
//
//===================================================================



class MTLGraphicsWindowImp : public MTLGraphicsWindow
{
public:
    MTLGraphicsWindowImp(int width, int height, const std::vector<tfw::ApiDefinition>& api) ;
    virtual ~MTLGraphicsWindowImp() ;
    
    virtual bool create() ;
    virtual GraphicsContext* getGraphicsContext() ;
	virtual tfw::MessageQueue* getMessageQueue();
    
    virtual int width() ;
    virtual int height() ;
    virtual bool shouldClose() ;
    virtual void pollEvents() ;
    virtual void requestClose() ;
    
protected:
    
    MetalGraphicsContext* m_graphics_context ;
    NSWindow* m_window = nil ;
	MTLMessageQueue *m_msg_queue;
    int m_deviceIndex;
};


MTLGraphicsWindowImp::MTLGraphicsWindowImp(int width, int height, const std::vector<tfw::ApiDefinition>& api) : MTLGraphicsWindow(width,height,api), m_deviceIndex(-1)
{
    m_window = nil ;
    m_graphics_context = nullptr ;
	m_msg_queue = nullptr;
    if(api.size() > 0)
    {
        m_deviceIndex = api[0].deviceIndex();
    }
}


MTLGraphicsWindowImp::~MTLGraphicsWindowImp()
{
    m_window = nil ;
}


bool MTLGraphicsWindowImp::create()
{
    [NSApplication sharedApplication] ;
    
    NSRect frame = NSMakeRect(50, 50, m_width, m_height) ;
    m_window = [[NSWindow alloc] initWithContentRect:frame
                                        styleMask:NSTitledWindowMask
                                        backing:NSBackingStoreBuffered
                                                      defer:NO];
    [m_window setBackgroundColor:[NSColor blueColor]] ;
    
    MetalView *mtl_view = [[MetalView alloc] init] ;
    
    [m_window setContentView:mtl_view] ;
    [m_window makeKeyAndOrderFront:m_window];
    [m_window display];
    
    m_graphics_context = new MetalGraphicsContextImp() ;
	
//	the graphics context should be initialized in testbase by device name
//
//    std::string name = ((MetalGraphicsContextImp*)m_graphics_context)->getDeviceName(m_deviceIndex);
//    ((MetalGraphicsContextImp*)m_graphics_context)->initContext(name);
//    
//    if(!m_graphics_context->isValid())
//    {
//        return false;
//    }
 
    assert(mtl_view.metalLayer) ;
    
    m_graphics_context->setMetalLayer(mtl_view.metalLayer) ;
	
	m_msg_queue = new MTLMessageQueue();
    return true;
}


int MTLGraphicsWindowImp::width()
{
    return m_width ;
}


int MTLGraphicsWindowImp::height()
{
    return m_height ;
}


GraphicsContext* MTLGraphicsWindowImp::getGraphicsContext()
{
    return m_graphics_context ;
}


tfw::MessageQueue* MTLGraphicsWindowImp::getMessageQueue()
{
	return m_msg_queue;
}


bool MTLGraphicsWindowImp::shouldClose()
{
    return false ;
}


void MTLGraphicsWindowImp::pollEvents()
{
	// allways add a mouse event
	NSPoint p = [m_window mouseLocationOutsideOfEventStream];
	m_msg_queue->push_back(m_msg_queue->makeCursorMessage(p.x, -p.y));
	
    NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
	
    if (event == nil)
    {
        return;
    }
    
	m_msg_queue->processEvent(event);
	
    [NSApp sendEvent:event];
}


void MTLGraphicsWindowImp::requestClose()
{
    
}


MTLGraphicsWindow* CreateMTLGraphicsWindow(int width, int height, const std::vector<tfw::ApiDefinition>& api)
{
    if ((width == 0) || (height == 0))
    {
        throw Poco::Exception("In case of mtl graphics context the width and height must be specified. Please use '-w' and '-h'");
    }
    
    return new MTLGraphicsWindowImp(width,height,api) ;
}

#endif

