/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPLanguageSelectorPage.h
//  app_ios
//
//  Created by Balazs Hajagos on 11/01/16.
//
//

#import "APPOverlayPage.h"
#import "Utils/NUICursor.h"

@interface APPLanguageSelectorPage : APPOverlayPage<NUICursorCallback, UIPickerViewDataSource, UIPickerViewDelegate>

@property (weak, nonatomic) IBOutlet UIView *sheet;
@property (weak, nonatomic) IBOutlet UILabel *pageTitle;
@property (weak, nonatomic) IBOutlet UIPickerView *languagePicker;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *trailingConstraint;

@end
