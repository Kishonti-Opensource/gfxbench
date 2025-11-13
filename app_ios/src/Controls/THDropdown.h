/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>

/**
 * @brief Protocol for Dropdown objects.
 * @discussion A protocol that defines the interface for objects that can
 * be stored and displayed in a dropdown control.
 */
@protocol DroppableObject

- (UIView *)createViewWithDropped:(BOOL)dropped;

@end

/**
 * @brief Protocol for Dropdown action handling.
 * @discussion A protocol that defines the interface to comunicate with
 * a dropdown object.
 */
@protocol DropdownActionHandler <NSObject>

- (void)dropdownItemSelected:(id)selected;

@end

/**
 * Defines the directions in which the dropdown control can drop.
 */
typedef NS_OPTIONS(NSUInteger, THDropdownDirection) {
    /// Drop downwards.
    THDropDown          = 0,
    /// Drop upwards.
    THDropUp            = 1 << 0
};

/**
 * @brief Dropdown control for iOS.
 * @discussion Droppable selector for iOS platform. Initializes based
 * on current theme. All properties are modifiable, and can drop in
 * two directions.
 */
@interface THDropdown : UIView<UITableViewDataSource, UITableViewDelegate>

/// The droppable areas height.
@property (nonatomic, assign) float dropSize;
/// Items to select from.
@property (nonatomic, strong) NSArray *items;
/// Indicates whether the control is currently dropped or not.
@property (nonatomic, assign) BOOL dropped;
/// The direction to use when dropping.
@property (nonatomic, assign) THDropdownDirection direction;

/// Padding to use in the main area.
@property (nonatomic, assign) float padding;
/// Padding to use in the cell area.
@property (nonatomic, assign) float cellPadding;

/// Image name to use for the left pager icon.
@property (nonatomic, strong) NSString *leftPagerImageName;
/// Image name to use for the right pager icon.
@property (nonatomic, strong) NSString *rightPagerImageName;

/// Size of the pager icon.
@property (nonatomic, assign) CGSize pagerSize;
/// Distance between the selected item and the pager icons.
@property (nonatomic, assign) float pagerItemDist;
/// Height of the separators between cells.
@property (nonatomic, assign) float separatorHeight;
/// Shadow offset in X direction.
@property (nonatomic, assign) float shadowOffsetX;
/// Shadow offset in Y direction.
@property (nonatomic, assign) float shadowOffsetY;
/// Shadow opacity [0,1].
@property (nonatomic, assign) float shadowOpacity;
/// Shadow radius.
@property (nonatomic, assign) float shadowRadius;

/// Color of the separator used in the drop area.
@property (nonatomic, strong) UIColor *separatorColor;
/// Background color of the drop area.
@property (nonatomic, strong) UIColor *dropBackColor;
/// Color of the pagers.
@property (nonatomic, strong) UIColor *pagerColor;
/// Color of the drop shadow.
@property (nonatomic, strong) UIColor *shadowColor;

/// Returns the selected item.
- (id)getSelectedItem;
/// Returns the selected index.
- (NSInteger)getSelectedIndex;
/// Selects the specified item.
- (void)selectItem:(id)item;
/// Selects the item at the given index.
- (void)selectItemAtIndex:(NSInteger)index;
/// Sets the dropsize of this object to fit the items.
- (void)autoDropSize;

/// Sets the handler for this object.
- (void)setHandler:(id)handler;

@end
