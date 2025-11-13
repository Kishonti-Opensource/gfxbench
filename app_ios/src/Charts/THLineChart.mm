/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Charts/THLineChart.h"
#import "Common/THTheme.h"


# pragma mark - LineLayer

#if __IPHONE_OS_VERSION_MAX_ALLOWED > __IPHONE_9_3
@interface THLineLayer : CALayer<CALayerDelegate>
#else
@interface THLineLayer : CALayer
#endif

@property (nonatomic, strong) UIColor *color;
@property (nonatomic, strong) NSArray *rValues;
@property (nonatomic, strong) NSArray *dValues;

@property (nonatomic, assign) float rMin;
@property (nonatomic, assign) float rMax;
@property (nonatomic, assign) float dMin;
@property (nonatomic, assign) float dMax;
@property (nonatomic, assign) float lineWidth;

@property (nonatomic, assign) CGAffineTransform gridTrafo;

@property (nonatomic, assign) BOOL displayGradient;

@end
@implementation THLineLayer

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)context {
    if(context == nil) return;

    if([layer isKindOfClass:[THLineLayer class]]) {
        [self drawLineLayer:(THLineLayer *)layer inContext:context];
    }
}

//- (float)transformRValue:(float)v {
//    return (v - self.rMin) / (self.rMax - self.rMin) * self.frame.size.height;
//}
//
//- (float)transformDValue:(float)v {
//    return (v - self.dMin) / (self.dMax - self.dMin) * self.frame.size.width;
//}

- (void)drawLineLayer:(THLineLayer*)layer inContext:(CGContextRef)context {
    UIGraphicsPushContext(context);
    CGContextSaveGState(context);

    if(self.displayGradient)
        [self drawLineGradientsToCGContext:context
                                     Color:layer.color
                                   RValues:layer.rValues
                                   DValues:layer.dValues];

    [self drawLineValuesToCGContext:context
                              Color:layer.color
                             RValues:layer.rValues
                             DValues:layer.dValues];

    CGContextRestoreGState(context);
    UIGraphicsPopContext();
}

- (void)drawLineGradientsToCGContext: (CGContextRef) context
                               Color: (UIColor *)color
                              RValues: (NSArray *)rValues
                              DValues: (NSArray *)dValues
{
    CGContextSetStrokeColorWithColor(context, [color CGColor]);
    CGContextSetLineWidth(context, self.lineWidth);

    CGFloat r, g, b, a;
    [color getRed:&r green:&g blue:&b alpha:&a];

    CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
    CGFloat colors[] =
    {
        r, g, b, 0.5,
        r, g, b, 0.05,
    };
    CGGradientRef gradient = CGGradientCreateWithColorComponents(rgb, colors, NULL, sizeof(colors)/(sizeof(colors[0])*4));
    CGColorSpaceRelease(rgb);

    CGContextSaveGState(context);


    NSUInteger count = rValues.count;
    if(count < 2) return;

    CGContextMoveToPoint(context,
                         0,
                         self.frame.size.height);

    for(int i = 0; i<count; ++i)
    {
        if(i>0 && i<count-1)
        {
            CGContextSetLineCap(context, kCGLineCapSquare);
        }
        else
        {
            CGContextSetLineCap(context, kCGLineCapRound);
        }

        CGPoint p = CGPointApplyAffineTransform(CGPointMake([dValues[i] floatValue], [rValues[i] floatValue]), self.gridTrafo);

        CGContextAddLineToPoint(context,
                                p.x,
                                self.frame.size.height - p.y);
    }

    CGContextAddLineToPoint(context,
                            self.frame.size.width,
                            self.frame.size.height);

    CGContextAddLineToPoint(context,
                            0,
                            self.frame.size.height);

    CGContextClip(context);

    CGPoint startPoint = CGPointMake(self.frame.size.width * 0.5f, self.frame.size.height - [[rValues valueForKeyPath:@"@max.floatValue"] floatValue]);
    CGPoint endPoint = CGPointMake(self.frame.size.width * 0.5f, self.frame.size.height);
    CGContextDrawLinearGradient(context, gradient, startPoint, endPoint, 0);
    CGGradientRelease(gradient), gradient = NULL;
    CGContextRestoreGState(context);
}

- (void)drawLineValuesToCGContext: (CGContextRef) context
                            Color: (UIColor *)color
                           RValues: (NSArray *)rValues
                           DValues: (NSArray *)dValues
{
    CGContextSetStrokeColorWithColor(context, [color CGColor]);
    CGContextSetLineWidth(context, self.lineWidth);

    NSUInteger count = rValues.count;
    if(count < 2) return;

    for(int i = 0; i<count; ++i)
    {
        if(i>0 && i<count-1)
        {
            CGContextSetLineCap(context, kCGLineCapSquare);
        }
        else
        {
            CGContextSetLineCap(context, kCGLineCapRound);
        }


        CGPoint p = CGPointApplyAffineTransform(CGPointMake([dValues[i] floatValue], [rValues[i] floatValue]), self.gridTrafo);

        if(i == 0) {
            CGContextMoveToPoint(context,
                                 p.x,
                                 (self.frame.size.height - p.y));
        } else {
            CGContextAddLineToPoint(context,
                                    p.x,
                                    (self.frame.size.height - p.y));
        }
    }

    CGContextStrokePath(context);
}


@end

#pragma mark - LineDiagram

@interface THLineChart()

@property (nonatomic, strong) NSMutableArray *valueLayers;

@end

@implementation THLineChart

- (void)setup {
    [super setup];
    self.valueLineWidth = [THTheme getFloatNamed:@"LineChart_ValueLineWidth"];
    self.displayGradients = false;

    if(self.valueLayers == nil) {
        self.valueLayers = [[NSMutableArray alloc] init];
    }
}

- (void)fillData {
    [super fillData];

//    NSArray *domainValues = self.data[@"domain"][@"values"];
//    float dMin = [[domainValues valueForKeyPath:@"@min.floatValue"] floatValue];
//    float dMax = [[domainValues valueForKeyPath:@"@max.floatValue"] floatValue];
//    float rMin = FLT_MAX;
//    float rMax = FLT_MIN;
//    for (NSDictionary *XSapmles in self.data[@"values"]) {
//        NSArray *values = XSapmles[@"values"];
//        rMax = MAX(rMax, [[values valueForKeyPath:@"@max.floatValue"] floatValue]);
//        rMin = MIN(rMin, [[values valueForKeyPath:@"@min.floatValue"] floatValue]);
//    }

    NSInteger color_count = [THTheme getIntegerNamed:@"Diagram_Value_Count"];
    for (NSInteger i = 0; i < self.sortedValues.count; ++i) {
        NSString *colorString = [NSString stringWithFormat:@"Diagram_Values_%ld", (long)(i%color_count)];
        UIColor *color = [THTheme getColorNamed:colorString];

        THLineLayer *lineLayer = [THLineLayer layer];

        lineLayer.delegate = lineLayer;

        [lineLayer setColor:color];
        [lineLayer setRValues:self.sortedValues[i][@"values"]];
        [lineLayer setDValues:self.data[@"domain"][@"values"]];
        [lineLayer setRMin:self.rMin];
        [lineLayer setRMax:self.rMax];
        [lineLayer setDMin:self.dMin];
        [lineLayer setDMax:self.dMax];
        [lineLayer setGridTrafo:self.gridTransform];
        [lineLayer setLineWidth:self.valueLineWidth];
        [lineLayer setDisplayGradient:self.displayGradients];
        [lineLayer setAnchorPoint:CGPointMake(0, 1)];
        lineLayer.contentsScale = [[UIScreen mainScreen] scale];
        [self.valueLayers addObject:lineLayer];
        [self.layer addSublayer:lineLayer];
    }
}

- (void) displayLayers {

    [super displayLayers];


    CGRect insetted = [self getChartRect];

    for (THLineLayer *lineLayer in self.valueLayers) {
        lineLayer.frame = insetted;

        CAShapeLayer *maskLayer = [[CAShapeLayer alloc] init];
        CGRect maskRect = CGRectMake(0, 0, lineLayer.frame.size.width, lineLayer.frame.size.height);
        CGPathRef path = CGPathCreateWithRect(maskRect, NULL);
        maskLayer.path = path;
        CGPathRelease(path);

        // Set the mask of the view.
        lineLayer.mask = maskLayer;
        lineLayer.mask.anchorPoint = CGPointMake(0, 0.5);
        [lineLayer setGridTrafo:self.gridTransform];

        [lineLayer setNeedsDisplay];
    }
}

- (void)animateLayers {
    [super animateLayers];
    for (int layerIndex = 0; layerIndex < self.valueLayers.count; ++layerIndex) {
        THLineLayer *lineLayer = self.valueLayers[layerIndex];
        if(self.animationTime > 0) {

            CABasicAnimation* prerevealAnimation = [CABasicAnimation animationWithKeyPath:@"transform.scale.x"];
            prerevealAnimation.fromValue = [NSNumber numberWithFloat:0];
            prerevealAnimation.toValue = [NSNumber numberWithFloat:0];
            [prerevealAnimation setBeginTime:CACurrentMediaTime()];
            [prerevealAnimation setDuration:0];
            [prerevealAnimation setRemovedOnCompletion:NO];
            [prerevealAnimation setFillMode:kCAFillModeForwards];
            [prerevealAnimation setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
            [lineLayer.mask addAnimation:prerevealAnimation forKey:@"prerevealAnimation"];

            CABasicAnimation* revealAnimation = [CABasicAnimation animationWithKeyPath:@"transform.scale.x"];
            revealAnimation.fromValue = [NSNumber numberWithFloat:0];
            revealAnimation.toValue = [NSNumber numberWithFloat:1];
            [revealAnimation setBeginTime:CACurrentMediaTime() + self.startAnimationDelay + self.animationDelay * layerIndex];
            [revealAnimation setDuration:self.animationTime];
            [revealAnimation setRemovedOnCompletion:NO];
            [revealAnimation setFillMode:kCAFillModeForwards];
            [revealAnimation setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];

            [lineLayer.mask addAnimation:revealAnimation forKey:@"revealAnimation"];
        }
    }
}

- (void)clearLayers {
    [super clearLayers];
    for (THLineLayer *lineLayer in self.valueLayers) {
        [lineLayer removeFromSuperlayer];
    }

    [self.valueLayers removeAllObjects];
}


# pragma mark Setters

- (void) setDisplayGradients:(BOOL)displayGradients {
    _displayGradients = displayGradients;

    for (THLineLayer *layer in self.valueLayers) {
        [layer setDisplayGradient:displayGradients];
    }
}

- (void) setValueLineWidth:(float)valueLineWidth {
    _valueLineWidth = valueLineWidth;

    for (THLineLayer *layer in self.valueLayers) {
        [layer setLineWidth:valueLineWidth];
    }
}

- (void)modifyViewMatrix {
    [super modifyViewMatrix];


    for (THLineLayer *lineLayer in self.valueLayers) {
        [lineLayer setGridTrafo:self.gridTransform];
        [lineLayer setNeedsDisplay];
    }
}

@end
