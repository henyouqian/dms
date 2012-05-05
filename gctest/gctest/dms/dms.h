#ifndef __DMS_H__
#define __DMS_H__

#define SECRET_KEY "480b4cf23ec84a1b93ab159f4938a3e5"

class DmsCallback;

void dmsInit(int appid);
void dmsDestroy();
void dmsMain();
void dmsSetCallback(DmsCallback* pCallback);

void dmsLogin(const char* gcid);
void dmsLogout();
void dmsHeartBeat();
void dmsGetTodayGames();
void dmsStartGame(int gameid);
void dmsSubmitScore(int gameid, int score);
void dmsGetTimeline(int offset);

struct Rank{
    Rank(int row, int rank, int score, const char* userName);
    int row;
    int rank;
    int score;
    std::string userName;
};

class DmsCallback{
public:
    virtual void onLogin(int error, const char* gcid) {};
    virtual void onLogout() {};
    virtual void onHeartBeat(int error) {};
    virtual void onGetTodayGames(int error) {};
    virtual void onStartGame(int error, int gameid){}
    virtual void onSubmitScore(int error, int gameid, int score) {};
    
};

void onLogin(int error, const char* gcid);
void onLogout();
void onHeartBeat(int error);
void onStartGame(int error, const char* token);
void onSubmitScore(int error, int gameid, int score);


#endif //__DMS_H__