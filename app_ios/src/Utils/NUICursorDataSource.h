/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUICursorDataSource.h
//  app_ios
//
//  Created by Balazs Hajagos on 25/08/2015.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "Utils/NUICursor.h"

@class NUICursorDataSource;

@protocol NUICursorDataSourceDelegate <NSObject>

- (void)cursorDataSource:(NUICursorDataSource *)dataSource handleCursorSelected:(NUICursor *)cursor atIndexPath:(NSIndexPath *)path;

@optional
- (void)cursorDataSource:(NUICursorDataSource *)dataSource dataSetChangedFrom:(NSInteger)from to:(NSInteger)to;
- (void)cursorDataSource:(NUICursorDataSource *)dataSource dataSetInvalidated:(NUICursor *)cursor;

@end

@interface NUICursorDataSource : NSObject<UITableViewDataSource, UITableViewDelegate, NUICursorCallback>

@property(strong, nonatomic) NUICursor *cursor;
@property(weak, nonatomic) UITableView *table;
@property(weak, nonatomic) id<NUICursorDataSourceDelegate> delegate;

@property(assign, nonatomic) NSUInteger cellHeight;
@property(assign, nonatomic) NSUInteger headerHeight;
@property(assign, nonatomic) NSUInteger footerHeight;
@property(assign, nonatomic) NSUInteger placeholderHeight;

- (id)initWithCursor:(NUICursor *)cursor
               Table:(UITableView *)table
            CellType:(Class)cell
          HeaderType:(Class)header;

- (id)initWithCursor:(NUICursor *)cursor
               Table:(UITableView *)table
            CellType:(Class)cell
          HeaderType:(Class)header
  PermanentSelection:(BOOL)selection;

- (id)initWithCursor:(NUICursor *)cursor
               Table:(UITableView *)table
            CellType:(Class)cell
          HeaderType:(Class)header
  PermanentSelection:(BOOL)selection
        AnimatedRows:(BOOL)animated;

- (id)initWithCursor:(NUICursor *)cursor
               Table:(UITableView *)table
            CellType:(Class)cell
          HeaderType:(Class)header
  PermanentSelection:(BOOL)selection
        AnimatedRows:(BOOL)animated
          SharedData:(NSDictionary *)shared;

- (void) setupWithCursor:(NUICursor *)cursor Table:(UITableView *)table CellType:(Class)cell HeaderType:(Class)header SharedData:(NSDictionary *)shared PermanentSelection:(BOOL)selection AnimatedRows:(BOOL)animated;

- (NSInteger)cursorIndexFromIndexPath:(NSIndexPath *)indexPath;

@end
