/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUIColoredImage.m
//  benchmark
//
//  Created by kishonti on 25/08/2014.
//
//

#import "NUIColoredImage.h"

@implementation UIImage (NUITint)


+ (UIImage *)imageWithContentsOfFile:(NSString *)path withTintColor:(UIColor *)color
{
    UIImage* img = [UIImage imageWithContentsOfFile:path];
    
    // begin a new image context, to draw our colored image onto
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

@end
