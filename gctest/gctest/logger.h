#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "dms.h"

class Logger : public DmsCallback{
public:
    Logger(UITextView* pTextView);
    ~Logger();
    void addLog(const char* log);
    virtual void onError(const char* error);
    virtual void onLogin(int error, const char* gcid, const char* datetime);
    virtual void onLogout();
    virtual void onHeartBeat(int error);
    virtual void onGetTodayGames(int error, const std::vector<DmsGame>& games);
    virtual void onStartGame(int error, int gameid);
    virtual void onSubmitScore(int error, int gameid, int score);
    virtual void onGetUnread(int error, int unread, int topid);
    
private:
    UITextView* _pTextView;
    NSMutableString* _str;
};

#endif //__LOGGER_H__
