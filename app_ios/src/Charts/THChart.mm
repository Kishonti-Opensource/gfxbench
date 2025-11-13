/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#import "Charts/THChart.h"
#import "Common/THTheme.h"

@interface THChart()

@property (nonatomic, assign) BOOL mHasData;

@end

@implementation THChart

- (id) init {
    self = [super init];
    if (self) {
        [self setup];
    }
    return self;
}

- (id) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self setup];
    }
    return self;
}

- (id) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setup];
    }
    return self;
}

- (id) initWithJsonString:(NSString *)jsonString {
    self = [super init];
    if (self) {
        [self setupWithJsonString:jsonString];
    }
    return self;
}

- (id) initWithFrame:(CGRect)frame WithJsonString:(NSString *)jsonString {
    self = [super initWithFrame:frame];
    if (self) {
        [self setupWithJsonString:jsonString];
    }
    return self;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    [self displayLayers];
}

- (void)fillDataWrapper {
    [self clearLayers];
    [self fillData];
    self.mHasData = true;
    [self displayLayers];
    [self animateLayers];
}

- (void)setup {
    self.mHasData = false;
}

- (void)setupWithJsonString:(NSString *)jsonString {
    [self setup];
    [self setDataJsonString:jsonString];
}

- (void)setupTransformation {}
- (void)fillData {}
- (void)clearLayers {}

- (void)setDataJsonString:(NSString *)jsonString {
    if(jsonString == nil) return;

    if([jsonString isEqualToString:@""]) {
        self.mHasData = false;
        return;
    }

    NSError *jsonError;
    NSData *objectData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
    _data = [NSJSONSerialization JSONObjectWithData:objectData
                                                         options:NSJSONReadingMutableContainers
                                                           error:&jsonError];
    [self setupTransformation];
    [self sortValues];
    [self fillDataWrapper];
    [self setNeedsDisplay];
}

- (void)sortValues {
    _sortedValues = [_data[@"values"] sortedArrayUsingComparator:^NSComparisonResult(id a, id b) {
        NSArray *first = (NSArray*)(a[@"values"]);
        NSArray *second = (NSArray*)(b[@"values"]);
        NSComparisonResult result = [[first valueForKeyPath:@"@avg.floatValue"] compare:[second valueForKeyPath:@"@avg.floatValue"]];

        if(result == NSOrderedAscending) return NSOrderedDescending;
        if(result == NSOrderedDescending) return NSOrderedAscending;
        return result;
    }];
}



- (void)displayLayers {}
- (void)animateLayers {}

- (BOOL)hasData { return self.mHasData; }

@end
