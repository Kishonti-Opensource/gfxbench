/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDuelPage.m
//  app_ios
//
//  Created by Balazs Hajagos on 17/11/15.
//
//

#import "APPDuelPage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUICursor.h"
#import "Utils/NUIFadeCursorDataSource.h"
#import "Cells/APPDuelCell.h"


@interface APPDuelPage ()

@property (assign, nonatomic) BOOL isCustomized;
@property (strong, nonatomic) NUICursor *cursor;
@property (strong, nonatomic) NUICursorDataSource *duelDataSource;

@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@end


@implementation APPDuelPage

- (instancetype) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if(self) {
        [self setup];
    }
    return self;
}

- (instancetype) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if(self) {
        [self setup];
    }
    return self;
}




#pragma mark - Page Setup
- (void) setup {
    self.isCustomized = NO;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    
    self.tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapAnywhere:)];
}

- (void) customize {
    self.isCustomized = true;
    
    [self.backView addGestureRecognizer:self.tapRecognizer];
    
    [self applyTheme];
    [self applyLocalization];
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Base Page Overrides

- (void)layoutSubviews {
    [super layoutSubviews];
    if(!self.isCustomized)
        [self customize];
}

- (void) applyLocalization {
    [super applyLocalization];
    [self.table reloadData];
}

- (void) applyTheme {
    [super applyTheme];
    [self.table reloadData];
    
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRoundedRect:self.page.bounds cornerRadius:8.0];
    self.page.layer.masksToBounds = NO;
    self.page.layer.cornerRadius = 8;
    self.page.layer.shadowColor = [UIColor blackColor].CGColor;
    self.page.layer.shadowRadius = 10;
    self.page.layer.shadowOpacity = 0.5f;
    self.page.layer.shadowOffset = CGSizeMake(0, 0);
    self.page.layer.shadowPath = shadowPath.CGPath;
    
    [self.leftName setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
    [self.leftSubtitle setTextColor:[THTheme getColorNamed:@"TextNormalCellDescColor"]];
    [self.rightName setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
    [self.rightSubtitle setTextColor:[THTheme getColorNamed:@"TextNormalCellDescColor"]];
    
    [self.leftName setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.rightName setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.leftSubtitle setFont:[THTheme getFontNamed:@"TextSFont"]];
    [self.rightSubtitle setFont:[THTheme getFontNamed:@"TextSFont"]];
}

+ (APPDuelPage *)addDuelPageIn:(UIView *)holder
                   deviceAName:(NSString *)nameA
               deviceASubtitle:(NSString *)subtitleA
                  deviceAImage:(NSString *)imageA
                       forAApi:(NSString *)apiA
                   deviceBName:(NSString *)nameB
               deviceBSubtitle:(NSString *)subtitleB
                  deviceBImage:(NSString *)imageB
                       forBApi:(NSString *)apiB
                   hasCloseBtn:(BOOL)hasCloseBtn {
    APPDuelPage *ret = (APPDuelPage *)[APPOverlayPage addOverlayPageIn:holder withSpecificClass:[APPDuelPage class] displayingCloseBtn:hasCloseBtn];
    
    [ret setupPageWithDeviceAName:nameA
                  deviceASubtitle:subtitleA
                     deviceAImage:imageA
                          forAApi:apiA
                      deviceBName:nameB
                  deviceBSubtitle:subtitleB
                     deviceBImage:imageB
                          forBApi:apiB];
    
    return ret;
    
}

+ (APPDuelPage *)addDuelPageIn:(UIView *)holder
                   deviceAName:(NSString *)nameA
               deviceASubtitle:(NSString *)subtitleA
                  deviceAImage:(NSString *)imageA
                       forAApi:(NSString *)apiA
                   deviceBName:(NSString *)nameB
               deviceBSubtitle:(NSString *)subtitleB
                  deviceBImage:(NSString *)imageB
                       forBApi:(NSString *)apiB {
    APPDuelPage *ret = (APPDuelPage *)[APPOverlayPage addOverlayPageIn:holder withSpecificClass:[APPDuelPage class]];
    
    [ret setupPageWithDeviceAName:nameA
                  deviceASubtitle:subtitleA
                     deviceAImage:imageA
                          forAApi:apiA
                      deviceBName:nameB
                  deviceBSubtitle:subtitleB
                     deviceBImage:imageB
                          forBApi:apiB];
    
    return ret;
}

- (void)setupPageWithDeviceAName:(NSString *)nameA
                 deviceASubtitle:(NSString *)subtitleA
                    deviceAImage:(NSString *)imageA
                         forAApi:(NSString *)apiA
                     deviceBName:(NSString *)nameB
                 deviceBSubtitle:(NSString *)subtitleB
                    deviceBImage:(NSString *)imageB
                         forBApi:(NSString *)apiB {
    self.cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getDuelResults([apiA UTF8String], "own", [apiB UTF8String], [nameB UTF8String])];
    
    self.duelDataSource = [[NUIFadeCursorDataSource alloc] initWithCursor:self.cursor
                                                                    Table:self.table
                                                                 CellType:[APPDuelCell class]
                                                               HeaderType:[NSNull class]
                                                       PermanentSelection:false
                                                             AnimatedRows:true];
    
    self.table.dataSource = self.duelDataSource;
    self.table.delegate = self.duelDataSource;
    self.duelDataSource.delegate = self;
    
    self.leftImage.image = [THTheme imageNamed:imageA];
    self.rightImage.image = [THTheme imageNamed:imageB];
    self.leftName.text = [NUIAppData getLocalized:nameA];
    self.rightName.text = [NUIAppData getLocalized:nameB];
    self.leftSubtitle.text = [NUIAppData getLocalized:subtitleA];
    self.rightSubtitle.text = [NUIAppData getLocalized:subtitleB];
}


#pragma mark - Animation Overrides

- (void) overlayWillAppear {
    [self setAlpha:0.0];
    
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:-self.superview.frame.size.width * 0.5f];
    }
}

- (void) overlayAppearAnimation {
    [self layoutIfNeeded];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:8];
    }
    
    
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 1.0;
        [self layoutIfNeeded];
    }];
}

- (void) disappearWithCompletion:(void (^)(BOOL finished))completion; {
    [self layoutIfNeeded];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:-self.superview.frame.size.width * 0.5f];
    }
    
    
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 0.0;
        [self layoutIfNeeded];
        
    } completion:^(BOOL finished) {
        if(finished) {
            [self removeFromSuperview];
            if(completion != nil)
                completion(finished);
        }
    }];
}


#pragma mark - Event Handling

- (void)handleLocalizationChanged {
    [self applyLocalization];
}

-(void)didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self disappearWithCompletion:nil];
}



#pragma mark - CursorDelegate
- (void)cursorDataSource:(NUICursorDataSource *)dataSource handleCursorSelected:(NUICursor *)cursor atIndexPath:(NSIndexPath *)path {
    // do nothing
}

@end
