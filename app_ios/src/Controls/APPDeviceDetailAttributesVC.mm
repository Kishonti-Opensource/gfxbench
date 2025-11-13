/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDeviceDetailAttributesVC.m
//  app_ios
//
//  Created by Balazs Hajagos on 02/10/2015.
//
//

#import "APPDeviceDetailAttributesVC.h"
#import "Common/THTheme.h"

@interface APPDeviceDetailAttributesVC ()

@end

@implementation APPDeviceDetailAttributesVC

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    self.attribTable.backgroundColor = [THTheme getColorNamed:@"BackColor"];
    self.attribTable.separatorColor = [THTheme getColorNamed:@"SeparatorColor"];
    
    [self.attribTable setDataSource:self];
    [self.attribTable setDelegate:self];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.attributes allKeys].count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"attribCell"];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:@"attribCell"];
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    
    NSString *key = [self.attributes allKeys][indexPath.row];
    cell.textLabel.text = key;
    
    NSObject *value = self.attributes[key];
    if([value isKindOfClass:[NSString class]]) {
        cell.detailTextLabel.text = (NSString *)value;
    } else if ([value respondsToSelector:@selector(stringValue)]){
        cell.detailTextLabel.text = [value performSelector:@selector(stringValue)];
    } else {
        cell.detailTextLabel.text = @"";
    }
    
    [cell.textLabel setTextColor:[THTheme getColorNamed:@"MainColor"]];
    
    return cell;
}

@end
