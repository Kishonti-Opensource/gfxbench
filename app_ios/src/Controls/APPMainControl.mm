/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "APPMainControl.h"
#import <CoreText/CoreText.h>
#import <CoreText/CTFramesetter.h>
#import <CoreText/CTFont.h>

@interface APPMainControl()

@property (strong, nonatomic) CAShapeLayer *mainCircleLayer;
@property (strong, nonatomic) CAShapeLayer *sideCircleLayer;
@property (strong, nonatomic) CAShapeLayer *baseLayer;

@property (strong, nonatomic) CAShapeLayer *longPressedLayer;

@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;
@property (strong, nonatomic) UILongPressGestureRecognizer *longPressRecognizer;

@property (strong, nonatomic) NSLayoutConstraint *mainTextTopConst;
@property (strong, nonatomic) NSLayoutConstraint *mainTextLeftConst;
@property (strong, nonatomic) NSLayoutConstraint *mainTextWidthConst;
@property (strong, nonatomic) NSLayoutConstraint *mainTextHeightConst;

@property (strong, nonatomic) NSLayoutConstraint *sideTextTopConst;
@property (strong, nonatomic) NSLayoutConstraint *sideTextLeftConst;
@property (strong, nonatomic) NSLayoutConstraint *sideTextWidthConst;
@property (strong, nonatomic) NSLayoutConstraint *sideTextHeightConst;


@property (assign, nonatomic) CGFloat circlePadding;
@property (assign, nonatomic) CGFloat mainRadius;
@property (assign, nonatomic) CGFloat sideRadius;
@property (assign, nonatomic) CGPoint mainCenter;
@property (assign, nonatomic) CGPoint sideCenter;

@end

@implementation APPMainControl

- (instancetype)init
{
    self = [super init];
    if (self) {
        [self setup];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self setup];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setup];
    }
    return self;
}

- (void) setup {
    self.mainRadius = 0;
    self.sideRadius = 0;
    self.longPressedLayer = nil;
    self.tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap:)];
    self.longPressRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handlePress:)];

    self.mainText = @"Main sample text with two lines";
    self.sideText = @"Side sample text";

    self.backColor = [UIColor lightGrayColor];
    self.mainColor = [UIColor grayColor];
    self.sideColor = [UIColor blackColor];
    self.mainTextColor = [UIColor blackColor];
    self.sideTextColor = [UIColor whiteColor];

    self.baseLayer = [CAShapeLayer layer];
    self.mainCircleLayer = [CAShapeLayer layer];
    self.sideCircleLayer = [CAShapeLayer layer];

    [self.baseLayer setFillColor:[self.backColor CGColor]];
    [self.mainCircleLayer setFillColor:[self.mainColor CGColor]];
    [self.sideCircleLayer setFillColor:[self.sideColor CGColor]];

    self.mainFontFactor = 36.0f/320.0f;
    self.sideFontFactor = 18.0f/320.0f;

    [[self layer] addSublayer:self.baseLayer];
    [[self layer] addSublayer:self.mainCircleLayer];
    [[self layer] addSublayer:self.sideCircleLayer];

    [self addGestureRecognizer:self.tapRecognizer];
    [self addGestureRecognizer:self.longPressRecognizer];
}

- (void)drawRect:(CGRect)rect {
    CGContextRef context = UIGraphicsGetCurrentContext();

    CGFloat currentPadding = MIN(rect.size.height, rect.size.width) * 0.05f;
    CGFloat sideRadiusX = (rect.size.width - self.layoutMargins.left - 2*currentPadding - self.layoutMargins.right - sin(M_PI_4) * currentPadding) / (3.0f + sin(M_PI_4)*3.0f);
    CGFloat sideRadiusY = (rect.size.height - self.layoutMargins.top - 2*currentPadding - self.layoutMargins.bottom - sin(M_PI_4) * currentPadding) / (3.0f + sin(M_PI_4)*3.0f);

    CGFloat sideRadius = MIN(sideRadiusX, sideRadiusY);
    CGFloat mainRadius = sideRadius*2;

    CGPoint mainCenter = CGPointMake(self.layoutMargins.right + currentPadding + mainRadius,
                                     self.layoutMargins.top + currentPadding + mainRadius);
    CGPoint sideCenter = CGPointMake(rect.size.width - self.layoutMargins.right - currentPadding - sideRadius,
                                     rect.size.height - self.layoutMargins.bottom - currentPadding - sideRadius);

    CGFloat centerDist = sqrt(pow(sideCenter.x - mainCenter.x, 2) + pow(sideCenter.y - mainCenter.y, 2));
    double theta = acos((mainRadius - sideRadius)/centerDist);
    CGVector centerVector = CGVectorMake((sideCenter.x - mainCenter.x) / centerDist,
                                         (sideCenter.y - mainCenter.y) / centerDist);

    CGVector radVector = CGVectorMake(centerVector.dx * cos(theta) - centerVector.dy * sin(theta),
                                      centerVector.dx * sin(theta) + centerVector.dy * cos(theta));

    CGVector radVector2 = CGVectorMake(centerVector.dx * cos(M_PI*2 - theta) - centerVector.dy * sin(M_PI*2 - theta),
                                       centerVector.dx * sin(M_PI*2 - theta) + centerVector.dy * cos(M_PI*2 - theta));

    CGMutablePathRef newBasePath = CGPathCreateMutable();
    CGPathAddArc(newBasePath, nil, self.mainCenter.x, self.mainCenter.y, self.mainRadius+self.circlePadding, atan2(radVector.dx, radVector.dy), atan2(radVector2.dx, radVector2.dy), true);
    CGPathAddArc(newBasePath, nil, self.sideCenter.x, self.sideCenter.y, self.sideRadius+self.circlePadding, atan2(radVector2.dx, radVector2.dy), atan2(radVector.dx, radVector.dy), true);


    CGContextSetLineWidth(context, 2.0);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();

    if (self.backColor != nil) {
        CGColorRef tmpColor = [self.backColor CGColor];
        CGFloat components[4] = {};
        memcpy(components, CGColorGetComponents(tmpColor), sizeof(components));
        CGColorRef color = CGColorCreate(colorspace, components);
        CGContextSetFillColorWithColor(context, color);
        CGContextAddArc(context, mainCenter.x, mainCenter.y, mainRadius+currentPadding,  atan2(radVector.dx, radVector.dy), atan2(radVector2.dx, radVector2.dy), true);
        CGContextAddArc(context, sideCenter.x, sideCenter.y, sideRadius+currentPadding,  atan2(radVector2.dx, radVector2.dy), atan2(radVector.dx, radVector.dy), true);
        CGContextClosePath(context);
        CGContextFillPath(context);
        CGColorRelease(color);
    }

    CGRect mainRect = CGRectMake(
                                 self.layoutMargins.left + currentPadding,
                                 self.layoutMargins.top + currentPadding,
                                 mainRadius * 2,
                                 mainRadius * 2);

    if (self.mainColor != nil) {
        CGColorRef tmpColor = [self.mainColor CGColor];
        CGFloat components[4] = {};
        memcpy(components, CGColorGetComponents(tmpColor), sizeof(components));
        CGColorRef color = CGColorCreate(colorspace, components);
        CGContextSetFillColorWithColor(context, color);
        CGContextAddEllipseInRect(context, mainRect);
    	CGContextFillPath(context);
        CGColorRelease(color);
    }

    if (self.mainTextColor != nil) {
        int mainFontSize = floor(rect.size.width*self.mainFontFactor*0.5f) *2;
        [self drawSubtractedText:self.mainText inRect:mainRect inContext:context withFont:[UIFont systemFontOfSize:mainFontSize] withColor:self.mainTextColor];
    }

    CGRect sideRect = CGRectMake(
                                 rect.size.width - self.layoutMargins.right - currentPadding - sideRadius*2,
                                 rect.size.height - self.layoutMargins.bottom - currentPadding - sideRadius*2,
                                 sideRadius * 2,
                                 sideRadius * 2);


    if (self.sideColor != nil) {
        CGColorRef tmpColor = [self.sideColor CGColor];
        CGFloat components[4] = {};
        memcpy(components, CGColorGetComponents(tmpColor), sizeof(components));
        CGColorRef color = CGColorCreate(colorspace, components);
        CGContextSetFillColorWithColor(context, color);
        CGContextAddEllipseInRect(context, sideRect);
        CGContextFillPath(context);
        CGColorRelease(color);
    }

    if (self.sideTextColor != nil) {
        int sideFontSize = floor(rect.size.width*self.sideFontFactor*0.5f) *2;
        [self drawSubtractedText:self.sideText inRect:sideRect inContext:context withFont:[UIFont systemFontOfSize:sideFontSize] withColor:self.sideTextColor];
    }

    CGColorSpaceRelease(colorspace);
    CGPathRelease(newBasePath);

    [super drawRect:rect];
}

- (void)drawSubtractedText:(NSString *)text
                    inRect:(CGRect)rect
                 inContext:(CGContextRef)context
                  withFont:(UIFont *)font
                 withColor:(UIColor *)color
{
    CGContextSaveGState(context);

    // Flip the coordinate system
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
    CGContextTranslateCTM(context, 0, rect.size.height + rect.origin.y);
    CGContextScaleCTM(context, 1.0, -1.0);

    CGContextSetShouldSmoothFonts(context, YES);
    CGContextSetShouldAntialias(context, YES);

    // set text horizontal alignment
    NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
    paragraphStyle.alignment = NSTextAlignmentCenter;

    CGFloat radius = floor(rect.size.width*0.5f);

    NSDictionary *attributes = @{NSParagraphStyleAttributeName:paragraphStyle, NSFontAttributeName:font, NSForegroundColorAttributeName:color};

    // An attributed string containing the text to render
    NSAttributedString* attString = [[NSAttributedString alloc]
                                     initWithString:text
                                     attributes:attributes];



    CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString((CFAttributedStringRef)attString); //3

    CGSize size = CTFramesetterSuggestFrameSizeWithConstraints(framesetter, CFRangeMake(0, [attString length]), NULL, CGSizeMake(radius*1.6, CGFLOAT_MAX), NULL);


    // Create a path to render text in
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathAddRect(path, NULL, CGRectMake(rect.origin.x + radius - size.width*0.5f, radius - size.height*0.5f, size.width, size.height));


    // Finally create the framesetter and render text
    CTFrameRef frame = CTFramesetterCreateFrame(framesetter,
                                                CFRangeMake(0, [attString length]), path, NULL);

    CTFrameDraw(frame, context);

    // Clean up
    CGPathRelease(path);
    CFRelease(frame);
    CFRelease(framesetter);


    // Restore the state of other drawing operations
    CGContextRestoreGState(context);
}

//- (void)updateConstraints {
//    [super updateConstraints];
////    [self updateCustomSizes];
//}

- (void)layoutSubviews {
    [super layoutSubviews];
    [self updateCustomSizes];
}

- (void) updateCustomSizes {
    [self layoutIfNeeded];

    self.circlePadding = MIN(self.frame.size.height, self.frame.size.width) * 0.05f;

    // main.radius = 2 * side.radius
    // width = margin.left + main.radius + (sin(45deg) * (main.radius + side.radius)) + side.radius + margin.right
    // height = top + main.radius + (cos(45deg) * (main.radius + side.radius)) + side.radius + bottom

    CGFloat sideRadiusX = (self.frame.size.width - self.layoutMargins.left - 2*self.circlePadding - self.layoutMargins.right - sin(M_PI_4) * self.circlePadding) / (3.0f + sin(M_PI_4)*3.0f);
    CGFloat sideRadiusY = (self.frame.size.height - self.layoutMargins.top - 2*self.circlePadding - self.layoutMargins.bottom - sin(M_PI_4) * self.circlePadding) / (3.0f + sin(M_PI_4)*3.0f);

    self.sideRadius = MIN(sideRadiusX, sideRadiusY);
    self.mainRadius = self.sideRadius*2;

    self.mainCenter = CGPointMake(self.layoutMargins.left + self.circlePadding + self.mainRadius,
                                  self.layoutMargins.top + self.circlePadding + self.mainRadius);
    self.sideCenter = CGPointMake(self.frame.size.width - self.layoutMargins.right - self.circlePadding - self.sideRadius,
                                  self.frame.size.height - self.layoutMargins.bottom - self.circlePadding - self.sideRadius);
}

- (void)handleTap: (UITapGestureRecognizer*) recognizer {
    CGPoint point = [recognizer locationInView:self];

    CGFloat touchDist = sqrt(pow(point.x - self.mainCenter.x, 2) + pow(point.y - self.mainCenter.y, 2));
    if(touchDist < self.mainRadius) {
        [self showTapOnLayer:self.mainCircleLayer];
        [self mainAction];

    }

    touchDist = sqrt(pow(point.x - self.sideCenter.x, 2) + pow(point.y - self.sideCenter.y, 2));
    if (touchDist < self.sideRadius) {
        [self showTapOnLayer:self.sideCircleLayer];
        [self sideAction];
    }
}

- (void)handlePress: (UILongPressGestureRecognizer*) recognizer {
    CGPoint point = [recognizer locationInView:self];

    CGFloat touchDist = sqrt(pow(point.x - self.mainCenter.x, 2) + pow(point.y - self.mainCenter.y, 2));
    if(touchDist < self.mainRadius) {
        if(recognizer.state == UIGestureRecognizerStateBegan) {
            self.longPressedLayer = self.mainCircleLayer;
            [self showPressOnLayer:self.mainCircleLayer];
        }

    }

    touchDist = sqrt(pow(point.x - self.sideCenter.x, 2) + pow(point.y - self.sideCenter.y, 2));
    if (touchDist < self.sideRadius) {
        if(recognizer.state == UIGestureRecognizerStateBegan) {
            self.longPressedLayer = self.sideCircleLayer;
            [self showPressOnLayer:self.sideCircleLayer];
        }
    }

    if(recognizer.state == UIGestureRecognizerStateCancelled ||
       recognizer.state == UIGestureRecognizerStateEnded ||
       recognizer.state == UIGestureRecognizerStateFailed) {
        if(self.longPressedLayer != nil) {
            [self showReleaseOnLayer:self.longPressedLayer];
            if(self.longPressedLayer == self.mainCircleLayer) {
                [self mainAction];
            } else if (self.longPressedLayer == self.sideCircleLayer) {
                [self sideAction];
            }
            self.longPressedLayer = nil;
        }
    }
}

- (void)showTapOnLayer:(CAShapeLayer *)layer {
    CAKeyframeAnimation *animation = [CAKeyframeAnimation animationWithKeyPath:@"opacity"];
    animation.beginTime = 0.0f;
    animation.duration = 0.2f;
    animation.values = @[
                         [NSNumber numberWithFloat:1.0f],
                         [NSNumber numberWithFloat:0.5f],
                         [NSNumber numberWithFloat:1.0f]
                         ];
    animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
    animation.removedOnCompletion = YES;
    animation.fillMode = kCAFillModeBoth;
    animation.additive = NO;
    [layer addAnimation:animation forKey:@"opacityINOUT"];
}

- (void)showPressOnLayer:(CAShapeLayer *)layer {
    CAKeyframeAnimation *animation = [CAKeyframeAnimation animationWithKeyPath:@"opacity"];
    animation.beginTime = 0.0f;
    animation.duration = 0.1f;
    animation.values = @[
                         [NSNumber numberWithFloat:1.0f],
                         [NSNumber numberWithFloat:0.5f]
                         ];
    animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
    animation.removedOnCompletion = NO;
    animation.fillMode = kCAFillModeBoth;
    animation.additive = NO;
    [layer addAnimation:animation forKey:@"opacityIN"];
}

- (void)showReleaseOnLayer:(CAShapeLayer *)layer {
    CAKeyframeAnimation *animation = [CAKeyframeAnimation animationWithKeyPath:@"opacity"];
    animation.beginTime = 0.0f;
    animation.duration = 0.1f;
    animation.values = @[
                         [NSNumber numberWithFloat:0.5f],
                         [NSNumber numberWithFloat:1.0f]
                         ];
    animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
    animation.removedOnCompletion = NO;
    animation.fillMode = kCAFillModeBoth;
    animation.additive = NO;
    [layer addAnimation:animation forKey:@"opacityOUT"];
}

- (void)mainAction {
    if(self.handler != nil && [self.handler respondsToSelector:@selector(handleMainPressedForMainControl:)]) {
        [self.handler handleMainPressedForMainControl:self];
    }
}

- (void)sideAction {
    if(self.handler != nil && [self.handler respondsToSelector:@selector(handleSidePressedForMainControl:)]) {
        [self.handler handleSidePressedForMainControl:self];
    }
}

- (void)setMainColor:(UIColor *)mainColor {
    _mainColor = mainColor;
    [self.mainCircleLayer setFillColor:[_mainColor CGColor]];
}

- (void)setSideColor:(UIColor *)sideColor {
    _sideColor = sideColor;
    [self.sideCircleLayer setFillColor:[_sideColor CGColor]];
}

- (void)setBackColor:(UIColor *)backColor {
    _backColor = backColor;
    [self.baseLayer setFillColor:[_backColor CGColor]];
}

- (void)setMainTextColor:(UIColor *)mainTextColor {
    _mainTextColor = mainTextColor;
    [self setNeedsDisplay];
}

- (void)setSideTextColor:(UIColor *)sideTextColor {
    _sideTextColor = sideTextColor;
    [self setNeedsDisplay];
}

- (void)setMainFontFactor:(float)mainFontFactor {
    _mainFontFactor = mainFontFactor;
    [self setNeedsDisplay];
}

- (void)setSideFontFactor:(float)sideFontFactor {
    _sideFontFactor = sideFontFactor;
    [self setNeedsDisplay];
}

- (void)setMainText:(NSString *)mainText {
    _mainText = mainText;
    [self setNeedsDisplay];
}

- (void)setSideText:(NSString *)sideText {
    _sideText = sideText;
    [self setNeedsDisplay];
}

- (CGFloat)getWidthToCenteredWidthRatio {
    return (self.bounds.size.width + (self.bounds.size.width - self.circlePadding*2 - self.mainRadius*2))/self.bounds.size.width;
}

@end
