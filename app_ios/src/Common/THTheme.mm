/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Common/THTheme.h"
#import "Utils/NSFileManagerLocations.h"
#import "Utils/NUIColoredImage.h"

@interface THTheme()

@property (strong, nonatomic) NSMutableDictionary *loadedResources;
@property (strong, nonatomic) NSMutableDictionary *imageCache;

@end

@implementation THTheme
{
    BOOL errorParsing;
    NSXMLParser *parser;
    NSMutableString* currentElementValue;
    NSString *currentName;
    NSString *current;
    NSString *currentElementName;
}

+ (UIColor *)getColorNamed:(NSString *)name
{
    [[THTheme sharedTheme] initResourcesIfNeeded];
    UIColor *ret = [[THTheme sharedTheme].loadedResources objectForKey:name];
    return ret;
}

+ (UIFont *)getFontNamed:(NSString *)name
{
    [[THTheme sharedTheme] initResourcesIfNeeded];
    UIFont *ret = [[THTheme sharedTheme].loadedResources objectForKey:name];
    return ret;
}

+ (NSInteger)getIntegerNamed:(NSString *)name
{
    [[THTheme sharedTheme] initResourcesIfNeeded];
    NSInteger ret = [[[THTheme sharedTheme].loadedResources objectForKey:name] integerValue];
    return ret;
}

+ (CGFloat)getFloatNamed:(NSString *)name
{
    [[THTheme sharedTheme] initResourcesIfNeeded];
    CGFloat ret = [[[THTheme sharedTheme].loadedResources objectForKey:name] floatValue];
    return ret;
}

+ (UIColor *) colorWithHexString: (NSString *) hexString
{
    NSString *colorString = [[hexString stringByReplacingOccurrencesOfString: @"#" withString: @""] uppercaseString];
    CGFloat alpha, red, blue, green;
    switch ([colorString length]) {
        case 3: // #RGB
            alpha = 1.0f;
            red   = [THTheme colorComponentFrom: colorString start: 0 length: 1];
            green = [THTheme colorComponentFrom: colorString start: 1 length: 1];
            blue  = [THTheme colorComponentFrom: colorString start: 2 length: 1];
            break;
        case 4: // #ARGB
            alpha = [THTheme colorComponentFrom: colorString start: 0 length: 1];
            red   = [THTheme colorComponentFrom: colorString start: 1 length: 1];
            green = [THTheme colorComponentFrom: colorString start: 2 length: 1];
            blue  = [THTheme colorComponentFrom: colorString start: 3 length: 1];
            break;
        case 6: // #RRGGBB
            alpha = 1.0f;
            red   = [THTheme colorComponentFrom: colorString start: 0 length: 2];
            green = [THTheme colorComponentFrom: colorString start: 2 length: 2];
            blue  = [THTheme colorComponentFrom: colorString start: 4 length: 2];
            break;
        case 8: // #AARRGGBB
            alpha = [THTheme colorComponentFrom: colorString start: 0 length: 2];
            red   = [THTheme colorComponentFrom: colorString start: 2 length: 2];
            green = [THTheme colorComponentFrom: colorString start: 4 length: 2];
            blue  = [THTheme colorComponentFrom: colorString start: 6 length: 2];
            break;
        default:
            [NSException raise:@"Invalid color value" format: @"Color value %@ is invalid.  It should be a hex value of the form #RBG, #ARGB, #RRGGBB, or #AARRGGBB", hexString];
            break;
    }
    return [UIColor colorWithRed: red green: green blue: blue alpha: alpha];
}

+ (CGFloat) colorComponentFrom:(NSString *)string start:(NSUInteger)start length:(NSUInteger)length
{
    NSString *substring = [string substringWithRange: NSMakeRange(start, length)];
    NSString *fullHex = length == 2 ? substring : [NSString stringWithFormat: @"%@%@", substring, substring];
    unsigned hexComponent;
    [[NSScanner scannerWithString: fullHex] scanHexInt: &hexComponent];
    return hexComponent / 255.0;
}

+ (void) clearChachedData
{
    [[THTheme sharedTheme].imageCache removeAllObjects];
    [THTheme sharedTheme].imageCache = nil;
    [[THTheme sharedTheme].loadedResources removeAllObjects];
    [THTheme sharedTheme].loadedResources = nil;
}

+ (UIImage *) imageNamed:(NSString *)imageName
{
    return [self imageNamed:imageName withTintColorName:@""];
}

+ (UIImage *) imageNamed:(NSString *)imageName withFallback:(NSString *)fallback
{
    UIImage *img = [self imageNamed:imageName withTintColorName:@""];
    if(img == nil)
    {
        return [self imageNamed:fallback withTintColorName:@""];
    }

    return img;
}

+ (UIImage *) imageNamed:(NSString *)imageName withTintColorName:(NSString *)tintName
{
    UIColor *tint = [THTheme getColorNamed:tintName];
    return [THTheme imageNamed:imageName withTintColor:tint];
}

+ (UIImage *) imageNamed:(NSString *)imageName withTintColor:(UIColor *)tint {
    THTheme *theme = [THTheme sharedTheme];
    [theme initImageCacheIfNeeded];
    NSString *key = [NSString stringWithFormat:@"%@%@", imageName, [tint description]];
    UIImage *img = [theme.imageCache objectForKey:key];
    if(img == nil)
    {
        NSString* pathToImageFile = @"";
        if (imageName == nil) {
            imageName = @"";
        }

        if([[imageName lastPathComponent] isEqualToString:imageName]) {
            pathToImageFile = [[NSBundle mainBundle] pathForResource:imageName ofType:@"png"];
            if([pathToImageFile isEqualToString:@""]) {
                pathToImageFile = [[NSBundle mainBundle] pathForResource:imageName ofType:@"jpg"];
            }

        } else {
            pathToImageFile = imageName;
        }


        if(tint != nil) {
            img = [THTheme imageWithContentsOfFile:pathToImageFile withTintColor:tint];
        }
        else
        {
            img = [UIImage imageWithContentsOfFile:pathToImageFile];
        }

        if(img != nil)
        {
            [theme.imageCache setObject:img forKey:key];
        }
    }

    return img;
}

#pragma mark - resource loading

- (void)loadFromPath:(NSString *)basePath
{
    NSArray *themeFiles = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[NSString stringWithFormat:@"%@/theme",basePath] error:nil];

    for (int i = 0 ; i < themeFiles.count; i++)
    {
        NSString *themeFile = themeFiles[i];
        NSString *dataPath = [NSString stringWithFormat:@"%@/theme/%@", basePath, themeFile];
        NSData *data = [NSData dataWithContentsOfFile:dataPath];

        [self parseXml:data];
    }
}

- (void)parseXml:(NSData *)data
{
    errorParsing = FALSE;

    NSString *strXML = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSRange range = [strXML rangeOfString:@"<?xml"];
    if(range.location != NSNotFound)
    {
        strXML = [[strXML substringFromIndex:NSMaxRange(range)-5] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
        NSData *xmldata = [strXML dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES];

        parser = [[NSXMLParser alloc] initWithData:xmldata];
        [parser setDelegate:self];

        [parser setShouldProcessNamespaces:NO];
        [parser setShouldReportNamespacePrefixes:NO];
        [parser setShouldResolveExternalEntities:NO];

        [parser parse];
    }
}


- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError {

    NSString *errorString = [NSString stringWithFormat:@"Error code %li", (long)[parseError code]];
    NSLog(@"Error parsing XML: %@", errorString);

    errorParsing=YES;
}


- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
    if([elementName isEqualToString:@"color"])
    {
        [_loadedResources setObject:[THTheme colorWithHexString:attributeDict[@"hex"]]
                             forKey:attributeDict[@"name"]];
    }
    else if([elementName isEqualToString:@"font"])
    {
        [_loadedResources setObject:[UIFont fontWithName:attributeDict[@"type"]
                                                    size:[attributeDict[@"size"] integerValue]]
                             forKey:attributeDict[@"name"]];
    }
    else if([elementName isEqualToString:@"int"])
    {
        [_loadedResources setObject:attributeDict[@"value"]
                             forKey:attributeDict[@"name"]];
    }
    else if([elementName isEqualToString:@"float"])
    {
        [_loadedResources setObject:attributeDict[@"value"]
                             forKey:attributeDict[@"name"]];
    }

    currentElementValue = nil;
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string {

    if(!currentElementValue)
        currentElementValue = [[NSMutableString alloc] initWithString:string];
    else
        [currentElementValue appendString:string];
}


- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
}

- (void)parserDidEndDocument:(NSXMLParser *)parser
{
    if (errorParsing == NO)
    {
        NSLog(@"Theme string processing done!");
    }
    else
    {
        NSLog(@"Error occurred during theme string processing");
    }
}


+ (UIImage *)imageWithContentsOfFile:(NSString *)path withTintColor:(UIColor *)color
{
    UIImage* img = [UIImage imageWithContentsOfFile:path];

    // begin a new image context, to draw our colored image onto
    UIGraphicsBeginImageContext(img.size);
    UIGraphicsBeginImageContext(img.size);

    // get a reference to that context we created
    CGContextRef context = UIGraphicsGetCurrentContext();

    if(context != NULL)
    {
        // set the fill color
        [color setFill];

        // translate/flip the graphics context (for transforming from CG* coords to UI* coords
        CGContextTranslateCTM(context, 0, img.size.height);
        CGContextScaleCTM(context, 1.0, -1.0);

        // set the blend mode to color copy, and the original image
        CGContextSetBlendMode(context, kCGBlendModeCopy);
        CGRect rect = CGRectMake(0, 0, img.size.width, img.size.height);
        //CGContextDrawImage(context, rect, img.CGImage); //--no need to draw original image

        // set a mask that matches the shape of the image, then draw (color burn) a colored rectangle
        CGContextClipToMask(context, rect, img.CGImage);
        CGContextAddRect(context, rect);
        CGContextDrawPath(context,kCGPathFill);

        // generate a new UIImage from the graphics context we drew onto
        UIImage *coloredImg = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();

        //return the color-burned image
        return coloredImg;
    }

    return NULL;
}

+ (UIImage *)imageWithColor:(UIColor *)color {
    CGRect rect = CGRectMake(0.0f, 0.0f, 1.0f, 1.0f);
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context = UIGraphicsGetCurrentContext();

    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextFillRect(context, rect);

    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    return image;
}



#pragma mark - singleton

// Singleton
+ (THTheme *)sharedTheme
{
    static THTheme *sharedMyTHTheme = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedMyTHTheme = [[self alloc] init];
    });
    return sharedMyTHTheme;
}

- (id)init {
    if (self = [super init])
    {
        _loadedResources = nil;
        _imageCache = nil;
    }
    return self;
}

- (void)initResourcesIfNeeded
{
    if(_loadedResources == nil) {
        NSLog(@"Resource store initialization");
        _loadedResources = [[NSMutableDictionary alloc] init];
        [self loadFromPath:[[NSBundle mainBundle] resourcePath]];
    }
}

- (void)initImageCacheIfNeeded
{
    if(_imageCache == nil) {
        NSLog(@"Image cache initialization");
        _imageCache = [[NSMutableDictionary alloc] init];
    }
}

@end
