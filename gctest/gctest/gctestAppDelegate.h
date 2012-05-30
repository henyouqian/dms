//
//  gctestAppDelegate.h
//  gctest
//
//  Created by Li Wei on 12-1-10.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class gctestViewController;

@interface gctestAppDelegate : NSObject <UIApplicationDelegate> {

}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet gctestViewController *viewController;

@end
