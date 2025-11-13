/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUICursor.mm
//  app_ios
//
//  Created by Kishonti Kft on 11/06/15.
//
//

#import "Utils/NUICursor.h"

class NUICursorCallbackWrapper: public CursorCallback
{
public:
    NUICursorCallbackWrapper()
    {
        mCallback = nil;
    }
    void BENCHMARK_SERVICE_API dataSetWillChange(int first, int last) override
    {
        if ([mCallback respondsToSelector:@selector(dataSetWillChangeFrom:to:)])
        {
            [mCallback dataSetWillChangeFrom:first to:last];
        }
    }
    void BENCHMARK_SERVICE_API dataSetChanged(int first, int last) override
    {
        if ([mCallback respondsToSelector:@selector(dataSetChangedFrom:to:)])
        {
            [mCallback dataSetChangedFrom:first to:last];
        }
    }
    void BENCHMARK_SERVICE_API dataSetWillBeInvalidated() override
    {
        if ([mCallback respondsToSelector:@selector(dataSetWillBeInvalidated)])
        {
            [mCallback dataSetWillBeInvalidated];
        }
    }
    void BENCHMARK_SERVICE_API dataSetInvalidated() override
    {
        if ([mCallback respondsToSelector:@selector(dataSetInvalidated)])
        {
            [mCallback dataSetInvalidated];
        }
    }
    id<NUICursorCallback> mCallback;
};



@implementation NUICursor

@dynamic callback, position, count;

+ (id)cursorWithPointer:(std::shared_ptr<Cursor>)pointer
{
    return [[NUICursor alloc] initWithCursor:pointer];
}



- (id)initWithCursor:(std::shared_ptr<Cursor>)pointer
{
    self = [super init];
    if (self) {
        cursorPointer = pointer;
        callbackWrapper = std::make_shared<NUICursorCallbackWrapper>();
        cursorPointer->setCallback(callbackWrapper.get());
    }
    return self;
}



- (Cursor*)getCursor
{
    return cursorPointer.get();
}



- (id<NUICursorCallback>)getCallback
{
    return callbackWrapper->mCallback;
}



- (void)setCallback:(id<NUICursorCallback>)callback
{
    callbackWrapper->mCallback = callback;
}



- (NSInteger)getPosition
{
    return cursorPointer->getPosition();
}



- (void)moveToPosition:(NSInteger)row
{
    cursorPointer->moveToPosition((int)row);
}



- (NSInteger)getColumnCount
{
    return cursorPointer->getColumnCount();
}



- (NSInteger)getCount
{
    return cursorPointer->getCount();
}



- (BOOL)getBooleanInColumn:(NSString *)column
{
    return cursorPointer->getBoolean(cursorPointer->getColumnIndex([column UTF8String])) ? YES : NO;
}



- (int64_t)getIntegerInColumn:(NSString *)column
{
    return cursorPointer->getLong(cursorPointer->getColumnIndex([column UTF8String]));
}



- (double)getDoubleInColumn:(NSString *)column
{
    return cursorPointer->getDouble(cursorPointer->getColumnIndex([column UTF8String]));
}



- (NSString*)getStringInColumn:(NSString *)column
{
    const char* pointer;
    size_t size;
    cursorPointer->getBlobPointer(cursorPointer->getColumnIndex([column UTF8String]), &pointer, &size);
    return [NSString stringWithUTF8String:pointer];
}



- (BOOL)columnIsNil:(NSString *)column
{
    return cursorPointer->isNull(cursorPointer->getColumnIndex([column UTF8String]));
}



@end
