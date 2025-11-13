/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Cells/NUIBaseCell.h"
#import "Controls/DynamicCollectionView.h"

@interface APPDeviceinfoCell : NUIBaseCell<UICollectionViewDataSource, UICollectionViewDelegate, UICollectionViewDelegateFlowLayout>

@property (weak, nonatomic) IBOutlet UIImageView *toDetailIcon;
@property (weak, nonatomic) IBOutlet UILabel *minor;
@property (weak, nonatomic) IBOutlet DynamicCollectionView *attribCollection;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *attribHeight;

@end
