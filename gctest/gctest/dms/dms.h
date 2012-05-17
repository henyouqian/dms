#ifndef __DMS_H__
#define __DMS_H__

#define APP_SECRET_KEY "6aa3b06bda37465ba506639d7035a763"
#define HEART_BEAT_SECOND 60

class DmsCallback;

void dmsInit();
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
    virtual void onHeartBeat(int error) {};
    virtual void onGetTodayGames(int error, const std::vector<DmsGame>& games) {};
    virtual void onStartGame(int error, int gameid) {};
    virtual void onSubmitScore(int error, int gameid, int score) {};
    virtual void onGetUnread(int error, int unread) {};
};


#endif //__DMS_H__