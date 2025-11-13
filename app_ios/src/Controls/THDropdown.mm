/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Controls/THDropdown.h"
#import "Common/THTheme.h"
#import "Controls/THButton.h"


@interface THDropdown()

@property (nonatomic, assign) NSInteger selectedIndex;
@property (nonatomic, strong) CALayer *dropLayer;

@property (nonatomic, strong) UIButton *leftPager;
@property (nonatomic, strong) UIButton *rightPager;
@property (nonatomic, strong) UIButton *dropButton;

@property (nonatomic, strong) UIView *droppedView;
@property (nonatomic, strong) UITableView *droppedTable;
@property (nonatomic, strong) UIView *selectedView;

@property (nonatomic, strong) id mHandler;

@end

@implementation THDropdown

- (id) init {
    self = [super init];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (void) setupWiTHTheme {
    // Setup default things
    self.selectedIndex = 0;
    self.dropped = false;
    self.direction = THDropDown;
    self.pagerSize = CGSizeMake(15, 23);
    self.padding = [THTheme getFloatNamed:@"Dropdown_Padding"];
    self.cellPadding = [THTheme getFloatNamed:@"Dropdown_CellPadding"];
    self.dropSize = [THTheme getFloatNamed:@"Dropdown_DropSize"];

    self.separatorColor = [THTheme getColorNamed:@"Dropdown_SeparatorColor"];
    self.dropBackColor = [THTheme getColorNamed:@"Dropdown_DropColor"];
    self.backgroundColor = [THTheme getColorNamed:@"Dropdown_BackColor"];
    self.shadowColor = [THTheme getColorNamed:@"Dropdown_ShadowColor"];

    self.pagerItemDist = [THTheme getFloatNamed:@"Dropdown_PagerItemDist"];
    self.separatorHeight = [THTheme getFloatNamed:@"Dropdown_SeparatorHeight"];
    self.shadowOffsetX = [THTheme getFloatNamed:@"Dropdown_ShadowOffsetX"];
    self.shadowOffsetY = [THTheme getFloatNamed:@"Dropdown_ShadowOffsetY"];
    self.shadowOpacity = [THTheme getFloatNamed:@"Dropdown_ShadowOpacity"];
    self.shadowRadius = [THTheme getFloatNamed:@"Dropdown_ShadowRadius"];

    self.pagerColor = [THTheme getColorNamed:@"Dropdown_PagerColor"];
    self.leftPagerImageName = @"theme_ios.bundle/custom_res/ArrowLeft";
    self.rightPagerImageName = @"theme_ios.bundle/custom_res/Arrow";


    self.dropButton = [[UIButton alloc] init];
    [self.dropButton setBackgroundColor:nil];
    [self.dropButton setTitle:@"" forState:UIControlStateNormal];
    [self.dropButton addTarget:self action:@selector(toggleDrop:) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:self.dropButton];

    self.leftPager = [[UIButton alloc] init];
    [self.leftPager setImage:[THTheme imageNamed:self.leftPagerImageName withTintColor:self.pagerColor] forState:UIControlStateNormal];
    [self.leftPager addTarget:self action:@selector(pageItems:) forControlEvents:UIControlEventTouchUpInside];
    [self.leftPager.imageView setContentMode:UIViewContentModeScaleAspectFit];
    [self addSubview:self.leftPager];

    self.rightPager = [[UIButton alloc] init];
    [self.rightPager setImage:[THTheme imageNamed:self.rightPagerImageName withTintColor:self.pagerColor] forState:UIControlStateNormal];
    [self.rightPager addTarget:self action:@selector(pageItems:) forControlEvents:UIControlEventTouchUpInside];
    [self.rightPager.imageView setContentMode:UIViewContentModeScaleAspectFit];
    [self addSubview:self.rightPager];

    self.droppedView = [[UIView alloc] init];
    [self.droppedView setFrame:[self getDropRectForDropped:self.dropped]];
    [self.droppedView setClipsToBounds:YES];

    self.droppedTable = [[UITableView alloc] initWithFrame:self.droppedView.bounds style:UITableViewStylePlain];
    [self.droppedTable setDataSource:self];
    [self.droppedTable setDelegate:self];
    [self.droppedTable setScrollEnabled:true];
    [self.droppedTable setUserInteractionEnabled:true];
    [self.droppedTable setSeparatorStyle:UITableViewCellSeparatorStyleNone];
    self.droppedTable.separatorInset = UIEdgeInsetsZero;
    self.droppedTable.layoutMargins = UIEdgeInsetsZero;
    self.droppedTable.allowsMultipleSelection = false;
    self.droppedTable.bounces = false;
    [self.droppedTable setBackgroundColor:[UIColor colorWithWhite:1.0 alpha:0.0]];

    [self.droppedView addSubview:self.droppedTable];
    [self addSubview:self.droppedView];

    [self layout];
}

- (IBAction)pageItems:(id)sender {
    self.dropped = false;
    if(sender == self.leftPager) {
        [self selectItemAtIndex:((self.selectedIndex-1 + self.items.count)%self.items.count)];
    } else {
        [self selectItemAtIndex:((self.selectedIndex+1)%self.items.count)];
    }
    [self layout];
}

-(void)updateConstraints {
    [super updateConstraints];
    [self layout];
}

- (IBAction)toggleDrop:(id)sender {
    self.dropped = !self.dropped;
}

- (id)getSelectedItem {
    if(self.items.count > self.selectedIndex)
        return self.items[self.selectedIndex];
    else
        return nil;
}

- (NSInteger)getSelectedIndex {
    return self.selectedIndex;
}

- (void)selectItem:(id)item {
    if([self.items containsObject:item]) {
        self.selectedIndex = [self.items indexOfObject:item];
        [self.mHandler dropdownItemSelected:item];
    }
    [self layout];
}

- (void)selectItemAtIndex:(NSInteger)index {
    if(self.items.count > index) {
        self.selectedIndex = index;
        [self.mHandler dropdownItemSelected:self.items[self.selectedIndex]];
    }
    [self layout];
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    [self layout];
}

- (void)layout {
    float padding = self.padding;

    [self.layer setShadowColor:self.shadowColor.CGColor];
    [self.layer setShadowOffset:CGSizeMake(self.shadowOffsetX, self.shadowOffsetY)];
    [self.layer setShadowOpacity:self.shadowOpacity];
    [self.layer setShadowRadius:self.shadowRadius];
    self.layer.masksToBounds = NO;

    if(self.leftPager != nil) {
        [self.leftPager setFrame:CGRectMake(padding,
                                            padding,
                                            self.pagerSize.width + self.pagerItemDist,
                                            self.frame.size.height - 2*padding)];

        float imageVerticalPadding = (self.leftPager.frame.size.height - self.pagerSize.height)*0.5f;
        [self.leftPager setImageEdgeInsets:UIEdgeInsetsMake(imageVerticalPadding,
                                                            0,
                                                            imageVerticalPadding,
                                                            self.pagerItemDist)];
    }
    if(self.rightPager != nil) {
        [self.rightPager setFrame:CGRectMake(self.frame.size.width - self.pagerSize.width - self.pagerItemDist - padding,
                                             padding,
                                             self.pagerSize.width + self.pagerItemDist,
                                             self.frame.size.height - 2*padding)];

        float imageVerticalPadding = (self.rightPager.frame.size.height - self.pagerSize.height)*0.5f;
        [self.rightPager setImageEdgeInsets:UIEdgeInsetsMake(imageVerticalPadding,
                                                             self.pagerItemDist,
                                                             imageVerticalPadding,
                                                             0)];
    }

    id selected = [self getSelectedItem];
    if(selected != nil) {
        if(self.selectedView != nil)
           [self.selectedView removeFromSuperview];

        self.selectedView = [selected createViewWithDropped:NO];
        [self.selectedView setFrame:CGRectMake(padding + self.pagerSize.width + self.pagerItemDist,
                                               padding,
                                               self.frame.size.width - 2*(self.pagerSize.width + padding + self.pagerItemDist),
                                               self.frame.size.height - 2*padding)];
        [self.selectedView setBackgroundColor:self.backgroundColor];
        [self addSubview:self.selectedView];
        [self sendSubviewToBack:self.selectedView];
    }

    if(self.dropButton != nil)
        [self.dropButton setFrame:CGRectMake(padding + self.pagerSize.width + self.pagerItemDist,
                                             padding,
                                             self.frame.size.width - 2*(self.pagerSize.width + padding + self.pagerItemDist),
                                             self.frame.size.height - 2*padding)];

}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.items.count;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    float padding = self.cellPadding;
    float pagerHeight = self.frame.size.height - 2*padding + self.separatorHeight * 2;
    return pagerHeight;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    float padding = self.cellPadding;

    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"DropCell"];
    if(cell==nil){
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"DropCell"];

    } else {
        for (UIView* view in cell.subviews) {
            [view removeFromSuperview];
        }
    }

    UIView *v = [self.items[indexPath.row] createViewWithDropped:YES];
    [v setFrame:CGRectMake(self.pagerSize.width + padding + self.pagerItemDist,
                           0,
                           tableView.frame.size.width - 2*(self.pagerSize.width + padding + self.pagerItemDist),
                           cell.frame.size.height - self.separatorHeight)];
    [v setBackgroundColor:[UIColor colorWithWhite:1.0 alpha:0.0]];
    [cell addSubview:v];

    UIView *separator = [[UIView alloc] initWithFrame:CGRectMake(0,
                                                                 cell.frame.size.height - self.separatorHeight,
                                                                 tableView.frame.size.width,
                                                                 self.separatorHeight)];
    [separator setBackgroundColor:self.separatorColor];
    [cell addSubview:separator];

    [cell setBackgroundColor:self.dropBackColor];

    cell.layoutMargins = UIEdgeInsetsZero;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    self.selectedIndex = indexPath.row;
    [self.mHandler dropdownItemSelected:self.items[self.selectedIndex]];
    [self toggleDrop:tableView];
    [tableView deselectRowAtIndexPath:indexPath animated:NO];
}


- (CGRect)getDropRectForDropped:(BOOL)dropped {
    if(self.direction == THDropDown) {
        return CGRectMake(0,
                          self.frame.size.height,
                          self.frame.size.width,
                          dropped ? self.dropSize : 0);
    }
    else {
        return CGRectMake(0,
                          - self.dropSize,
                          self.frame.size.width,
                          dropped ? self.dropSize : 0);
    }
}


// Needed to get hit events in a view out of the dropdown view's bounds.
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event {

    // Convert the point to the target view's coordinate system.
    // The target view isn't necessarily the immediate subview
    CGPoint pointForTargetView = [self.droppedView convertPoint:point fromView:self];

    if (CGRectContainsPoint(self.droppedView.bounds, pointForTargetView)) {

        // The target view may have its view hierarchy,
        // so call its hitTest method to return the right hit-test view
        return [self.droppedView hitTest:pointForTargetView withEvent:event];
    }

    return [super hitTest:point withEvent:event];
}

- (void)autoDropSize {
    float padding = self.cellPadding;
    float pagerHeight = self.frame.size.height - 2*padding + 2*self.separatorHeight;
    self.dropSize = pagerHeight * self.items.count;
}

#pragma mark - setters

-(void)setDropSize:(float)dropSize {
    _dropSize = dropSize;
    [self.droppedView setFrame:[self getDropRectForDropped:self.dropped]];
    [self.droppedTable setFrame:self.droppedView.bounds];
    [self layout];
}

- (void) setItems:(NSArray *)items {
    BOOL OK = true;
    for (id item in items) {
        if(![item conformsToProtocol:@protocol(DroppableObject)]) OK = false;
    }

    if(OK) {
        _items = items;
        self.selectedIndex = 0;
        if(self.items.count > 0) {
            [self.mHandler dropdownItemSelected:self.items[self.selectedIndex]];
        }

        if(self.droppedTable != nil) {
            [self.droppedTable reloadData];
        }
        [self layout];
    } else
        NSLog(@"Error, THDropdown needs DroppableObjects as items!");
}

- (void)setLeftPagerImageName:(NSString *)leftPagerImageName {
    _leftPagerImageName = leftPagerImageName;
    if(self.leftPager != nil) {
        [self.leftPager setImage:[THTheme imageNamed:self.leftPagerImageName withTintColor:self.pagerColor] forState:UIControlStateNormal];
    }
}

- (void)setRightPagerImageName:(NSString *)rightPagerImageName {
    _rightPagerImageName = rightPagerImageName;
    if(self.rightPager != nil) {
        [self.rightPager setImage:[THTheme imageNamed:self.rightPagerImageName withTintColor:self.pagerColor] forState:UIControlStateNormal];
    }
}

- (void)setDropped:(BOOL)dropped {
    if(_dropped != dropped) {
        CGRect base = [self getDropRectForDropped:true];
        CGRect offsetted = [self getDropRectForDropped:true];
        CGRect tableBase = CGRectMake(0, 0, base.size.width, base.size.height);
        CGRect tableOffsetted = CGRectMake(0, 0, base.size.width, base.size.height);
        self.droppedView.frame = base;

        offsetted.size.height = 0;

        if(self.direction == THDropDown) {
            tableOffsetted.origin.y -= self.dropSize;
        }
        else {
            offsetted.origin.y += self.dropSize;
        }

        float targetAlpha = 0;
        CGRect targetRect = base;
        CGRect tableTargetRect = tableBase;

        // If opening
        if(dropped) {
            self.droppedView.alpha = 0;
            self.droppedView.frame = offsetted;
            self.droppedTable.frame = tableOffsetted;
            targetAlpha = 1;
        }
        // Closing
        else {
            self.droppedView.alpha = 1;
            self.droppedView.frame = base;
            self.droppedTable.frame = tableBase;
            targetAlpha = 0;
            targetRect = offsetted;
            tableTargetRect = tableOffsetted;
        }

        [UIView animateWithDuration:0.2
                              delay:0.0
                            options:UIViewAnimationOptionCurveEaseInOut
                         animations:^{
                             self.droppedView.alpha = targetAlpha;
                             self.droppedView.frame = targetRect;
                             self.droppedTable.frame = tableTargetRect;
                         }
                         completion:^(BOOL finished){
                             //                             [self.droppedView setFrame:[self getDropRectForDropped:self.dropped]];
                         }];
        [self layout];
    }

    _dropped = dropped;
}

- (void)setDirection:(THDropdownDirection)direction {
    _direction = direction;
    [self.droppedView setFrame:[self getDropRectForDropped:self.dropped]];
    [self.droppedTable setFrame:self.droppedView.bounds];
    [self layout];
}

- (void)setPagerSize:(CGSize)pagerSize {
    _pagerSize = pagerSize;
    [self layout];
}

- (void)setPagerItemDist:(float)pagerItemDist {
    _pagerItemDist = pagerItemDist;
    [self layout];
}

- (void)setSeparatorHeight:(float)separatorHeight {
    _separatorHeight = separatorHeight;
    [self layout];
}

- (void)setShadowOffsetX:(float)shadowOffsetX {
    _shadowOffsetX = shadowOffsetX;
    [self layout];
}

- (void)setShadowOffsetY:(float)shadowOffsetY {
    _shadowOffsetY = shadowOffsetY;
    [self layout];
}

- (void)setShadowOpacity:(float)shadowOpacity {
    _shadowOpacity = shadowOpacity;
    [self layout];
}

- (void)setShadowRadius:(float)shadowRadius {
    _shadowRadius = shadowRadius;
    [self layout];
}

- (void)setSeparatorColor:(UIColor *)separatorColor {
    _separatorColor = separatorColor;
    [self layout];
}

- (void)setDropBackColor:(UIColor *)dropBackColor {
    _dropBackColor = dropBackColor;
    [self layout];
}

- (void)setPagerColor:(UIColor *)pagerColor {
    _pagerColor = pagerColor;
    if(self.leftPager != nil) {
        [self.leftPager setImage:[THTheme imageNamed:self.leftPagerImageName withTintColor:self.pagerColor] forState:UIControlStateNormal];
    }
    if(self.rightPager != nil) {
        [self.rightPager setImage:[THTheme imageNamed:self.rightPagerImageName withTintColor:self.pagerColor] forState:UIControlStateNormal];
    }
}

- (void)setShadowColor:(UIColor *)shadowColor {
    _shadowColor = shadowColor;
    [self layout];
}

- (void)setBackgroundColor:(UIColor *)backgroundColor {
    [super setBackgroundColor:backgroundColor];
    if(self.selectedView != nil) {
        [self.selectedView setBackgroundColor:backgroundColor];
    }
}

- (void)setPadding:(float)padding {
    _padding = padding;
    [self layout];
}

- (void)setCellPadding:(float)cellPadding {
    _cellPadding = cellPadding;
    if(self.droppedTable != nil) {
        [self.droppedTable reloadData];
    }
    [self layout];
}

- (void)setHandler:(id)handler {
    if([handler respondsToSelector:@selector(dropdownItemSelected:)]) {
        self.mHandler = handler;
    }
}


@end
