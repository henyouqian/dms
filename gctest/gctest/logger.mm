#include "logger.h"
#include "dms.h"
#include "dmsError.h"

Logger::Logger(UITextView* pTextView):_pTextView(pTextView){
    _str = [[NSMutableString alloc] init];
}

Logger::~Logger(){
    [_str release];
}

void Logger::onError(const char* text){
    NSString* str = [[NSString alloc] initWithUTF8String:text];
    [_str insertString:str atIndex:0];
    _pTextView.text = _str;
    [str release];
}

void Logger::onLogin(int error, const char* gcid, const char* datetime){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onLogin:error=%s\n", getDmsErrorString(error)];
    }else{
        str = [[NSString alloc] initWithFormat:@"onLogin:gcid=%s, datetime=%s\n", gcid, datetime];
    }
    [_str insertString:str atIndex:0];
    _pTextView.text = _str;
    [str release];
}

void Logger::onLogout(){
    [_str insertString:@"onLogout\n" atIndex:0];
    _pTextView.text = _str;
}

void Logger::onHeartBeat(int error){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onHeartBeat:error=%s\n", getDmsErrorString(error)];
    }else{
        str = @"onHeartBeat\n";
    }
    [_str insertString:str atIndex:0];
    _pTextView.text = _str;
    [str release];
}

void Logger::onGetTodayGames(int error, const std::vector<DmsGame>& games){
    NSMutableString* str = nil;
    if ( error ){
        str = [[NSMutableString alloc] initWithFormat:@"onGetTodayGames:error=%s\n", getDmsErrorString(error)];
    }else{
        str = [[NSMutableString alloc] initWithFormat:@"onGetTodayGames:\n"];
        std::vector<DmsGame>::const_iterator it = games.begin();
        std::vector<DmsGame>::const_iterator itend = games.end();
        for ( int i = 1; it != itend; ++it, ++i ){
            [str appendFormat:@"  %d: id=%d, score=%d\n", i, it->gameid, it->score];
        }
    }
    [_str insertString:str atIndex:0];
    _pTextView.text = _str;
    [str release];
}

void Logger::onStartGame(int error, int gameid){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onStartGame:error=%s\n", getDmsErrorString(error)];
    }else{
        str = [[NSString alloc] initWithFormat:@"onStartGame:gameid=%d\n", gameid];
    }
    [_str insertString:str atIndex:0];
    _pTextView.text = _str;
    [str release];
}

void Logger::onSubmitScore(int error, int gameid, int score){
    NSString* str = nil;
    if ( error ){
        str = [[NSString alloc] initWithFormat:@"onSubmitScore:error=%s\n", getDmsErrorString(error)];
    }else{
        str = [[NSString alloc] initWithFormat:@"onSubmitScore:gameid=%d,score=%d\n", gameid, score];
    }
    [_str insertString:str atIndex:0];
    _pTextView.text = _str;
    [str release];
}

