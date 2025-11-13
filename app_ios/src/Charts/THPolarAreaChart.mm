/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Charts/THPolarAreaChart.h"
#import "Common/THTheme.h"



# pragma mark - PolarLayer

#if __IPHONE_OS_VERSION_MAX_ALLOWED > __IPHONE_9_3
@interface THPolarLayer : CALayer<CALayerDelegate>
#else
@interface THPolarLayer : CALayer
#endif

@property (nonatomic, strong) UIColor *color;
@property (nonatomic, assign) CGPoint center;
@property (nonatomic, assign) CGRect rect;
@property (nonatomic, assign) float radius;
@property (nonatomic, assign) float value;
@property (nonatomic, assign) float dir;
@property (nonatomic, assign) float arcSize;
@property (nonatomic, assign) BOOL isBackground;

@end
@implementation THPolarLayer

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)context {
    if(context == nil) return;

    if([layer isKindOfClass:[THPolarLayer class]]) {
        [self drawPolarLayer:(THPolarLayer *)layer inContext:context];
    }
}

- (void)drawPolarLayer:(THPolarLayer*)layer inContext:(CGContextRef)context {
    UIGraphicsPushContext(context);

    CGFloat h, s, b, a;
    CGFloat dr, dg, db;
    CGFloat sr, sg, sb, sa;
    [layer.color getHue:&h saturation:&s brightness:&b alpha:&a];


    CGContextSaveGState(context);

    if(layer.isBackground) {
        // clip to slice
        UIBezierPath* clip = [UIBezierPath bezierPathWithArcCenter:layer.center
                                                            radius:layer.radius
                                                        startAngle:-layer.dir + layer.arcSize/2.0f
                                                          endAngle:-layer.dir - layer.arcSize/2.0f
                                                         clockwise:NO];
        [clip addLineToPoint:layer.center];
        [clip closePath];
        [clip addClip];

        // fill subtle background on slice
        UIColor *saturatedColor = [UIColor colorWithHue:h saturation:s*0.7 brightness:b alpha:0.2];
        CGContextSetFillColorWithColor(context, [saturatedColor CGColor]);
        CGContextFillEllipseInRect(context, layer.rect);


    } else {
        // clip to value slice
        float temp_radius = layer.radius * layer.value;
        UIBezierPath* clip = [UIBezierPath bezierPathWithArcCenter:layer.center
                                              radius:temp_radius
                                          startAngle:-layer.dir + layer.arcSize/2.0f
                                            endAngle:-layer.dir - layer.arcSize/2.0f
                                           clockwise:NO];
        [clip addLineToPoint:layer.center];
        [clip closePath];
        [clip addClip];

        // fill with true fill color
        UIColor *saturatedColor = [UIColor colorWithHue:h saturation:s*0.7 brightness:b alpha:1.0];
        [saturatedColor getRed:&sr green:&sg blue:&sb alpha:&sa];
        [layer.color getRed:&dr green:&dg blue:&db alpha:&a];
        CGFloat colors [] = {
            dr, dg, db, 1.0,
            sr, sg, sb, 1.0
        };

        CGColorSpaceRef baseSpace = CGColorSpaceCreateDeviceRGB();
        CGGradientRef gradient = CGGradientCreateWithColorComponents(baseSpace, colors, NULL, 2);
        CGColorSpaceRelease(baseSpace);

        CGPoint startPoint = CGPointMake(layer.center.x, layer.center.y);
        CGPoint endPoint = CGPointMake(layer.center.x + cos(-layer.dir) * layer.radius, layer.center.y + sin(-layer.dir) * layer.radius);


        CGContextDrawLinearGradient(context, gradient, startPoint, endPoint, 0);


        //draw shadow on the slices sides
        UIColor *darkerColor = [UIColor colorWithHue:h saturation:s brightness:b*0.9 alpha:1.0];
        [darkerColor getRed:&sr green:&sg blue:&sb alpha:&a];
        CGFloat colorsShadow [] = {
            sr, sg, sb, 0.7,
            dr, dg, db, 0.0
        };
        CGColorSpaceRef baseSpaceShadow = CGColorSpaceCreateDeviceRGB();
        CGGradientRef gradientShadow = CGGradientCreateWithColorComponents(baseSpaceShadow, colorsShadow, NULL, 2);
        CGColorSpaceRelease(baseSpaceShadow);

        CGPoint shadowDir = CGPointMake(cos(-layer.dir + layer.arcSize/2.0f), sin(-layer.dir + layer.arcSize/2.0f));
        CGPoint shadowRotatedDir = CGPointMake(sin(-layer.dir + layer.arcSize/2.0f), -cos(-layer.dir + layer.arcSize/2.0f));
        CGPoint shadowDarkMid = CGPointMake(layer.center.x + shadowDir.x * layer.radius/2.0f, layer.center.y + shadowDir.y * layer.radius/2.0f);
        CGPoint shadowLightMid = CGPointMake(shadowDarkMid.x + shadowRotatedDir.x * 30, shadowDarkMid.y + shadowRotatedDir.y * 30);

        startPoint = CGPointMake(shadowDarkMid.x, shadowDarkMid.y);
        endPoint = CGPointMake(shadowLightMid.x, shadowLightMid.y);

        CGContextDrawLinearGradient(context, gradientShadow, startPoint, endPoint, 0);

        shadowDir = CGPointMake(cos(-layer.dir - layer.arcSize/2.0f), sin(-layer.dir - layer.arcSize/2.0f));
        shadowRotatedDir = CGPointMake(-sin(-layer.dir - layer.arcSize/2.0f), cos(-layer.dir - layer.arcSize/2.0f));
        shadowDarkMid = CGPointMake(layer.center.x + shadowDir.x * layer.radius/2.0f, layer.center.y + shadowDir.y * layer.radius/2.0f);
        shadowLightMid = CGPointMake(shadowDarkMid.x + shadowRotatedDir.x * 20, shadowDarkMid.y + shadowRotatedDir.y * 20);

        startPoint = CGPointMake(shadowDarkMid.x, shadowDarkMid.y);
        endPoint = CGPointMake(shadowLightMid.x, shadowLightMid.y);

        CGContextDrawLinearGradient(context, gradientShadow, startPoint, endPoint, 0);
    }


    CGContextRestoreGState(context);
    UIGraphicsPopContext();
}

@end



# pragma mark - SeparatorLayer

@interface THSeparatorLayer : CALayer

@property (nonatomic, strong) UIColor *color;
@property (nonatomic, assign) CGPoint center;
@property (nonatomic, assign) CGRect rect;
@property (nonatomic, assign) float radius;
@property (nonatomic, assign) NSUInteger slices;
@property (nonatomic, assign) int lineWidth;

@end
@implementation THSeparatorLayer

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)context {
    if(context == nil) return;

    if([layer isKindOfClass:[THSeparatorLayer class]]) {
        [self drawSeparatorLayer:(THSeparatorLayer *)layer inContext:context];
    }
}

- (void)drawSeparatorLayer:(THSeparatorLayer*)layer inContext:(CGContextRef)context {
    UIGraphicsPushContext(context);
    CGContextSaveGState(context);

    CGContextSetStrokeColorWithColor(context, [layer.color CGColor]);
    CGContextSetLineWidth(context, layer.lineWidth);

    for(int i = 0; i < layer.slices; ++i) {
        float dir = (2*M_PI/layer.slices) * i - M_PI_2;

        CGContextMoveToPoint(context, layer.center.x, layer.center.y);
        CGContextAddLineToPoint(context, layer.center.x + cos(dir) * layer.radius, layer.center.y + sin(dir) * layer.radius);
        CGContextStrokePath(context);
    }

    CGContextRestoreGState(context);
    UIGraphicsPopContext();
}

@end



# pragma mark - LabelLayer

@interface THLabelLayer : CALayer

@property (nonatomic, strong) UIColor *color;
@property (nonatomic, assign) CGPoint center;
@property (nonatomic, assign) float radius;
@property (nonatomic, assign) float dir;
@property (nonatomic, copy) NSString *text;

@end
@implementation THLabelLayer

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)context {
    if(context == nil) return;

    if([layer isKindOfClass:[THLabelLayer class]]) {
        [self drawLabelLayer:(THLabelLayer *)layer inContext:context];
    }
}

- (void)drawLabelLayer:(THLabelLayer*)layer inContext:(CGContextRef)context {
    UIGraphicsPushContext(context);
    CGContextSaveGState(context);

    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    textStyle.alignment = NSTextAlignmentCenter;
    NSDictionary* textFontAttributes = @{NSFontAttributeName: [THTheme getFontNamed:@"PieDiagram_Text_Font"], NSForegroundColorAttributeName: layer.color, NSParagraphStyleAttributeName: textStyle};

    CGSize textSize = [layer.text sizeWithAttributes:textFontAttributes];
    CGRect rect = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
    CGPoint p = CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect));
    CGRect textRect = CGRectMake(p.x - textSize.width*0.5f, p.y - textSize.height*0.5f, textSize.width, textSize.height);
    [layer.text drawInRect:textRect withAttributes:textFontAttributes];

    CGContextRestoreGState(context);
    UIGraphicsPopContext();
}

@end




# pragma mark - PolarAreaDiagram

@interface THPolarAreaChart()

@property (nonatomic, strong) NSMutableArray *backPolarLayers;
@property (nonatomic, strong) NSMutableArray *polarLayers;
@property (nonatomic, strong) NSMutableArray *labelLayers;
@property (nonatomic, strong) THSeparatorLayer *separatorLayer;

@end

@implementation THPolarAreaChart {
    NSUInteger numberOfSlices;
}

- (void)setup {
    [super setup];

    [self setBackgroundColor:[THTheme getColorNamed:@"Diagram_Background"]];
    self.layer.backgroundColor = [THTheme getColorNamed:@"Diagram_Background"].CGColor;
    self.separatorWidth = [THTheme getFloatNamed:@"PolarAreaChart_SeparatorWidth"];
    self.textDistance = [THTheme getFloatNamed:@"PolarAreaChart_LabelDistance"];
    self.animationTime = [THTheme getFloatNamed:@"PolarAreaChart_AnimationTime"];
    self.animationDelay = [THTheme getFloatNamed:@"PolarAreaChart_AnimationDelay"];

    if(self.polarLayers == nil) {
        self.polarLayers = [[NSMutableArray alloc] init];
    }
    if(self.backPolarLayers == nil) {
        self.backPolarLayers = [[NSMutableArray alloc] init];
    }
    if(self.labelLayers == nil) {
        self.labelLayers = [[NSMutableArray alloc] init];
    }
}

- (void)setupTransformation {
}

- (void)fillData {
    [super fillData];
    numberOfSlices = ((NSArray *)self.data[@"values"]).count;

    if(self.backPolarLayers.count < numberOfSlices) {
        NSInteger color_count = [THTheme getIntegerNamed:@"Diagram_Value_Count"];
        for (int i = 0; i < numberOfSlices; ++i) {
            float dir = (2*M_PI/numberOfSlices) * i - M_PI_2;
            float arcSize = 2*M_PI/numberOfSlices;

            NSString *colorString = [NSString stringWithFormat:@"Diagram_Values_%ld", (long)(i%color_count)];
            UIColor *color = [THTheme getColorNamed:colorString];

            THPolarLayer *backpolarLayer = [THPolarLayer layer];
            backpolarLayer.delegate = backpolarLayer;
            [backpolarLayer setIsBackground:true];
            [backpolarLayer setColor:color];
            [backpolarLayer setValue:[self.data[@"values"][i][@"values"][0] floatValue]];
            [backpolarLayer setDir:dir];
            [backpolarLayer setArcSize:arcSize];
            backpolarLayer.contentsScale = [[UIScreen mainScreen] scale];

            [self.backPolarLayers addObject:backpolarLayer];
            [self.layer addSublayer:backpolarLayer];

            THPolarLayer *polarLayer = [THPolarLayer layer];
            polarLayer.delegate = polarLayer;
            [polarLayer setIsBackground:false];
            [polarLayer setColor:color];
            [polarLayer setValue:[self.data[@"values"][i][@"values"][0] floatValue]];
            [polarLayer setDir:dir];
            [polarLayer setArcSize:arcSize];
            polarLayer.contentsScale = [[UIScreen mainScreen] scale];

            [self.polarLayers addObject:polarLayer];
            [self.layer addSublayer:polarLayer];

            THLabelLayer *labelLayer = [THLabelLayer layer];
#ifndef __IPHONE_10_0
            labelLayer.delegate = labelLayer;
#endif
            [labelLayer setColor:color];
            [labelLayer setDir:dir];
            [labelLayer setText:[self getLabelForIndex:i]];
            labelLayer.contentsScale = [[UIScreen mainScreen] scale];

            [self.labelLayers addObject:labelLayer];
            [self.layer addSublayer:labelLayer];
        }

        self.separatorLayer = [THSeparatorLayer layer];
#ifndef __IPHONE_10_0
        self.separatorLayer.delegate = self.separatorLayer;
#endif
        [self.separatorLayer setLineWidth:self.separatorWidth];
        [self.layer addSublayer:self.separatorLayer];
        [self.separatorLayer setColor:self.backgroundColor];
        self.separatorLayer.contentsScale = [[UIScreen mainScreen] scale];
    }
}


- (NSString *)getLabelForIndex:(int)index {
    return [NSString stringWithFormat:@"%@ \n %.0f%%",
            self.data[@"values"][index][@"name"],
            [self.data[@"values"][index][@"values"][0] doubleValue] * 100];
}

- (void)clearLayers {
    [super clearLayers];
    for (CALayer *layer in self.polarLayers) {
        [layer removeFromSuperlayer];
    }
    for (CALayer *layer in self.backPolarLayers) {
        [layer removeFromSuperlayer];
    }
    for (CALayer *layer in self.labelLayers) {
        [layer removeFromSuperlayer];
    }
    [self.separatorLayer removeFromSuperlayer];

    [self.polarLayers removeAllObjects];
    [self.backPolarLayers removeAllObjects];
    [self.labelLayers removeAllObjects];
    self.separatorLayer = nil;
}

- (void)setBackgroundColor:(UIColor *)backgroundColor {
    [super setBackgroundColor:backgroundColor];

    self.layer.backgroundColor = self.backgroundColor.CGColor;
    if(self.separatorLayer != nil)
        [self.separatorLayer setColor:self.backgroundColor];
}

- (void)displayLayers {
    float padding = [THTheme getFloatNamed:@"DefaultPadding"];
    float xTextMinSpace = 100;
    float yTextMinSpace = 40;

    CGRect insetted = CGRectMake(padding + xTextMinSpace,
                                 padding + yTextMinSpace,
                                 self.layer.frame.size.width - 2*xTextMinSpace - 2*padding,
                                 self.layer.frame.size.height - 2*yTextMinSpace - 2*padding);
    CGRect squaredBase = CGRectMake(
                                    insetted.origin.x + (insetted.size.width - MIN(insetted.size.width, insetted.size.height)) * 0.5f,
                                    insetted.origin.y + (insetted.size.height - MIN(insetted.size.width, insetted.size.height)) * 0.5f,
                                    MIN(insetted.size.width, insetted.size.height),
                                    MIN(insetted.size.width, insetted.size.height));

    if(self.backPolarLayers.count > 0) {
        for (THPolarLayer *polarLayer in self.backPolarLayers) {
            polarLayer.frame = squaredBase;
            [polarLayer setRect:CGRectMake(0, 0, squaredBase.size.width, squaredBase.size.height)];
            [polarLayer setCenter:CGPointMake(CGRectGetMidX(polarLayer.rect), CGRectGetMidY(polarLayer.rect))];
            [polarLayer setRadius:squaredBase.size.width*0.5f];
            [polarLayer setNeedsDisplay];
        }
    }

    if(self.polarLayers.count > 0) {
        for (int layerIndex = 0; layerIndex < self.polarLayers.count; ++layerIndex) {
            THPolarLayer *polarLayer = self.polarLayers[layerIndex];
            polarLayer.frame = squaredBase;
            [polarLayer setRect:CGRectMake(0, 0, squaredBase.size.width, squaredBase.size.height)];
            [polarLayer setCenter:CGPointMake(CGRectGetMidX(polarLayer.rect), CGRectGetMidY(polarLayer.rect))];
            [polarLayer setRadius:squaredBase.size.width*0.5f];
            [polarLayer setNeedsDisplay];
        }
    }

    if(self.labelLayers.count > 0) {
        for (int labelIndex = 0; labelIndex < self.labelLayers.count; ++labelIndex) {
            THLabelLayer *labelLayer = self.labelLayers[labelIndex];
            float dir = (2*M_PI/numberOfSlices) * labelIndex - M_PI_2;
            float radius = squaredBase.size.width*0.5f;
            //        float textRadius = sqrt(xTextMinSpace*xTextMinSpace + yTextMinSpace*yTextMinSpace) * 0.2f;

            CGPoint center = CGPointMake(squaredBase.origin.x + radius, squaredBase.origin.y + radius);
            CGPoint labelCenter = CGPointMake(center.x + cos(-dir) * (self.textDistance * radius),
                                              center.y + sin(-dir) * (self.textDistance * radius));

            labelLayer.frame = CGRectMake(labelCenter.x - xTextMinSpace*0.5f,
                                          labelCenter.y - yTextMinSpace*0.5f,
                                          xTextMinSpace,
                                          yTextMinSpace);
            [labelLayer setNeedsDisplay];
        }
    }


    if(self.separatorLayer != nil) {
        self.separatorLayer.frame = squaredBase;
        [self.separatorLayer setSlices:numberOfSlices];
        [self.separatorLayer setRect:CGRectMake(0, 0, squaredBase.size.width, squaredBase.size.height)];
        [self.separatorLayer setCenter:CGPointMake(CGRectGetMidX(self.separatorLayer.rect), CGRectGetMidY(self.separatorLayer.rect))];
        [self.separatorLayer setRadius:squaredBase.size.width*0.5f];
        [self.separatorLayer setNeedsDisplay];
        [self.separatorLayer removeFromSuperlayer];
        [self.layer insertSublayer:self.separatorLayer atIndex:(unsigned int)[self.layer.sublayers count]];
    }

    [super displayLayers];
}

- (void)animateLayers {

    if(self.polarLayers.count > 0) {
        for (int layerIndex = 0; layerIndex < self.polarLayers.count; ++layerIndex) {
            THPolarLayer *polarLayer = self.polarLayers[layerIndex];

            if(self.animationTime > 0) {
                CABasicAnimation *prescale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
                [prescale setFromValue:[NSNumber numberWithFloat:0.0f]];
                [prescale setToValue:[NSNumber numberWithFloat:0.0f]];
                [prescale setBeginTime:CACurrentMediaTime()];
                [prescale setDuration:0];
                [prescale setRemovedOnCompletion:NO];
                [prescale setFillMode:kCAFillModeForwards];
                [prescale setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
                [polarLayer addAnimation:prescale forKey:@"transform.scale.before"];

                CABasicAnimation *scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
                [scale setFromValue:[NSNumber numberWithFloat:0.0f]];
                [scale setToValue:[NSNumber numberWithFloat:1.0f]];
                [scale setBeginTime:CACurrentMediaTime() + self.animationDelay * layerIndex];
                [scale setDuration:self.animationTime];
                [scale setRemovedOnCompletion:NO];
                [scale setFillMode:kCAFillModeForwards];
                [scale setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
                [polarLayer addAnimation:scale forKey:@"transform.scale"];
            }
        }
    }

    if(self.labelLayers.count > 0) {
        for (int labelIndex = 0; labelIndex < self.labelLayers.count; ++labelIndex) {
            THLabelLayer *labelLayer = self.labelLayers[labelIndex];

            if(self.animationTime > 0) {
                labelLayer.opacity = 0;
                CABasicAnimation *scale = [CABasicAnimation animationWithKeyPath:@"opacity"];
                [scale setFromValue:[NSNumber numberWithFloat:0.0f]];
                [scale setToValue:[NSNumber numberWithFloat:1.0f]];
                [scale setBeginTime:CACurrentMediaTime() + self.animationDelay * labelIndex];
                [scale setDuration:self.animationTime];
                [scale setRemovedOnCompletion:NO];
                [scale setFillMode:kCAFillModeForwards];
                [scale setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];
                [labelLayer addAnimation:scale forKey:@"opacity"];
            }
        }
    }

    [super animateLayers];
}

@end

