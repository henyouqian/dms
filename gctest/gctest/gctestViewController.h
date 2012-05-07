//
//  gctestViewController.h
//  gctest
//
//  Created by Li Wei on 12-1-10.
//  Copyright 2012年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "dms.h"

class MyDmsCallback : public DmsCallback{
public:
    MyDmsCallback(UILabel* pLabel):_pLabel(pLabel){};
    virtual void onLogin(int error, const char* gcid);
    virtual void onLogout();
    virtual void onHeartBeat(int error);
    virtual void onGetTodayGames(int error);
    virtual void onStartGame(int error, int gameid);
    virtual void onSubmitScore(int error, int gameid, int score);
    
private:
    UILabel* _pLabel;
};


@interface gctestViewController : UIViewController {
    MyDmsCallback* _pDmsCallback;
}

@property (nonatomic, retain) IBOutlet UITextField* tfFakeLogin;
@property (nonatomic, retain) IBOutlet UITextField* tfStartGameID;
@property (nonatomic, retain) IBOutlet UITextField* tfSubmitGameID;
@property (nonatomic, retain) IBOutlet UILabel* lbOutput;

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
