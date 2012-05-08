//
//  gctestViewController.h
//  gctest
//
//  Created by Li Wei on 12-1-10.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

class Logger;


@interface gctestViewController : UIViewController {
    Logger* _pLogger;
}

@property (nonatomic, retain) IBOutlet UITextField* tfFakeLogin;
@property (nonatomic, retain) IBOutlet UITextField* tfStartGameID;
@property (nonatomic, retain) IBOutlet UITextField* tfSubmitGameID;
@property (nonatomic, retain) IBOutlet UITextView* tvOutput;

-(void)setCallback;

-(IBAction)onLogin:(id)sender;
-(IBAction)onLogout:(id)sender;
-(IBAction)onHeart:(id)sender;
-(IBAction)onGetTodayGames:(id)sender;
-(IBAction)onStartGames:(id)sender;
-(IBAction)onSubmitScore:(id)sender;
-(IBAction)onSubmitScoreRandom:(id)sender;
-(IBAction)onFakeLogin:(id)sender;
-(IBAction)onBGTap:(id)sender;

@end
