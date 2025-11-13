/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPResultCell.h
//  app_ios
//
//  Created by Balazs Hajagos on 31/08/2015.
//
//

#import "Cells/NUIBaseCell.h"

@interface APPResultCell : NUIBaseCell

@property (weak, nonatomic) IBOutlet UIView *resultBack;
@property (weak, nonatomic) IBOutlet UILabel *mainResult;
@property (weak, nonatomic) IBOutlet UILabel *secondaryResult;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *secondaryResultHeightConstraint;

@end
