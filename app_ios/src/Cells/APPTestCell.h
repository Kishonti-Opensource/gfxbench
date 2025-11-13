/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import "Cells/NUIBaseCell.h"

@interface APPTestCell : NUIBaseCell

@property (weak, nonatomic) IBOutlet UIImageView *tickIcon;
@property (weak, nonatomic) IBOutlet UILabel *disableLabel;

- (void) updateSelectionFromCursor:(NUICursor *)cursor;

@end
