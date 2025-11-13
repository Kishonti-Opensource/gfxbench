/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Controls/THProgressBar.h"
#import "Common/THTheme.h"

@interface ProgressLayer: CAShapeLayer

@property (nonatomic, assign) float startPercent;
@property (nonatomic, assign) float endPercent;
@property (nonatomic, assign) bool infinite;
@property (nonatomic, assign) bool isRounded;
@property (nonatomic, strong) UIColor *startColor;
@property (nonatomic, strong) UIColor *endColor;

@end

@implementation ProgressLayer

@dynamic startPercent;
@dynamic endPercent;

- (id)init
{
    if ((self = [super init]))
    {
        self.infinite = false;
        self.isRounded = false;
        self.startPercent = 0;
        self.endPercent = 0;
        self.startColor = [UIColor blueColor];
        self.endColor = [UIColor blueColor];
    }
    return self;
}

+ (BOOL)needsDisplayForKey:(NSString *)key
{
    if ([@"startPercent" isEqualToString:key] ||
        [@"endPercent" isEqualToString:key])
    {
        return YES;
    }
    return [super needsDisplayForKey:key];
}

- (void)display
{
    //get interpolated time value
    float start = [[self presentationLayer] startPercent];
    float end = [[self presentationLayer] endPercent];

    //create drawing context
    UIGraphicsBeginImageContextWithOptions(self.bounds.size, NO, 0);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);

    // Create foreground gradient
    CGFloat sr, sg, sb, sa;
    CGFloat er, eg, eb, ea;

    [self.startColor getRed:&sr green:&sg blue:&sb alpha:&sa];
    [self.endColor getRed:&er green:&eg blue:&eb alpha:&ea];
    CGFloat colors [] = {
        sr, sg, sb, sa,
        er, eg, eb, ea
    };

    CGColorSpaceRef baseSpace = CGColorSpaceCreateDeviceRGB();
    CGGradientRef gradient = CGGradientCreateWithColorComponents(baseSpace, colors, NULL, 2);

    // Setup gradient direction
    CGPoint startPoint = CGPointMake(self.bounds.size.width * start, self.bounds.size.height * 0.5f);
    CGPoint endPoint = CGPointMake(self.bounds.size.width * end, self.bounds.size.height * 0.5f);


    if(self.isRounded) {
        CGFloat startCol [] = {sr, sg, sb, sa};
        CGFloat endCol [] = {er, eg, eb, ea};

        CGColorRef color = CGColorCreate(baseSpace, endCol);
        CGContextSetFillColorWithColor(ctx, color);
        CGContextAddArc(ctx, endPoint.x - endPoint.y, endPoint.y, endPoint.y,  M_PI_2, -M_PI_2, true);
        CGContextClosePath(ctx);
        CGContextFillPath(ctx);
        CGColorRelease(color);

        if(endPoint.x > startPoint.x + startPoint.y + endPoint.y) {
            color = CGColorCreate(baseSpace, startCol);
            CGContextSetFillColorWithColor(ctx, color);
            CGContextAddArc(ctx, startPoint.x + startPoint.y, startPoint.y, startPoint.y,  M_PI_2, 3*M_PI_2, false);
            CGContextClosePath(ctx);
            CGContextFillPath(ctx);
            CGColorRelease(color);

            CGContextDrawLinearGradient(ctx, gradient, CGPointMake(startPoint.x + startPoint.y, startPoint.y), CGPointMake(endPoint.x - endPoint.y + 0.5f, endPoint.y), 0);

        } else if(endPoint.x > startPoint.x) {
            CGContextDrawLinearGradient(ctx, gradient, CGPointMake(startPoint.x, startPoint.y), CGPointMake(endPoint.x - endPoint.y + 0.5f, endPoint.y), 0);
        }

    } else {
        // Fill gradient rectangle based on progress
        CGContextDrawLinearGradient(ctx, gradient, startPoint, endPoint, 0);
    }

    CGGradientRelease(gradient);
    CGColorSpaceRelease(baseSpace);
    CGContextRestoreGState(ctx);

    //set backing image
    self.contents = (id)UIGraphicsGetImageFromCurrentImageContext().CGImage;
    UIGraphicsEndImageContext();
}

- (id<CAAction>)actionForKey:(NSString *)key
{
    if ([key isEqualToString:@"startPercent"])
    {
        CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:key];
        animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
        animation.fromValue = @([[self presentationLayer] startPercent]);
        return animation;
    }
    if ([key isEqualToString:@"endPercent"])
    {
        CABasicAnimation *animation = [CABasicAnimation animationWithKeyPath:key];
        animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
        animation.fromValue = @([[self presentationLayer] endPercent]);
        return animation;
    }
    return [super actionForKey:key];
}

- (void) setInfinite:(bool)infinite {
    _infinite = infinite;
    if(infinite) {
        CABasicAnimation *endAnimation = [CABasicAnimation animationWithKeyPath:@"endPercent"];
        endAnimation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
        endAnimation.fromValue = @(0);
        endAnimation.toValue = @(1.4);
        endAnimation.duration = 1.5;

        CABasicAnimation *startAnimation = [CABasicAnimation animationWithKeyPath:@"startPercent"];
        startAnimation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionLinear];
        startAnimation.fromValue = @(-0.4);
        startAnimation.toValue = @(1);
        startAnimation.duration = 1.5;

        CAAnimationGroup *group = [CAAnimationGroup animation];
        group.duration = 1.5;
        group.repeatCount = 10000;
        group.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
        group.animations = @[endAnimation, startAnimation];

        [self addAnimation:group forKey:@"infiniteAnimation"];

    } else {
        self.endPercent = [self.presentationLayer endPercent];
        self.startPercent = [self.presentationLayer startPercent];
        [self removeAnimationForKey:@"infiniteAnimation"];
    }
}

@end




@interface THProgressBar()

@property (nonatomic, strong) CALayer *backgroundLayer;
@property (nonatomic, strong) ProgressLayer *foregroundLayer;
@property (assign, nonatomic) CGFloat infiniteProgress;

@end

@implementation THProgressBar

- (id) init {
    self = [super init];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (void) setupWiTHTheme {
    // Setup default things
    self.progress = 0;
    self.style = THProgressbarStyleNormal;
    self.padding = [THTheme getFloatNamed:@"Progressbar_Padding_Default"];
    self.isInfinite = false;
    self.isRounded = false;
    self.backgroundColor = [UIColor redColor];

    // Setup layers only once
    if(self.backgroundLayer == nil || self.foregroundLayer == nil)
    {
        // Setup background layer, default color
        self.backgroundLayer = [CALayer layer];
        [self.backgroundLayer setBackgroundColor:[THTheme getColorNamed:@"Progressbar_Background_Default"].CGColor];
        self.backgroundLayer.contentsScale = [[UIScreen mainScreen] scale];
        [self.layer addSublayer:self.backgroundLayer];

        // Setup foreground layer, set colors
        self.foregroundLayer = [ProgressLayer layer];
        self.foregroundLayer.name = @"foregroundLayer";
        [self.foregroundLayer setStartColor:[THTheme getColorNamed:@"Progressbar_Foreground_Default"]];
        [self.foregroundLayer setEndColor:[THTheme getColorNamed:@"Progressbar_Foreground_Default"]];
        self.foregroundLayer.contentsScale = [[UIScreen mainScreen] scale];
        [self.layer addSublayer:self.foregroundLayer];
    }

    // Call for layout
    [self layoutLayers];
}

- (void) setFrame:(CGRect)frame {
    [super setFrame:frame];

    // Call for layout
    [self layoutLayers];
}

- (void) updateConstraints {
    [super updateConstraints];

    // Call for layout
    [self layoutLayers];
}

- (void) updateConstraintsIfNeeded {
    [super updateConstraintsIfNeeded];

    // Call for layout
    [self layoutLayers];
}

- (void) layoutSubviews {
    [super layoutSubviews];

    // Call for layout
    [self layoutLayers];
}

- (void) layoutLayers {
    // Setup layer frames based on current size
    CGRect backgroundRect = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
    CGRect foregroundRect = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);

    // If the style is Padded, use padded rect for foreground layer
    if (self.style == THProgressbarStylePadded) {
        foregroundRect = CGRectMake(self.padding,
                                    self.padding,
                                    self.frame.size.width - 2*self.padding,
                                    self.frame.size.height - 2*self.padding);

    // If style is Underlined setup different rect for both layers based on padding
    } else if (self.style == THProgressbarStyleUnderlined) {
        backgroundRect = CGRectMake(backgroundRect.origin.x,
                                    backgroundRect.origin.y + backgroundRect.size.height - self.padding,
                                    backgroundRect.size.width,
                                    self.padding);
        foregroundRect = CGRectMake(foregroundRect.origin.x,
                                    foregroundRect.origin.y,
                                    foregroundRect.size.width,
                                    foregroundRect.size.height - self.padding);
    }

    // Update layer frames
    if(self.backgroundLayer != nil) {
        self.backgroundLayer.frame = backgroundRect;
        [self.backgroundLayer setNeedsDisplay];
    }

    if(self.foregroundLayer != nil) {
        self.foregroundLayer.frame = foregroundRect;
        [self.foregroundLayer setNeedsDisplay];
    }

    if(_isRounded) {
        [self.layer setCornerRadius:self.frame.size.height*0.5f];
        [self.layer setMasksToBounds:true];
    } else {
        [self.layer setCornerRadius:0];
        [self.layer setMasksToBounds:false];
    }
}

- (void)drawRect:(CGRect)rect {
}

- (void)setProgress:(float)p {
    _progress = MAX(0, MIN(p,1));
    [self.foregroundLayer setStartPercent:0];
    [self.foregroundLayer setEndPercent:_progress];
}

- (void)setStyle:(THProgressbarStyle)s {
    _style = s;
    [self layoutLayers];
    [self setNeedsDisplay];
}

- (void)setForegroundColor:(UIColor *)color {
    [self.foregroundLayer setStartColor:color];
    [self.foregroundLayer setEndColor:color];
}

- (void)setForegroundStartColor:(UIColor *)color {
    [self.foregroundLayer setStartColor:color];
}

- (void)setForegroundEndColor:(UIColor *)color {
    [self.foregroundLayer setEndColor:color];
}

- (void)setBackgroundColor:(UIColor *)color {
    self.backgroundLayer.backgroundColor = color.CGColor;

    [self.backgroundLayer setNeedsDisplay];
    [self setNeedsDisplay];
}

- (void)setIsInfinite:(BOOL)isInfinite {
    _isInfinite = isInfinite;
    [self.foregroundLayer setInfinite:_isInfinite];
}

- (void) setIsRounded:(BOOL)isRounded {
    _isRounded = isRounded;

    if(_isRounded) {
        [self.foregroundLayer setIsRounded:true];
        [self.layer setCornerRadius:self.frame.size.height*0.5f];
        [self.layer setMasksToBounds:true];
    } else {
        [self.foregroundLayer setIsRounded:false];
        [self.layer setCornerRadius:0];
        [self.layer setMasksToBounds:false];
    }

    [self.foregroundLayer setNeedsDisplay];
    [self.backgroundLayer setNeedsDisplay];
    [self setNeedsDisplay];
}

@end
