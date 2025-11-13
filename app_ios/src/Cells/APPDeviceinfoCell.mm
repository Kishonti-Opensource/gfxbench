/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "APPDeviceinfoCell.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Controls/NUITickedTextView.h"
#import "Utils/NUIUtilities.h"

@interface APPDeviceinfoCell()

@property (copy, nonatomic) NSString *iconName;
@property (strong, nonatomic) NUITickedTextView *sampleCell;
@property (strong, nonatomic) NSDictionary *attribDict;
@property (assign, nonatomic) BOOL headerLike;

@end

@implementation APPDeviceinfoCell

//
/**
 * Queries system information. E.g.: hardware and OS details, graphics and
 * compute api information. Attributes can be retrieved synchronously as a JSON string in the
 * column "attributesJson" or asynchronously by calling getSystemInfoAttributes.
 * @see getSystemInfoAttributes
 * @returns a cursor with the following columns: _id, title, icon, major, minor, isEnabled,
 * attributesJson.
 */
//

+ (NSString *)getCellId {
    return @"APPDeviceinfoCell";
}

+ (NSString *)getNibName {
    return @"APPDeviceinfoCell";
}

+ (BOOL)needsDynamicLayout {
    return YES;
}

- (void) setupWithCursor:(NUICursor *)cursor {
    self.headerLike = false;
    [super setupWithCursor:cursor];

    [self setupMinor:cursor];
    [self setupAttributes:cursor];

    self.attribCollection.dataSource = self;
    self.attribCollection.delegate = self;
    [self.attribCollection setUserInteractionEnabled:NO];
    [self.attribCollection
     registerNib:[UINib nibWithNibName:@"TickedTextView" bundle:nil]
     forCellWithReuseIdentifier:@"attribCell"];
}

- (void)setupIcon:(NUICursor *)cursor {
    self.iconName = [cursor getStringInColumn:@"icon"];
}

- (void)setupTitle:(NUICursor *)cursor {
    self.headerLike = [[[cursor getStringInColumn:@"title"] lowercaseString] isEqualToString:@"device"];
    NSString *title = [NUIAppData getLocalized:[[cursor getStringInColumn:@"title"] lowercaseString]];
    [self.titleLabel setText:title];
}

- (void)setupSubtitle:(NUICursor *)cursor {
    NSString *subtitle = [NUIAppData getLocalized:[cursor getStringInColumn:@"major"]];
    [self.subtitleLabel setText:subtitle];
}

- (void)setupMinor:(NUICursor *)cursor {
    NSString *minor = [NUIAppData getLocalized:[cursor getStringInColumn:@"minor"]];
    [self.minor setText:minor];
}

- (void)setupAttributes:(NUICursor *)cursor {
    if([NUIUtilities shouldGetGeekyDetails:[[cursor getStringInColumn:@"title"] lowercaseString]]) {
        [self.toDetailIcon setImage:[THTheme imageNamed:@"qMark_battery"]];
    }

    if([cursor getBooleanInColumn:@"isEnabled"]) {
        self.attribDict = @{};

    } else {
        NSString *attributes = [cursor getStringInColumn:@"compactAttributesJson"];
        NSData *attribData = [attributes dataUsingEncoding:NSUTF8StringEncoding];
        self.attribDict = [NSJSONSerialization JSONObjectWithData:attribData options:NSJSONReadingMutableContainers error:nil];
    }

    [self.attribCollection reloadData];

}

- (void)applyTheme {
    [super applyTheme];

    if(self.headerLike) {
        [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];
        [self setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
        [self.icon setImage:[THTheme imageNamed:self.iconName withTintColorName:@"BackColor"]];
        [self.minor setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];
        [self.subtitleLabel setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];

    } else {
        [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextInfoCellTitleColor"]];
        [self setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
        [self.icon setImage:[THTheme imageNamed:self.iconName withTintColorName:@"MainColor"]];
        [self.minor setTextColor:[THTheme getColorNamed:@"TextInfoCellMinorColor"]];
        [self.subtitleLabel setTextColor:[THTheme getColorNamed:@"TextInfoCellMajorColor"]];
    }

    [self.toDetailIcon setAlpha:0.25];

    [self.minor setFont:[THTheme getFontNamed:@"TextSFont"]];
    [self.subtitleLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.titleLabel setFont:[THTheme getFontNamed:@"TextLFont"]];

    [self.attribCollection setBackgroundColor:self.backgroundColor];
}

- (void)displayAsLastInGroup {
    // Do nothing
}


#pragma mark - Collection view

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView {
    return 1;
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    if(self.attribDict != nil)
        return [self.attribDict allKeys].count;

    return 0;
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout minimumInteritemSpacingForSectionAtIndex:(NSInteger)section {
    return 8;
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout minimumLineSpacingForSectionAtIndex:(NSInteger)section {
    return 4;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    NUITickedTextView *cell = (NUITickedTextView *)[collectionView dequeueReusableCellWithReuseIdentifier:@"attribCell" forIndexPath:indexPath];

    NSString *name = [self.attribDict allKeys][indexPath.row];
    BOOL value = [[self.attribDict objectForKey:name] boolValue];
    [cell setText:[NUIAppData getLocalized:name]];
    [cell setTicked:value];
    return cell;
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath {

    if(self.sampleCell == nil) {
        UIView *rootView = [[[UINib nibWithNibName:@"TickedTextView" bundle:nil] instantiateWithOwner:nil options:nil] lastObject];

        if ([rootView isKindOfClass:[NUITickedTextView class]]) {
            self.sampleCell = (NUITickedTextView*)rootView;

        } else {
            return CGSizeMake(0, 0);
        }
    }

    NSString *name = [self.attribDict allKeys][indexPath.row];
    BOOL value = [[self.attribDict objectForKey:name] boolValue];
    [self.sampleCell setText:[NUIAppData getLocalized:name]];
    [self.sampleCell setTicked:value];

    return [self.sampleCell preferredLayoutSizeFittingSize:CGSizeMake(50, 10)];
}




@end
