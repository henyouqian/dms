//
//  gctestViewController.m
//  gctest
//
//  Created by Li Wei on 12-1-10.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import "gctestViewController.h"
#include "logger.h"

@implementation gctestViewController

@synthesize tfFakeLogin;
@synthesize tfStartGameID;
@synthesize tfSubmitGameID;
@synthesize tvOutput;
@synthesize tfTimelineOffset;
@synthesize tfTimelineLimit;

-(void)setCallback{
    _pLogger = new Logger(tvOutput);
    dmsSetCallback(_pLogger);
}

-(void)dealloc {
    if ( _pLogger ){
        delete _pLogger;
    }
    [super dealloc];
}

-(IBAction)onLogin:(id)sender{
    GKLocalPlayer *localPlayer = [GKLocalPlayer localPlayer];
    dmsLogin([localPlayer.playerID UTF8String], [localPlayer.alias UTF8String]);
}

-(IBAction)onHeart:(id)sender{
    dmsHeartBeat();
}
-(IBAction)onGetTodayGames:(id)sender{
    dmsGetTodayGames();
    dmsGetUnread(); //test
}

-(IBAction)onStartGames:(id)sender{
    dmsStartGame([[tfStartGameID text] intValue]);
}

-(IBAction)onSubmitScore:(id)sender{
    dmsSubmitScore([[tfStartGameID text] intValue], [[tfSubmitGameID text] intValue]);
}

-(IBAction)onSubmitScoreRandom:(id)sender{
    dmsSubmitScore([[tfStartGameID text] intValue], rand()%1000);
}

-(IBAction)onFakeLogin:(id)sender{
    dmsLogin([[tfFakeLogin text] UTF8String], [[tfFakeLogin text] UTF8String]);
}

-(IBAction)onBGTap:(id)sender{
    [tfFakeLogin resignFirstResponder];
    [tfStartGameID resignFirstResponder];
    [tfSubmitGameID resignFirstResponder];
    [tfTimelineOffset resignFirstResponder];
    [tfTimelineLimit resignFirstResponder];
}

-(IBAction)onTimeline:(id)sender{
    dmsGetTimeline([[tfTimelineOffset text] intValue], [[tfTimelineLimit text] intValue]);
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewDidUnload{
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
