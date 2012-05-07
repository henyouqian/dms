//
//  gctestViewController.m
//  gctest
//
//  Created by Li Wei on 12-1-10.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import "gctestViewController.h"
#include "dms.h"
#include "dmsError.h"

void MyDmsCallback::onLogin(int error, const char* gcid){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onLogin:error=%s", getDmsErrorString(error)];
    }else{
        str = [[NSString alloc] initWithFormat:@"onLogin:gcid=%s", gcid];
    }
    _pLabel.text = str;
    [str release];
}

void MyDmsCallback::onLogout(){
   _pLabel.text = @"onLogout";
}

void MyDmsCallback::onHeartBeat(int error){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onHeartBeat:error=%s", getDmsErrorString(error)];
    }else{
        str = @"onHeartBeat";
    }
    _pLabel.text = str;
    [str release];
}

void MyDmsCallback::onGetTodayGames(int error){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onGetTodayGames:error=%s", getDmsErrorString(error)];
    }else{
        str = [[NSString alloc] initWithFormat:@"onGetTodayGames"];
    }
    _pLabel.text = str;
    [str release];
}

void MyDmsCallback::onStartGame(int error, int gameid){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onStartGame:error=%s", getDmsErrorString(error)];
    }else{
        str = [[NSString alloc] initWithFormat:@"onStartGame:gameid=%d", gameid];
    }
    _pLabel.text = str;
    [str release];
}

void MyDmsCallback::onSubmitScore(int error, int gameid, int score){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onSubmitScore:error=%s", getDmsErrorString(error)];
    }else{
        str = [[NSString alloc] initWithFormat:@"onSubmitScore:gameid=%d,score=%d", gameid, score];
    }
    _pLabel.text = str;
    [str release];
}


@implementation gctestViewController

@synthesize tfFakeLogin;
@synthesize tfStartGameID;
@synthesize tfSubmitGameID;
@synthesize lbOutput;

-(void)setCallback{
    _pDmsCallback = new MyDmsCallback(lbOutput);
    dmsSetCallback(_pDmsCallback);
}

-(void)dealloc {
    if ( _pDmsCallback ){
        delete _pDmsCallback;
    }
    [super dealloc];
}

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
-(IBAction)onGetTodayGames:(id)sender{
    dmsGetTodayGames();
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
    dmsLogin([[tfFakeLogin text] UTF8String]);
}

-(IBAction)onBGTap:(id)sender{
    [tfFakeLogin resignFirstResponder];
    [tfStartGameID resignFirstResponder];
    [tfSubmitGameID resignFirstResponder];
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
