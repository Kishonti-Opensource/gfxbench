/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Utils/NUIUtilities.h"
#import <UIKit/UIKit.h>
#import <math.h>
#import "Common/THTheme.h"
#import "Common/NUIAppData.h"

@implementation NUIUtilities

static NSDateFormatter *uiFormatter;

+ (NSDateFormatter *)UIDateFormatter
{
    if(uiFormatter == nil)
    {
        uiFormatter =[[NSDateFormatter alloc] init];
        [uiFormatter setDateFormat:@"y.MM.d - HH:mm:ss"];
    }
    return uiFormatter;
}

+ (BOOL) BundleVersionIsGreaterThanOrEqual:(NSString *)targetVersion {
    return [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"BUIVersion"] compare:targetVersion options:NSNumericSearch] != NSOrderedAscending;
}

+ (NSString *) getProductId {
    return [[NSBundle mainBundle] objectForInfoDictionaryKey:(NSString*) @"BUIProdctId"];
}

+ (NSString *) getProductBaseApi {
    NSString *productId = [NUIUtilities getProductId];
    NSString *api = @"gl";
    if([productId containsString:@"_"]) {
        api = [productId substringFromIndex:[productId rangeOfString:@"_"].location + 1];
    }
    return api;
}

+ (NSString *) getVersionString {
    NSString *ret = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"BUIVersion"];
    if(ret == nil)
        ret = @"";

#if !IS_CORPORATE
    NSString *key = @"Version";
#else
    NSString *key = @"AppVersionCorporate";
#endif

    ret = [NSString stringWithFormat:[NUIAppData getLocalized:key], ret.UTF8String];

    return ret;
}

+ (NSDictionary *)getSystemVersion
{
    NSString *versionString = [[UIDevice currentDevice] systemVersion];
    NSRange dotRange = [versionString rangeOfString:@"."];
    if(dotRange.location == NSNotFound)
    {
        return @{
                 @"major": [NSNumber numberWithLong:[versionString integerValue]],
                 @"minor": [NSNumber numberWithInt:0]};
    }
    else
    {
        NSString *majorstring = [versionString substringToIndex:dotRange.location];
        NSString *minorstring = [versionString substringFromIndex:dotRange.location + 1];

        return @{
                 @"major": [NSNumber numberWithLong:[majorstring integerValue]],
                 @"minor": [NSNumber numberWithLong:[minorstring integerValue]]};
    }
}

+ (NSNumber *)ApplyMetric:(NSString *)metric To:(double)num
{
    NSNumber *ret = [NSNumber numberWithInt:num];

    if([metric isEqualToString:@"MTexels"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500000.0) / 1000000.0];
    }
    else if ([metric isEqualToString:@"MTexels/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500000.0) / 1000000.0];
    }
    else if ([metric isEqualToString:@"MTriangles/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500000.0) / 1000000.0];
    }
    else if ([metric isEqualToString:@"kTexels"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500.0) / 1000.0];
    }
    else if ([metric isEqualToString:@"kTexels/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500.0) / 1000.0];
    }
    else if ([metric isEqualToString:@"kTriangles/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500.0) / 1000.0];
    }
    else if ([metric isEqualToString:@"Kbit/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500.0) / 1000.0];
    }
    else if ([metric isEqualToString:@"kVertex/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500.0) / 1000.0];
    }
    else if ([metric isEqualToString:@"kFragment/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500.0) / 1000.0];
    }
    else if ([metric isEqualToString:@"kShader/s"])
    {
        ret = [NSNumber numberWithDouble:([ret doubleValue] + 500.0) / 1000.0];
    }

    return ret;
}

+ (NSString *)getStatusString:(NSString *)status
{
    if([status isEqualToString:@"FAILED"])
    {
        return @"Failed";
    }
    else if([status isEqualToString:@"CANCELLED"])
    {
        return @"Cancelled";
    }
    else if([status isEqualToString:@"INCOMPATIBLE"])
    {
        return @"Version not supported";
    }
    else
    {
        return @"N/A";
    }
}

+ (NSString *)getStatusStringFromEnum:(tfw::Result::Status)status
{
    if(status == tfw::Result::FAILED)
    {
        return @"Failed";
    }
    else if(status == tfw::Result::CANCELLED)
    {
        return @"Cancelled";
    }
    else if(status == tfw::Result::INCOMPATIBLE)
    {
        return @"Version not supported";
    }
    else
    {
        return @"N/A";
    }
}

+ (NSArray *)getStatusString:(NSString *)status withError:(NSString *)error
{
    NSString *eString = @"";
    if([error isEqualToString:@"NOERROR"])
    {
        eString = @"";
    }
    else if ([error isEqualToString:@"OUT_OF_VMEMORY"])
    {
        eString = @"Out of video memory";
    }
    else if ([error isEqualToString:@"SHADER_ERROR"])
    {
        eString = @"Shader error";
    }
    else if ([error isEqualToString:@"FILE_NOT_FOUND"])
    {
        eString = @"File not found";
    }
    else if ([error isEqualToString:@"UNKNOWNERROR"])
    {
        eString = @"Unknown error";
    }
    else if ([error isEqualToString:@"OFFSCREEN_NOT_SUPPORTED"])
    {
        eString = @"Your device does not support offscreen";
    }
    else if ([error isEqualToString:@"SKIPPED"])
    {
        eString = @"Skipped";
    }
    else if ([error isEqualToString:@"OUT_OF_MEMORY"])
    {
        eString = @"Out of memory";
    }
    else if ([error isEqualToString:@"BATTERYTEST_PLUGGED_ON_CHARGER"])
    {
        eString = @"Stopped, charger plugged";
    }
    else if ([error isEqualToString:@"VBO_ERROR"])
    {
        eString = @"VBO error";
    }
    else if ([error isEqualToString:@"UNSUPPORTED_TC_TYPE"])
    {
        eString = @"Unsupported texture format";
    }
    else if ([error isEqualToString:@"OFFSCREEN_NOT_SUPPORTED_IN_MSAA"])
    {
        eString = @"Your device does not support offscreen with MSAA";
    }
    else if ([error isEqualToString:@"INVALID_SCREEN_RESOLUTION"])
    {
        eString = @"Invalid screen resolution";
    }
    else if ([error isEqualToString:@"BATTERYTEST_BRIGHTNESS_CHANGED"])
    {
        eString = @"Stopped, brightness changed";
    }
    else if ([error isEqualToString:@"MOTIONBLUR_WITH_MSAA_NOT_SUPPORTED"])
    {
        eString = @"Your device does not support motion blur with MSAA";
    }
    else if ([error isEqualToString:@"ES3_NOT_SUPPORTED"])
    {
        eString = @"ES 3 not supported";
    }
    else if ([error isEqualToString:@"BUILT_WITH_INCOMPATIBLE_ES_VERSION"])
    {
        eString = @"Built with incompatible ES version";
    }
    else if ([error isEqualToString:@"GUI_BENCHMARK_NOT_SUPPORTED"])
    {
        eString = @"Not supported user interface";
    }
    else if ([error isEqualToString:@"DX_FEATURE_LEVEL"])
    {
        eString = @"DirectX feature level not available";
    }
    else if ([error isEqualToString:@"REQUIRED_FSAA_LEVEL_NOT_SUPPORTED"])
    {
        eString = @"Your device does not support the required FSAA level";
    }
    else if ([error isEqualToString:@"NETWORK_ERROR"])
    {
        eString = @"Network error";
    }
    else
    {
        eString = @"Undefined error";
    }

    if([status isEqualToString:@"FAILED"])
    {
        return @[@"Failed", eString];
    }
    else if([status isEqualToString:@"CANCELLED"])
    {
        return @[@"Cancelled", @""];
    }
    else if([status isEqualToString:@"INCOMPATIBLE"])
    {
        return @[@"ES3 not supported", @""];
    }
    else
    {
        return @[@"N/A", @""];
    }
}

+ (NSString *)getUnit:(NSString *)unit withScore:(NSNumber *)score withFps:(NSNumber *)fps
{
    if([unit isEqualToString:@"frames"])
        return [NSString stringWithFormat:@"(%.1f fps)", [fps floatValue]];
    else
        return unit;
}

+ (NSString *) getFormattedResult:(double)score
{
    int frac = MAX(0, 3 - (int) floor(log10(score)));
    NSString *formatterString = [NSString stringWithFormat:@"%%.%df", frac];
    return [NSString stringWithFormat:formatterString, score];
}

+ (NSString *)getDuelFormattedString:(NSNumber *)number
{
    float n = [number floatValue];
    float absDiff = (n < 0) ? n * (-1) : n;
    absDiff = (absDiff > 998) ? 998 : absDiff;
    n = absDiff;
    if(absDiff < 1)
    {
        n = n*100;

        int n_100 = (int)(n/100.0f);
        int n_10 = (int)(n/10.0f);

        if (n_100 > 0) return [NSString stringWithFormat:@"%.0f%%", n];
        if (n_10 > 0) return [NSString stringWithFormat:@"%.1f%%", n];
        if (n < 0.01) return @"0%";
        return [NSString stringWithFormat:@"%.2f%%", n];
    }
    else
    {
        n = n+1;

        int n_100 = (int)(n/100.0f);
        int n_10 = (int)(n/10.0f);
        int n_1 = (int)(n);

        if (n_100 > 0) return [NSString stringWithFormat:@"x%.0f", n];
        if (n_10 > 0) return [NSString stringWithFormat:@"x%.1f", n];
        if (n_1 > 0) return [NSString stringWithFormat:@"x%.2f", n];
        return [NSString stringWithFormat:@"x%.3f", n];
    }

    return 0;
}

+ (CGRect)getSizeForLabel:(UILabel *)l withMaxWidth:(CGFloat)width {
    NSString *text = l.text != nil ? l.text : @"";

    if([l.text isEqualToString:@""]) return CGRectMake(0, 0, width, 0);

    UIFont *font = l.font;
    NSAttributedString *attributedText =
    [[NSAttributedString alloc] initWithString:text attributes:@
     {
     NSFontAttributeName: font
     }];
    CGRect rect = [attributedText boundingRectWithSize:(CGSize){width, CGFLOAT_MAX}
                                               options:NSStringDrawingUsesLineFragmentOrigin
                                               context:nil];

    return rect;
}

+ (NSDictionary *)getDiagramInfo:(tfw::ResultItem)result {
//    std::vector<std::string> extra_data = result.result().gfxResult().extraData();
//    if(extra_data.size() > 0) {
//        NSMutableArray *battery = [[NSMutableArray alloc] init];
//        NSMutableArray *fps = [[NSMutableArray alloc] init];
//
//        for(int i = 0; i < extra_data.size()/2.0; ++i) {
//            NSString *battery_string = [NSString stringWithUTF8String:extra_data[i * 2].c_str()];
//            NSString *fps_string = [NSString stringWithUTF8String:extra_data[i * 2 + 1].c_str()];
//            [battery addObject:[NSNumber numberWithFloat:[battery_string floatValue]]];
//            [fps addObject:[NSNumber numberWithFloat:[fps_string floatValue]]];
//        }
//
//        return @{
//                 @"battery" : battery,
//                 @"fps" : fps,
//                 @"time" : [NSNumber numberWithInteger:result.result().elapsedTime()]
//                 };
//    }
//    else
    {
        return @{};
    }
}


+ (NSAttributedString *)getHeaderVersionStringWithBase:(NSString *)base {
    UIFont *baseFont = [THTheme getFontNamed:@"FontHeaderTitle"];
    UIFont *subFont = [THTheme getFontNamed:@"FontInfoCellMinor"];

    NSDictionary *baseAttrs = [NSDictionary dictionaryWithObjectsAndKeys:
                               baseFont, NSFontAttributeName, nil];
    NSDictionary *subAttrs = [NSDictionary dictionaryWithObjectsAndKeys:
                               subFont, NSFontAttributeName, nil];

    NSString *versionString = [NSString stringWithFormat:[NUIAppData getLocalized:@"Version"], ((NSString *)[[[NSBundle mainBundle] infoDictionary] objectForKey:@"BUIVersion"]).UTF8String];
    NSString *ret = [[NUIAppData getLocalized:base] stringByAppendingString:@"\n"];
    ret = [ret stringByAppendingString:versionString];

    const NSRange range = [ret rangeOfString:versionString];
    NSMutableAttributedString *attributedText =
    [[NSMutableAttributedString alloc] initWithString:ret
                                           attributes:baseAttrs];
    [attributedText setAttributes:subAttrs range:range];


    return attributedText;
}

+ (BOOL)shouldGetGeekyDetails:(NSString *)name {
    if([name isEqualToString:@"cpu"] ||
       [name isEqualToString:@"gpu"] ||
       [name isEqualToString:@"memory"] ||
       [name isEqualToString:@"storage"] ||
       [name isEqualToString:@"battery"]) {
        return NO;
    }

    return YES;
}

@end
