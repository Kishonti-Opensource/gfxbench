/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDeviceDetailAttributesVC.h
//  app_ios
//
//  Created by Balazs Hajagos on 02/10/2015.
//
//

#import <UIKit/UIKit.h>

@interface APPDeviceDetailAttributesVC : UIViewController<UITableViewDataSource, UITableViewDelegate>

@property (weak, nonatomic) IBOutlet UITableView *attribTable;
@property (strong, nonatomic) NSDictionary *attributes;

@end
