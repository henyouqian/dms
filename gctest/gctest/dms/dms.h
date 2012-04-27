#ifndef __DMS_H__
#define __DMS_H__

#define SECRET_KEY "480b4cf23ec84a1b93ab159f4938a3e5"

void dmsInit();
void dmsDestroy();
void dmsAddGame(int gameid);

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

void onLogin(int error, const char* gcid);
void onLogout();
void onHeartBeat(int error);


#endif //__DMS_H__