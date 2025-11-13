/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

/**
 * @brief Singleton class for managing pictures, themes.
 * @discussion This class has two main objectives. Th first is to manage
 * picture loading (since UIImage imagenamed is force caching). It stores,
 * loads and provides the images contacted through it. Recolored images
 * are stored only once then they are reused. With a simple
 * function call all loaded images and other resources can be freed.
 * It's second functionality is to load and provide themed assets to other
 * parts of the code like colors, fonts, sizes and so on. These assets are
 * are loaded at first request and stored until a clear cache call. After
 * that they can be reloaded with a new request for one of them.
 */
@interface THTheme : NSObject <NSXMLParserDelegate>

/**
 * @brief Request for a color named as the parameter. Loads assets.
 * @return The requested UIColor or nil if it's not found.
 */
+ (UIColor *)getColorNamed:(NSString *)name;
/**
 * @brief Request for a font named as the parameter. Loads assets.
 * @return The requested UIFont or nil if it's not found.
 */
+ (UIFont *)getFontNamed:(NSString *)name;
/**
 * @brief Request for an integer named as the parameter. Loads assets.
 * @return The requested NSInteger or 0 if it's not found.
 */
+ (NSInteger)getIntegerNamed:(NSString *)name;
/**
 * @brief Request for a float named as the parameter. Loads assets.
 * @return The requested CGFloat or 0 if it's not found.
 */
+ (CGFloat)getFloatNamed:(NSString *)name;

/**
 * @brief Request for an image named as the parameter. Loads the image.
 * @return The requested UIImage or nil if it's not found.
 */
+ (UIImage *) imageNamed:(NSString *)imageName;
/**
 * @brief Request for an image named as the parameter with fallback image. Loads the image.
 * @return The requested UIImage or the fallback if it's not found (fallback may still fail).
 */
+ (UIImage *) imageNamed:(NSString *)imageName withFallback:(NSString *)fallback;
/**
 * @brief Request for a tinted image named as the parameter. Loads the image.
 * @return The requested recolored UIImage or nil if it's not found.
 */
+ (UIImage *) imageNamed:(NSString *)imageName withTintColorName:(NSString *)tintName;
/**
 * @brief Request for a tinted image named as the parameter. Loads the image.
 * @return The requested recolored UIImage or nil if it's not found.
 */
+ (UIImage *) imageNamed:(NSString *)imageName withTintColor:(UIColor *)color;

// Helper functions
//+ (UIImage *)imageWithContentsOfFile:(NSString *)path withTintColor:(UIColor *)color;
+ (UIImage *)imageWithColor:(UIColor *)color;

/**
 * @brief Clears all cached data.
 * @discussion Removes all loaded and recolored image from the cache.
 * Also removes all loaded color, float, int, and font.
 */
+ (void) clearChachedData;

// Singleton
+ (THTheme *)sharedTheme;

@end
