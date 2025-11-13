/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "testfw.h"
#import "schemas/descriptors.h"

class GraphicsContext;

@interface ViewController : UIViewController {
  GraphicsContext *ctx_;
  tfw::TestBase *test_;
  NSString *dataPrefix;
}

- (void)setTest:(tfw::TestBase *)test;
- (void)initContext;
- (void)setupMainViewWithContext:(GraphicsContext *)context;
- (void)start;
- (void)saveResult:(std::string)jsonString fileName:(NSString *)name toPath:(NSString*)path;
- (void)setupEnv:(tfw::Descriptor *)des;

@end
