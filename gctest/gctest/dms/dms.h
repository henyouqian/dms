#ifndef __DMS_H__
#define __DMS_H__

#define SECRET_KEY "96d8f494d9ce45d6bb26378b266d3e8b"
#define HEART_BEAT_SECOND 60

class DmsCallback;

void dmsInit(int appid);
void dmsDestroy();
void dmsSetCallback(DmsCallback* pCallback);

void dmsLogin(const char* gcid, const char* username);
void dmsLogout();
void dmsHeartBeat();
void dmsGetTodayGames();
void dmsStartGame(int gameid);
bool dmsSubmitScore(int gameid, int score);
void dmsGetUnread();
void dmsGetTimeline(int offset);

struct Rank{
    Rank(int row, int rank, int score, const char* userName);
    int row;
    int rank;
    int score;
    std::string userName;
};

struct DmsGame{
    int gameid;
    int score;
    std::string time;
};

class DmsCallback{
public:
    virtual void onError(const char* error) {};
    virtual void onLogin(int error, const char* gcid, const char* datetime) {};
    virtual void onLogout() {};
    virtual void onHeartBeat(int error, int unread) {};
    virtual void onGetTodayGames(int error, const std::vector<DmsGame>& games) {};
    virtual void onStartGame(int error, int gameid) {};
    virtual void onSubmitScore(int error, int gameid, int score) {};
    virtual void onGetUnread(int error, int num) {};
    
};

void onLogin(int error, const char* gcid, const char* datetime);
void onLogout();
void onHeartBeat(int error, int unread);
void onGetTodayGames(int error, const std::vector<DmsGame>& games);
void onStartGame(int error, const char* token, int gameid);
void onSubmitScore(int error, int gameid, int score);
void onGetUnread(int error, int num);


#endif //__DMS_H__