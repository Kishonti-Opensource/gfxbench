/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Cells/NUIBaseCell.h"

@interface APPDuelCell : NUIBaseCell

@property (weak, nonatomic) IBOutlet UIView *rightArrowBack;
@property (weak, nonatomic) IBOutlet UIView *leftArrowBack;
@property (weak, nonatomic) IBOutlet UIView *rightScoreBack;
@property (weak, nonatomic) IBOutlet UIView *leftScoreBack;

@property (weak, nonatomic) IBOutlet UIImageView *rightArrow;
@property (weak, nonatomic) IBOutlet UILabel *rightPercent;
@property (weak, nonatomic) IBOutlet UIImageView *leftArrow;
@property (weak, nonatomic) IBOutlet UILabel *leftArrowPercent;
@property (weak, nonatomic) IBOutlet UILabel *rightPrimaryScore;
@property (weak, nonatomic) IBOutlet UILabel *rightSecondaryScore;
@property (weak, nonatomic) IBOutlet UILabel *leftPrimaryScore;
@property (weak, nonatomic) IBOutlet UILabel *leftSecondaryScore;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *rightSecondaryHeightConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *leftSecondaryHeightConstraint;


@end
