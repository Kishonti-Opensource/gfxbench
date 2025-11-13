/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Charts/THGridChart.h"
#import "Common/THTheme.h"


# pragma mark - AxisLayer

#if __IPHONE_OS_VERSION_MAX_ALLOWED > __IPHONE_9_3
@interface THAxisLayer : CALayer<CALayerDelegate>
#else
@interface THAxisLayer : CALayer
#endif

@property (nonatomic, assign) float maxValue;
@property (nonatomic, assign) float minValue;
@property (nonatomic, copy) NSString *title;
@property (nonatomic, assign) CGSize textSize;

@property (nonatomic, strong) UIFont * textFont;
@property (nonatomic, strong) UIColor * textColor;

@property (nonatomic, assign) BOOL displayAxis;

@property (nonatomic, assign) CGAffineTransform gridTrafo;

@property (nonatomic, assign) float rangeMin;
@property (nonatomic, assign) float rangeMax;

@end
@implementation THAxisLayer {
    float firstPos;
    float lastPos;
    float valueMinDist;
    CGRect chartRect;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];

    if(self.frame.size.height >= self.frame.size.width)
        chartRect = CGRectInset(self.bounds, 0, self.textSize.height*0.5f);
    else
        chartRect = CGRectInset(self.bounds, self.textSize.width*0.5f, 0);

    [self setupTransform];
}

- (void)setTextSize:(CGSize)textSize {
    _textSize = textSize;

    if(self.frame.size.height >= self.frame.size.width)
        chartRect = CGRectInset(self.bounds, 0, self.textSize.height*0.5f);
    else
        chartRect = CGRectInset(self.bounds, self.textSize.width*0.5f, 0);

    [self setupTransform];
}

- (void)setMaxValue:(float)maxValue {
    _maxValue = maxValue;
    [self setupTransform];
}

- (void)setMinValue:(float)minValue {
    _minValue = minValue;
    [self setupTransform];
}

- (void)setupTransform {
    float minDist = 50.0f;

    float rangeDiff = self.maxValue - self.minValue;
    float division = pow(10, ceil(log10(rangeDiff))) * (MAX(chartRect.size.width, chartRect.size.height) / rangeDiff);
    int rangeMSD = (int)log10(division/minDist);
    float div10 = division / pow(10, rangeMSD);

    int range2MSD = (int)log2(div10/minDist);
    float div2 = div10 / pow(2, range2MSD);

    valueMinDist = div2 / MAX(chartRect.size.width, chartRect.size.height) * rangeDiff;

    firstPos = (floorf(self.minValue / valueMinDist) + 1) * valueMinDist;
    lastPos = (ceil(self.maxValue / valueMinDist) - 1) * valueMinDist;
}

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)context {
    if(context == nil) return;

    if([layer isKindOfClass:[THAxisLayer class]] && self.displayAxis) {
        [self drawAxisLayer:(THAxisLayer *)layer inContext:context];
    }
}

- (void)drawAxisLayer:(THAxisLayer*)layer inContext:(CGContextRef)context {
    UIGraphicsPushContext(context);
    CGContextSaveGState(context);

    if(self.textFont == nil) {
        self.textColor = [THTheme getColorNamed:@"LineChart_TextColor"];
        self.textFont = [THTheme getFontNamed:@"TextSFont"];
    }

    BOOL isVertical = chartRect.size.height > chartRect.size.width;

    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    textStyle.alignment = isVertical ? NSTextAlignmentRight : NSTextAlignmentCenter;
    NSDictionary* textFontAttributes = @{NSFontAttributeName: self.textFont,
                                         NSForegroundColorAttributeName: self.textColor,
                                         NSParagraphStyleAttributeName: textStyle};

    CGPoint textPoint;
    if(firstPos != self.minValue) {
        textPoint = CGPointMake(isVertical ? chartRect.size.width - self.textSize.width*0.5f : chartRect.origin.x,
                                isVertical ? chartRect.origin.y + chartRect.size.height : self.textSize.height*0.5f);

        NSString *label = [NSString stringWithFormat:@"%.2f", self.minValue];
        [label drawInRect:CGRectMake(textPoint.x - self.textSize.width*0.5f,
                                     textPoint.y - self.textSize.height*0.5f,
                                     self.textSize.width,
                                     self.textSize.height)
           withAttributes:textFontAttributes];
    }

    if(lastPos != self.maxValue) {
        textPoint = CGPointMake(isVertical ? chartRect.size.width - self.textSize.width*0.5f : chartRect.origin.x + chartRect.size.width,
                                isVertical ? chartRect.origin.y : self.textSize.height*0.5f);

        NSString *label = [NSString stringWithFormat:@"%.2f", self.maxValue];
        [label drawInRect:CGRectMake(textPoint.x - self.textSize.width*0.5f,
                                     textPoint.y - self.textSize.height*0.5f,
                                     self.textSize.width,
                                     self.textSize.height)
           withAttributes:textFontAttributes];
    }


    for(int i = 0; firstPos + i * valueMinDist <= lastPos; ++i) {

        CGPoint p = CGPointMake(0, firstPos + i * valueMinDist);
        if(!isVertical)
            p = CGPointMake(firstPos + i * valueMinDist, 0);

        p = CGPointApplyAffineTransform(p, self.gridTrafo);

        textPoint = CGPointMake(isVertical ?
                                chartRect.size.width - self.textSize.width*0.5f :
                                chartRect.origin.x + p.x,
                                isVertical ?
                                chartRect.origin.y + chartRect.size.height - p.y :
                                self.textSize.height*0.5f);

        if((chartRect.origin.x + chartRect.size.width - textPoint.x > self.textSize.width || isVertical) &&
           (textPoint.x - chartRect.origin.x > self.textSize.width || isVertical) &&
           (chartRect.origin.y + chartRect.size.height - textPoint.y > self.textSize.height || !isVertical) &&
           (textPoint.y - chartRect.origin.y > self.textSize.height || !isVertical)) {

            NSString *label = [NSString stringWithFormat:@"%.2f", firstPos + i * valueMinDist];
            [label drawInRect:CGRectMake(textPoint.x - self.textSize.width*0.5f,
                                         textPoint.y - self.textSize.height*0.5f,
                                         self.textSize.width,
                                         self.textSize.height)
               withAttributes:textFontAttributes];
        }
    }

    CGContextSaveGState(context);
    NSString *axisName = self.title;
    CGSize axisSize = [axisName sizeWithAttributes:textFontAttributes];
    CGPoint p = CGPointMake(isVertical ? axisSize.height : chartRect.size.width * 0.5f,
                            isVertical ? chartRect.size.height * 0.5f : chartRect.size.height - self.textSize.height);

    CGRect r = CGRectMake(p.x - axisSize.width * 0.5f, p.y - axisSize.height * 0.5f, axisSize.width, axisSize.width);

    if(isVertical) {
        // Rotate the context 90 degrees (convert to radians)
        CGAffineTransform transform = CGAffineTransformIdentity;
        transform = CGAffineTransformTranslate(transform, p.x, p.y);
        transform = CGAffineTransformRotate(transform, -90.0 * M_PI/180.0);
        transform = CGAffineTransformTranslate(transform, -p.x, -p.y);
        CGContextConcatCTM(context, transform);
    }

    [axisName drawInRect:r withAttributes:textFontAttributes];
    CGContextRestoreGState(context);




    CGContextRestoreGState(context);
    UIGraphicsPopContext();
}


@end


# pragma mark - GridLayer

#if __IPHONE_OS_VERSION_MAX_ALLOWED > __IPHONE_9_3
@interface THGridLayer : CALayer<CALayerDelegate>
#else
@interface THGridLayer : CALayer
#endif

@property (nonatomic, strong) UIColor *lineColor;
@property (nonatomic, strong) UIColor *chartBorderColor;

@property (nonatomic, assign) float xMaxValue;
@property (nonatomic, assign) float xMinValue;
@property (nonatomic, assign) float yMaxValue;
@property (nonatomic, assign) float yMinValue;
@property (nonatomic, assign) float lineWidth;
@property (nonatomic, assign) float chartBorderWidth;

@property (nonatomic, assign) BOOL displayGrid;

@property (nonatomic, assign) CGAffineTransform gridTrafo;

@property (nonatomic, assign) CGRect valueRange;

@end
@implementation THGridLayer {
    float xFirstPos;
    float xLastPos;
    float xValueMinDist;
    float yFirstPos;
    float yLastPos;
    float yValueMinDist;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];

    [self setupXTransform];
    [self setupYTransform];
}

- (void)setXMaxValue:(float)maxValue {
    _xMaxValue = maxValue;
    [self setupXTransform];
}
- (void)setXMinValue:(float)minValue {
    _xMinValue = minValue;
    [self setupXTransform];
}
- (void)setYMaxValue:(float)maxValue {
    _yMaxValue = maxValue;
    [self setupYTransform];
}
- (void)setYMinValue:(float)minValue {
    _yMinValue = minValue;
    [self setupYTransform];
}

- (void)setGridTrafo:(CGAffineTransform)gridTrafo {
    _gridTrafo = gridTrafo;
    [self setupXTransform];
    [self setupYTransform];
}

- (void)setupXTransform {
    float minDist = 50.0f;

    float rangeDiff = self.xMaxValue - self.xMinValue;
    float division = pow(10, ceil(log10(rangeDiff))) * self.gridTrafo.a;
    int rangeMSD = (int)log10(division/minDist);
    float div10 = division / pow(10, rangeMSD);

    int range2MSD = (int)log2(div10/minDist);
    float div2 = div10 / pow(2, range2MSD);

    xValueMinDist = div2 / self.gridTrafo.a;
    xFirstPos = (floorf(self.xMinValue / xValueMinDist) + 1) * xValueMinDist;
    xLastPos = (ceil(self.xMaxValue / xValueMinDist) - 1) * xValueMinDist;
}

- (void)setupYTransform {
    float minDist = 50.0f;

    float rangeDiff = self.yMaxValue - self.yMinValue;
    float division = pow(10, ceil(log10(rangeDiff))) * self.gridTrafo.d;
    int rangeMSD = (int)log10(division/minDist);
    float div10 = division / pow(10, rangeMSD);

    int range2MSD = (int)log2(div10/minDist);
    float div2 = div10 / pow(2, range2MSD);

    yValueMinDist = div2 / self.gridTrafo.d;
    yFirstPos = (floorf(self.yMinValue / yValueMinDist) + 1) * yValueMinDist;
    yLastPos = (ceil(self.yMaxValue / yValueMinDist) - 1) * yValueMinDist;
}

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)context {
    if(context == nil) return;

    if([layer isKindOfClass:[THGridLayer class]]) {
        [self drawGridLayer:(THGridLayer *)layer inContext:context];
    }
}

- (void)drawGridLayer:(THGridLayer*)layer inContext:(CGContextRef)context {
    UIGraphicsPushContext(context);
    CGContextSaveGState(context);

    CGContextSetStrokeColorWithColor(context, [self.lineColor CGColor]);
    float lineWidth = self.lineWidth;
    CGContextSetLineWidth(context, lineWidth);

    if(self.displayGrid) {
        for(int i = 0; xFirstPos + i * xValueMinDist <= xLastPos; ++i) {
            CGPoint p = CGPointApplyAffineTransform(CGPointMake(xFirstPos + i * xValueMinDist, 0), self.gridTrafo);
            if(xLastPos - xFirstPos < 0) p.x = 0;
            CGContextMoveToPoint(context, p.x, 0);
            CGContextAddLineToPoint(context, p.x, self.bounds.size.height);
            CGContextStrokePath(context);
        }
        for(int i = 0; yFirstPos + i * yValueMinDist <= yLastPos; ++i) {
            CGPoint p = CGPointApplyAffineTransform(CGPointMake(0, yFirstPos + i * yValueMinDist), self.gridTrafo);
            if(yLastPos - yFirstPos < 0) p.y = 0;
            CGContextMoveToPoint(context, 0, self.bounds.size.height - p.y);
            CGContextAddLineToPoint(context, self.bounds.size.width, self.bounds.size.height - p.y);
            CGContextStrokePath(context);
        }
    }


    CGContextSetStrokeColorWithColor(context, [self.chartBorderColor CGColor]);
    lineWidth = self.chartBorderWidth;
    CGContextSetLineWidth(context, lineWidth);

    CGContextMoveToPoint(context, lineWidth * 0.5f, lineWidth * 0.5f);
    CGContextAddLineToPoint(context, lineWidth * 0.5f, self.bounds.size.height - lineWidth * 0.5f);
    CGContextAddLineToPoint(context, self.bounds.size.width - lineWidth * 0.5f, self.bounds.size.height - lineWidth * 0.5f);
    CGContextAddLineToPoint(context, self.bounds.size.width - lineWidth * 0.5f, lineWidth * 0.5f);
    CGContextAddLineToPoint(context, lineWidth * 0.5f, lineWidth * 0.5f);
    CGContextStrokePath(context);

    CGContextRestoreGState(context);
    UIGraphicsPopContext();
}


@end


# pragma mark - LegendLayer
#if __IPHONE_OS_VERSION_MAX_ALLOWED > __IPHONE_9_3
@interface THLegendLayer : CALayer<CALayerDelegate>
#else
@interface THLegendLayer : CALayer
#endif


@property (nonatomic, assign) BOOL displayLegend;
@property (nonatomic, assign) float padding;
@property (nonatomic, assign) float colorCodeWidth;

@property (nonatomic, strong) UIFont *font;
@property (nonatomic, strong) UIColor *fontColor;
@property (nonatomic, strong) UIColor *colorCodeBorderColor;
@property (nonatomic, strong) NSArray *labels;

- (CGSize)getPreferredSize;

@end
@implementation THLegendLayer

- (CGSize)getPreferredSize {
    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    textStyle.alignment = NSTextAlignmentRight;
    NSDictionary* textFontAttributes = @{NSFontAttributeName: self.font,
                                         NSForegroundColorAttributeName: self.fontColor,
                                         NSParagraphStyleAttributeName: textStyle};

    float maxWidth = 0.0f;
    float height = self.padding;
    for (NSDictionary *label in self.labels) {
        NSString *name = label[@"name"];
        CGSize nameSize = [name sizeWithAttributes:textFontAttributes];

        maxWidth = MAX(nameSize.width + 2*self.padding, maxWidth);
        height += nameSize.height + self.padding;
    }

    return CGSizeMake(maxWidth + self.padding + self.colorCodeWidth, height);
}

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)context {
    if(context == nil) return;

    if([layer isKindOfClass:[THLegendLayer class]]) {
        [self drawLegendLayer:(THLegendLayer *)layer inContext:context];
    }
}

- (void)drawLegendLayer:(THLegendLayer*)layer inContext:(CGContextRef)context {
    if(self.displayLegend) {
        UIGraphicsPushContext(context);
        CGContextSaveGState(context);

        NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
        textStyle.alignment = NSTextAlignmentLeft;

        float sumHeight = self.padding;
        for (NSDictionary *label in self.labels) {
            NSString *name = label[@"name"];
            UIColor *color = label[@"color"];
            NSDictionary* textFontAttributes = @{NSFontAttributeName: self.font,
                                                 NSForegroundColorAttributeName: color,
                                                 NSParagraphStyleAttributeName: textStyle};
            CGSize nameSize = [name sizeWithAttributes:textFontAttributes];

            [name drawInRect:CGRectMake(2*self.padding + self.colorCodeWidth,
                                        sumHeight,
                                        nameSize.width,
                                        nameSize.height)
              withAttributes:textFontAttributes];

            CGContextSetFillColorWithColor(context, [self.colorCodeBorderColor CGColor]);
            CGContextFillRect(context, CGRectMake(self.padding, sumHeight, self.colorCodeWidth, nameSize.height));
            CGContextSetFillColorWithColor(context, self.backgroundColor);
            CGContextFillRect(context, CGRectMake(self.padding + 1, sumHeight + 1, self.colorCodeWidth - 2, nameSize.height- 2));
            CGContextSetFillColorWithColor(context, [color CGColor]);
            CGContextFillRect(context, CGRectMake(self.padding + 2, sumHeight + 2, self.colorCodeWidth - 4, nameSize.height - 4));


            sumHeight += self.padding + nameSize.height;
        }

        CGContextRestoreGState(context);
        UIGraphicsPopContext();
    }
}


@end

@interface THGridChart()

@property (nonatomic, strong) THAxisLayer *xAxisLayer;
@property (nonatomic, strong) THAxisLayer *yAxisLayer;
@property (nonatomic, strong) THGridLayer *gridLayer;
@property (nonatomic, strong) THLegendLayer *legendLayer;

@property (nonatomic, strong) UIPinchGestureRecognizer *pinchRecognizer;
@property (nonatomic, strong) UIPanGestureRecognizer *panRecognizer;

@property (nonatomic, assign) float currentScaleX;
@property (nonatomic, assign) float currentScaleY;

@property (nonatomic, assign) CGRect currentView;
@property (nonatomic, assign) CGRect valueRange;
@property (nonatomic, assign) CGPoint currentPos;
@property (nonatomic, assign) CGPoint origPos;

@end

@implementation THGridChart

- (void)setup {
    [super setup];

    self.currentPos = CGPointMake(0, 0);
    self.origPos = CGPointMake(0, 0);
    self.currentScaleX = 1;
    self.currentScaleY = 1;
    self.currentView = [self getChartRect];
    self.pinchRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];
    [self addGestureRecognizer:self.pinchRecognizer];
    self.panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePan:)];
    [self addGestureRecognizer:self.panRecognizer];

    [self setBackgroundColor:[THTheme getColorNamed:@"Diagram_Background"]];
    self.layer.backgroundColor = [THTheme getColorNamed:@"Diagram_Background"].CGColor;

    self.lineWidth = [THTheme getFloatNamed:@"LineChart_LineWidth"];
    self.chartBorderWidth = [THTheme getFloatNamed:@"LineChart_BorderWidth"];
    self.legendColorWidth = [THTheme getFloatNamed:@"LineChart_LegendColorWidth"];

    self.lineColor = [THTheme getColorNamed:@"LineChart_LineColor"];
    self.chartBorderColor = [THTheme getColorNamed:@"LineChart_BorderColor"];

    self.animationTime = [THTheme getFloatNamed:@"LineChart_AnimationTime"];
    self.animationDelay = [THTheme getFloatNamed:@"LineChart_AnimationDelay"];
    self.startAnimationDelay = [THTheme getFloatNamed:@"LineChart_StartAnimationDelay"];


    self.padding = [THTheme getFloatNamed:@"DefaultPadding"];
    self.textColor = [THTheme getColorNamed:@"LineChart_TextColor"];
    self.textFont = [THTheme getFontNamed:@"TextSFont"];

    self.displayAxis = true;
    self.displayGrid = true;
    self.displayLegend = true;
}

- (void)fillData {
    [super fillData];
    [self layoutIfNeeded];
    self.currentView = [self getChartRect];

    self.valueRange = CGRectMake(FLT_MIN, FLT_MIN, FLT_MAX, FLT_MAX);

    NSArray *domainValues = self.data[@"domain"][@"values"];
    self.dMin = [[domainValues valueForKeyPath:@"@min.floatValue"] floatValue];
    self.dMax = [[domainValues valueForKeyPath:@"@max.floatValue"] floatValue];
    self.rMin = FLT_MAX;
    self.rMax = FLT_MIN;
    for (NSDictionary *XSapmles in self.data[@"values"]) {
        NSArray *values = XSapmles[@"values"];
        self.rMax = MAX(self.rMax, [[values valueForKeyPath:@"@max.floatValue"] floatValue]);
        self.rMin = MIN(self.rMin, [[values valueForKeyPath:@"@min.floatValue"] floatValue]);
    }

    self.xAxisLayer = [THAxisLayer layer];
#ifndef __IPHONE_10_0
    [self.xAxisLayer setDelegate:self.xAxisLayer];
#endif
    [self.xAxisLayer setMinValue:self.dMin];
    [self.xAxisLayer setMaxValue:self.dMax];
    [self.xAxisLayer setTitle:self.data[@"domain"][@"name"]];
    [self.xAxisLayer setDisplayAxis:self.displayAxis];
    [self.xAxisLayer setTextColor:self.textColor];
    [self.xAxisLayer setTextFont:self.textFont];
    [self.xAxisLayer setGridTrafo:self.gridTransform];
    [self.xAxisLayer setRangeMin:self.valueRange.origin.x];
    [self.xAxisLayer setRangeMax:self.valueRange.size.width];
    [self.xAxisLayer setMasksToBounds:NO];
    [self.xAxisLayer setContentsScale:[[UIScreen mainScreen] scale]];
    [self.layer addSublayer:self.xAxisLayer];

    self.yAxisLayer = [THAxisLayer layer];
    [self.yAxisLayer setDelegate:self.yAxisLayer];
    [self.yAxisLayer setMinValue:self.rMin];
    [self.yAxisLayer setMaxValue:self.rMax];
    [self.yAxisLayer setTitle:self.data[@"sample_axis"]];
    [self.yAxisLayer setDisplayAxis:self.displayAxis];
    [self.yAxisLayer setTextColor:self.textColor];
    [self.yAxisLayer setTextFont:self.textFont];
    [self.yAxisLayer setGridTrafo:self.gridTransform];
    [self.yAxisLayer setRangeMin:self.valueRange.origin.y];
    [self.yAxisLayer setRangeMax:self.valueRange.size.height];
    [self.yAxisLayer setMasksToBounds:NO];
    [self.yAxisLayer setContentsScale:[[UIScreen mainScreen] scale]];
    [self.layer addSublayer:self.yAxisLayer];

    self.gridLayer = [THGridLayer layer];
    [self.gridLayer setDelegate:self.gridLayer];
    [self.gridLayer setYMinValue:self.rMin];
    [self.gridLayer setYMaxValue:self.rMax];
    [self.gridLayer setXMinValue:self.dMin];
    [self.gridLayer setXMaxValue:self.dMax];
    [self.gridLayer setGridTrafo:self.gridTransform];
    [self.gridLayer setLineWidth:self.lineWidth];
    [self.gridLayer setLineColor:self.lineColor];
    [self.gridLayer setChartBorderColor:self.chartBorderColor];
    [self.gridLayer setChartBorderWidth:self.chartBorderWidth];
    [self.gridLayer setDisplayGrid:self.displayGrid];
    [self.gridLayer setValueRange:self.valueRange];
    [self.gridLayer setContentsScale:[[UIScreen mainScreen] scale]];
    [self.layer addSublayer:self.gridLayer];

    NSMutableArray *legendLabels = [[NSMutableArray alloc] init];
    NSInteger color_count = [THTheme getIntegerNamed:@"Diagram_Value_Count"];
    for (NSInteger i = 0; i < self.sortedValues.count; ++i) {
        NSString *colorString = [NSString stringWithFormat:@"Diagram_Values_%ld", (long)(i%color_count)];
        [legendLabels addObject:@{
                                  @"name" : self.sortedValues[i][@"name"],
                                  @"color" : [THTheme getColorNamed:colorString]
                                  }];
    }

    self.legendLayer = [THLegendLayer layer];
    [self.legendLayer setDelegate:self.legendLayer];
    [self.legendLayer setDisplayLegend:self.displayLegend];
    [self.legendLayer setPadding:[THTheme getFloatNamed:@"Diagram_Padding"]];
    [self.legendLayer setColorCodeWidth:self.legendColorWidth];
    [self.legendLayer setFont:self.textFont];
    [self.legendLayer setFontColor:self.textColor];
    [self.legendLayer setColorCodeBorderColor:self.lineColor];
    [self.legendLayer setBackgroundColor:self.backgroundColor.CGColor];
    [self.legendLayer setLabels:legendLabels];
    [self.legendLayer setContentsScale:[[UIScreen mainScreen] scale]];
    [self.layer addSublayer:self.legendLayer];

    [self setupGridTransform];
}

- (CGRect)getChartRect {
    float padding = self.padding;
    UIFont *textFont = self.textFont != nil ? self.textFont : [THTheme getFontNamed:@"TextSFont"];
    UIColor *textColor = self.textColor != nil ? self.textColor : [THTheme getColorNamed:@"LineChart_TextColor"];


    NSString *testString = @"10000";

    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    NSDictionary* textFontAttributes = @{NSFontAttributeName: textFont,
                                         NSForegroundColorAttributeName: textColor,
                                         NSParagraphStyleAttributeName: textStyle};
    CGSize testStringSize = [testString sizeWithAttributes:textFontAttributes];

    float verticalAxisWidth = ceil(2*padding + testStringSize.width + testStringSize.height);
    float horizontalAxisHeight = ceil(2*padding + 2 * testStringSize.height);

    if(!self.displayAxis) {
        verticalAxisWidth = padding;
        horizontalAxisHeight = padding;
    }


    CGRect insetted = CGRectMake(padding + verticalAxisWidth + testStringSize.width * 0.5f,
                                 padding + testStringSize.height*0.5f,
                                 self.layer.frame.size.width - verticalAxisWidth - 2*padding - testStringSize.width,
                                 self.layer.frame.size.height - horizontalAxisHeight - 2*padding - testStringSize.height);

    return insetted;
}

- (void) displayLayers {

    float padding = self.padding;
    UIFont *textFont = self.textFont != nil ? self.textFont : [THTheme getFontNamed:@"TextSFont"];
    UIColor *textColor = self.textColor != nil ? self.textColor : [THTheme getColorNamed:@"LineChart_TextColor"];


    NSString *testString = @"10000";

    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    NSDictionary* textFontAttributes = @{NSFontAttributeName: textFont,
                                         NSForegroundColorAttributeName: textColor,
                                         NSParagraphStyleAttributeName: textStyle};
    CGSize testStringSize = [testString sizeWithAttributes:textFontAttributes];

    float verticalAxisWidth = ceil(2*padding + testStringSize.width + testStringSize.height);
    float horizontalAxisHeight = ceil(2*padding + 2 * testStringSize.height);

    if(!self.displayAxis) {
        verticalAxisWidth = padding;
        horizontalAxisHeight = padding;
    }


    CGRect insetted = CGRectMake(padding + verticalAxisWidth + testStringSize.width * 0.5f,
                                 padding + testStringSize.height*0.5f,
                                 self.layer.frame.size.width - verticalAxisWidth - 2*padding - testStringSize.width,
                                 self.layer.frame.size.height - horizontalAxisHeight - 2*padding - testStringSize.height);

    if(self.yAxisLayer != nil) {
        self.yAxisLayer.textSize = testStringSize;
        self.yAxisLayer.frame = CGRectMake(padding,
                                           padding,
                                           verticalAxisWidth,
                                           insetted.size.height + testStringSize.height);
        [self.yAxisLayer setNeedsDisplay];
    }

    if(self.xAxisLayer != nil) {
        self.xAxisLayer.textSize = testStringSize;
        self.xAxisLayer.frame = CGRectMake(padding + verticalAxisWidth,
                                           insetted.origin.y + insetted.size.height + testStringSize.height * 0.5f,
                                           insetted.size.width + testStringSize.width,
                                           horizontalAxisHeight);
        [self.xAxisLayer setNeedsDisplay];
    }

    if(self.gridLayer != nil) {
        self.gridLayer.frame = insetted;
        [self.gridLayer setNeedsDisplay];
    }

    if(self.legendLayer != nil) {
        CGSize preferred = [self.legendLayer getPreferredSize];
        [self.legendLayer setFrame:CGRectMake(insetted.origin.x + insetted.size.width - preferred.width - padding,
                                              insetted.origin.y + padding,
                                              preferred.width,
                                              preferred.height)];

        [self.legendLayer setNeedsDisplay];
    }


    [self setupGridTransform];
    [self.gridLayer setGridTrafo:self.gridTransform];
    [self.xAxisLayer setGridTrafo:self.gridTransform];
    [self.yAxisLayer setGridTrafo:self.gridTransform];

    [super displayLayers];
}

- (void)animateLayers {
    [super animateLayers];

    if(self.animationTime > 0) {
        [self.legendLayer setOpacity:0];

        CABasicAnimation* revealAnimation = [CABasicAnimation animationWithKeyPath:@"opacity"];
        revealAnimation.fromValue = [NSNumber numberWithFloat:0];
        revealAnimation.toValue = [NSNumber numberWithFloat:1];
        [revealAnimation setDuration:self.animationTime];
        [revealAnimation setRemovedOnCompletion:NO];
        [revealAnimation setFillMode:kCAFillModeForwards];
        [revealAnimation setTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut]];

        [self.legendLayer addAnimation:revealAnimation forKey:@"revealAnimation"];
    }
}

- (void)clearLayers {
    [super clearLayers];

    if(self.xAxisLayer != nil) {
        [self.xAxisLayer removeFromSuperlayer];
        self.xAxisLayer = nil;
    }
    if(self.yAxisLayer != nil) {
        [self.yAxisLayer removeFromSuperlayer];
        self.yAxisLayer = nil;
    }
    if(self.gridLayer != nil) {
        [self.gridLayer removeFromSuperlayer];
        self.gridLayer = nil;
    }
    if(self.legendLayer != nil) {
        [self.legendLayer removeFromSuperlayer];
        self.legendLayer = nil;
    }
}


# pragma mark Setters

- (void) setDisplayGrid:(BOOL)displayGrid {
    _displayGrid = displayGrid;
    if(self.gridLayer != nil)
        [self.gridLayer setDisplayGrid:displayGrid];
}

- (void) setDisplayAxis:(BOOL)displayAxis {
    _displayAxis = displayAxis;
    if(self.xAxisLayer != nil)
        [self.xAxisLayer setDisplayAxis:displayAxis];
    if(self.yAxisLayer != nil)
        [self.yAxisLayer setDisplayAxis:displayAxis];

    [self displayLayers];
}

- (void) setDisplayLegend:(BOOL)displayLegend {
    _displayLegend = displayLegend;

    if(self.legendLayer != nil)
        [self.legendLayer setDisplayLegend:displayLegend];
}

- (void) setLineWidth:(float)lineWidth {
    _lineWidth = lineWidth;

    if(self.gridLayer != nil)
        [self.gridLayer setLineWidth:lineWidth];
}

- (void) setChartBorderWidth:(float)borderWidth {
    _chartBorderWidth = borderWidth;

    if(self.gridLayer != nil)
        [self.gridLayer setChartBorderWidth:borderWidth];
}

- (void) setLineColor:(UIColor *)lineColor {
    _lineColor = lineColor;

    if(self.gridLayer != nil)
        [self.gridLayer setLineColor:lineColor];

    if(self.legendLayer != nil)
        [self.legendLayer setColorCodeBorderColor:lineColor];
}

- (void) setChartBorderColor:(UIColor *)borderColor {
    _chartBorderColor = borderColor;

    if(self.gridLayer != nil)
        [self.gridLayer setChartBorderColor:borderColor];
}

- (void) setTextColor:(UIColor *)textColor {
    _textColor = textColor;

    if(self.xAxisLayer != nil)
        [self.xAxisLayer setTextColor:textColor];
    if(self.yAxisLayer != nil)
        [self.yAxisLayer setTextColor:textColor];
    if(self.legendLayer != nil)
        [self.legendLayer setFontColor:textColor];
}

- (void) setTextFont:(UIFont *)textFont {
    _textFont = textFont;

    if(self.xAxisLayer != nil)
        [self.xAxisLayer setTextFont:textFont];
    if(self.yAxisLayer != nil)
        [self.yAxisLayer setTextFont:textFont];
    if(self.legendLayer != nil)
        [self.legendLayer setFont:textFont];

    [self displayLayers];
}

- (void) setPadding:(float)padding {
    _padding = padding;

    if(self.legendLayer != nil)
        [self.legendLayer setPadding:padding];

    [self displayLayers];
}

- (void) setLegendColorWidth:(float)legendColorWidth {
    _legendColorWidth = legendColorWidth;

    if(self.legendLayer != nil)
        [self.legendLayer setColorCodeWidth:legendColorWidth];

    [self displayLayers];
}

- (void) setBackgroundColor:(UIColor *)backgroundColor {
    [super setBackgroundColor:backgroundColor];

    if(self.legendLayer != nil)
        [self.legendLayer setBackgroundColor:backgroundColor.CGColor];
}

- (CGAffineTransform)getGridTransform {
    return self.gridTransform;
}

- (void)setupGridTransform {
    float dDivider = (ABS(self.dMax - self.dMin) < 0.00001) ? 1 : self.dMax - self.dMin;
    float rDivider = (ABS(self.rMax - self.rMin) < 0.00001) ? 1 : self.rMax - self.rMin;

    float ScaleD = 1/dDivider * self.gridLayer.bounds.size.width;
    float TransD = (- self.dMin)/dDivider * self.gridLayer.bounds.size.width;


    float ScaleR = 1/rDivider * self.gridLayer.bounds.size.height * 0.95;
    float TransR = (- self.rMin)/rDivider * self.gridLayer.bounds.size.height * 0.95;

    self.baseTransform = CGAffineTransformMake(ScaleD, 0, 0, ScaleR, TransD, TransR);
    self.viewTransform = CGAffineTransformMake(1, 0, 0, 1, 0, 0);
    self.gridTransform = CGAffineTransformConcat(self.baseTransform, self.viewTransform);
}

-(void)handlePinch:(UIPinchGestureRecognizer *)recognizer {

    CGPoint loc = [recognizer locationInView:self];
    float focusX = loc.x / self.bounds.size.width;
    float focusY = (1 - loc.y / self.bounds.size.height);

    CGPoint pos = CGPointMake((self.currentView.origin.x - self.bounds.size.width * focusX) / self.currentScaleX,
                              (self.currentView.origin.y - self.bounds.size.height * focusY) / self.currentScaleY);

    float scaleX = recognizer.scale;
    float scaleY = recognizer.scale;
    float y = [recognizer locationInView:self].y - [recognizer locationOfTouch:0 inView:self].y;
    float x = [recognizer locationInView:self].x - [recognizer locationOfTouch:0 inView:self].x;

    float absRad = ABS(atan(y/x));
    if( absRad <= M_PI / 6) {
        scaleY = 1;
    }
    if( absRad >= M_PI / 6 * 2) {
        scaleX = 1;
    }

    self.currentScaleX *= scaleX;
    self.currentScaleY *= scaleY;

    self.currentScaleX = MAX(1, MIN(100, self.currentScaleX));
    self.currentScaleY = MAX(1, MIN(100, self.currentScaleY));

    pos = CGPointMake(pos.x * self.currentScaleX,
                      pos.y * self.currentScaleY);

    CGPoint finalPos = CGPointMake(pos.x + self.bounds.size.width * focusX,
                                   pos.y + self.bounds.size.height * focusY);

    self.currentView = CGRectMake(finalPos.x,
                                  finalPos.y,
                                  self.bounds.size.width * (1/self.currentScaleX),
                                  self.bounds.size.height * (1/self.currentScaleY));

    if(self.currentView.origin.x > 0) {
        self.currentView = CGRectMake(0, self.currentView.origin.y, self.currentView.size.width, self.currentView.size.height);
    }
    if(self.currentView.origin.x < -self.bounds.size.width * self.currentScaleX + self.bounds.size.width) {
        self.currentView = CGRectMake(-self.bounds.size.width * self.currentScaleX + self.bounds.size.width, self.currentView.origin.y, self.currentView.size.width, self.currentView.size.height);
    }
    if(self.currentView.origin.y > 0) {
        self.currentView = CGRectMake(self.currentView.origin.x, 0, self.currentView.size.width, self.currentView.size.height);
    }
    if(self.currentView.origin.y < -self.bounds.size.height * self.currentScaleY + self.bounds.size.height) {
        self.currentView = CGRectMake(self.currentView.origin.x, -self.bounds.size.height * self.currentScaleY + self.bounds.size.height, self.currentView.size.width, self.currentView.size.height);
    }


    self.viewTransform = CGAffineTransformMake(self.currentScaleX,
                                               0,
                                               0,
                                               self.currentScaleY,
                                               self.currentView.origin.x,
                                               self.currentView.origin.y);

    [self modifyViewMatrix];

    recognizer.scale = 1;
}

-(BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer {
    if(gestureRecognizer == self.panRecognizer) {
        if([self.panRecognizer translationInView:self].y < 0 && self.currentView.origin.y >= 0)
            return false;

        if([self.panRecognizer translationInView:self].y > 0 && self.currentView.origin.y <= -self.gridLayer.bounds.size.height * (self.currentScaleY - 1))
            return false;
    }

    return true;
}

-(void)handlePan:(UIPanGestureRecognizer *)recognizer {



    CGPoint translation = CGPointMake(0, 0);
    if(recognizer.state == UIGestureRecognizerStateBegan) {
        self.currentPos = [recognizer locationInView:self];
        self.origPos = CGPointMake(self.currentView.origin.x, self.currentView.origin.y);
    } else {
        translation = CGPointMake([recognizer locationInView:self].x - self.currentPos.x,
                                  [recognizer locationInView:self].y - self.currentPos.y);
    }


    float max_min_x = MAX(MIN(0, self.origPos.x + translation.x), -self.gridLayer.bounds.size.width * (self.currentScaleX - 1));
    float max_min_y = MAX(MIN(0, self.origPos.y - translation.y), -self.gridLayer.bounds.size.height * (self.currentScaleY - 1));

    self.currentView = CGRectMake(max_min_x,
                                  max_min_y,
                                  self.currentView.size.width,
                                  self.currentView.size.height);

    self.viewTransform = CGAffineTransformMake(self.currentScaleX,
                                               0,
                                               0,
                                               self.currentScaleY,
                                               self.currentView.origin.x,
                                               self.currentView.origin.y);

    [self modifyViewMatrix];
}

- (void)modifyViewMatrix {

    self.gridTransform = CGAffineTransformConcat(self.baseTransform, self.viewTransform);
    CGAffineTransform labelTrafo = self.viewTransform;
    labelTrafo = CGAffineTransformConcat(self.baseTransform, labelTrafo);

    [self.gridLayer setGridTrafo:self.gridTransform];
    [self.xAxisLayer setGridTrafo:self.gridTransform];
    [self.yAxisLayer setGridTrafo:self.gridTransform];

    CGPoint minP = CGPointMake(0, 0);
    CGPoint maxP = CGPointMake(self.gridLayer.bounds.size.width, self.gridLayer.bounds.size.height);

    labelTrafo = CGAffineTransformInvert(labelTrafo);

    minP = CGPointApplyAffineTransform(minP, labelTrafo);
    maxP = CGPointApplyAffineTransform(maxP, labelTrafo);




    [self.gridLayer setYMinValue:minP.y];
    [self.gridLayer setYMaxValue:maxP.y];
    [self.gridLayer setXMinValue:minP.x];
    [self.gridLayer setXMaxValue:maxP.x];

    [self.xAxisLayer setMinValue:minP.x];
    [self.xAxisLayer setMaxValue:maxP.x];

    [self.yAxisLayer setMinValue:minP.y];
    [self.yAxisLayer setMaxValue:maxP.y];



    [self.gridLayer setNeedsDisplay];
    [self.yAxisLayer setNeedsDisplay];
    [self.xAxisLayer setNeedsDisplay];
    [self.legendLayer setNeedsDisplay];
}

@end
