//
//  gctestViewController.m
//  gctest
//
//  Created by Li Wei on 12-1-10.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import "gctestViewController.h"
#include "dms.h"

@implementation gctestViewController

-(IBAction)onLogin:(id)sender{
    GKLocalPlayer *localPlayer = [GKLocalPlayer localPlayer];
    dmsLogin([localPlayer.playerID UTF8String]);
}
-(IBAction)onLogout:(id)sender{
    dmsLogout();
}
-(IBAction)onHeart:(id)sender{
    dmsHeartBeat();
}

- (void)dealloc
{
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
}
*/

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

@end
