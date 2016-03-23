//
//  AppDelegate.m
//  SpeedMonitor
//
//  Created by Charles Wu on 3/23/16.
//  Copyright © 2016 Charles Wu. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate
@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  // Insert code here to initialize your application
	NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval: 1.0
													  target: self
													selector: @selector(updateStatusItem)
													userInfo: nil
													 repeats: YES];
	[timer fire];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
  // Insert code here to tear down your application
}

- (id)init {
	self = [super init];
	memset(&ifdata, 0, sizeof(ifdata));

	return self ? self : nil;
}

- (void)awakeFromNib {
	[self createStatusItem];
}

- (void)createStatusItem {
  statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
  statusMenu = [[NSMenu alloc] init];
  speedString = [[NSMutableAttributedString alloc] initWithString: @"debug"];
  quit       = [[NSMenuItem alloc] initWithTitle:@"quit"
										  action:@selector(terminate:)
								   keyEquivalent:@"q"];

  [statusItem setAttributedTitle:speedString];
  [statusItem setEnabled:NO];
  [statusItem setMenu:statusMenu];
  [statusMenu insertItem:quit atIndex:0];

  [self updateStatusItem];
}

- (void)updateStatusItem {
  [statusItem setEnabled:YES];

  struct ifmibdata ifmib;
  struct human_readble_string string = {0, NULL};

  fill_interface_data(&ifmib);
  size_t rx_bytes = ifmib.ifmd_data.ifi_ibytes - ifdata.ifi_ibytes;
  size_t tx_bytes = ifmib.ifmd_data.ifi_obytes - ifdata.ifi_obytes;

  humanize_digit(rx_bytes, &string);
  //NSLog(@"%@", [NSString stringWithFormat:@"⇡ %.3Lf%s\n", string.number, string.suffix]);

  NSFont *font             = [NSFont fontWithName:@"Lucida Grande" size:9];
  NSDictionary *attributes = [[NSDictionary alloc]
      initWithObjectsAndKeys:font, NSFontAttributeName, nil];

  [speedString setAttributedString: [[NSAttributedString alloc]
              initWithString:[NSString stringWithFormat:@"⇡ %.1Lf%s\n",
                                                        string.number,
                                                        string.suffix]
                  attributes:attributes]];

  humanize_digit(tx_bytes, &string);
  [speedString appendAttributedString: [[NSAttributedString alloc]
              initWithString:[NSString stringWithFormat:@"⇣ %.1Lf%s",
                                                        string.number,
                                                        string.suffix]
                  attributes:attributes]];

  [statusItem setAttributedTitle:speedString];

  ifdata = ifmib.ifmd_data;
}

@end
