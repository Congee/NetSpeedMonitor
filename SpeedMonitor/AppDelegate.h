//
//  AppDelegate.h
//  SpeedMonitor
//
//  Created by Charles Wu on 3/23/16.
//  Copyright © 2016 Charles Wu. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SpeedProvider.h"

@interface AppDelegate : NSObject<NSApplicationDelegate> {
  IBOutlet NSMenu *statusMenu;
  IBOutlet NSMenuItem *quit;
  NSMutableAttributedString *speedString;
  struct if_data64 ifdata;

  NSWindow *window;
  NSStatusItem *statusItem;
}

@property IBOutlet NSWindow *window;
- (void)createStatusItem;

@end
