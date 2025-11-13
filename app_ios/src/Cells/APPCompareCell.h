/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#import "Cells/NUIBaseCell.h"

@interface APPCompareCell : NUIBaseCell

@property (weak, nonatomic) IBOutlet UIView *resultBack;
@property (weak, nonatomic) IBOutlet UILabel *mainResult;
@property (weak, nonatomic) IBOutlet UILabel *secondaryResult;
@property (weak, nonatomic) IBOutlet UIView *progressView;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *secondaryResultHeightConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *progressWidthConstraint;

@end
