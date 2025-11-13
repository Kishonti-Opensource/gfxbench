/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUICursorCallback.h
//  GFXBench
//
//  Created by Kishonti Kft on 10/06/15.
//
//

#include "cursor.h"

#import <Foundation/Foundation.h>



@protocol NUICursorCallback <NSObject>

@optional
- (void) dataSetWillChangeFrom:(NSInteger)from to:(NSInteger)to;
- (void) dataSetChangedFrom:(NSInteger)from to:(NSInteger)to;
- (void) dataSetWillBeInvalidated;
- (void) dataSetInvalidated;

@end



class NUICursorCallbackWrapper;



@interface NUICursor: NSObject
{
    std::shared_ptr<Cursor> cursorPointer;
    std::shared_ptr<NUICursorCallbackWrapper> callbackWrapper;
}

@property(weak, nonatomic) id<NUICursorCallback> callback;
@property(nonatomic, readonly, getter=getCount) NSInteger count;
@property(nonatomic, readonly, getter=getColumnCount) NSInteger columnCount;
@property(nonatomic, getter=getPosition, setter=moveToPosition:) NSInteger position;

+ (id)cursorWithPointer:(std::shared_ptr<Cursor>)pointer;

- (BOOL)getBooleanInColumn:(NSString *)column;
- (int64_t)getIntegerInColumn:(NSString *)column;
- (double)getDoubleInColumn:(NSString *)column;
- (NSString*)getStringInColumn:(NSString *)column;

- (BOOL)columnIsNil:(NSString *)column;

@end
