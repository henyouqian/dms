#ifndef __DMS_H__
#define __DMS_H__

#define SECRET_KEY "480b4cf23ec84a1b93ab159f4938a3e5"

void dmsInit();
void dmsDestroy();

void dmsLogin(const char* gcid);
void dmsLogout();
void dmsHeartBeat();
bool dmsGetTodayMatches(int offset, int limit);
bool dmsGetCommingMatches(int offset, int limit);
bool dmsGetClosedMatches(int offset, int limit);
bool dmsSubmitScore(int matchid, int score);
bool dmsGetRanks(int matchid, int offset, int limit);

struct Match{
    Match(int matchId, const char* matchName, const char* date, int gameId, const char* gameName, int score, const char* finishTime, int row, int rank);
    int matchId;
    int gameId;
    int score;
    int row;
    int rank;
    std::string gameName;
    std::string matchName;
    std::string date;
    std::string finishTime;
};

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
void onGetTodayMatches(const char* error, const std::vector<Match>& matches, int matchNumTotal);
void onGetCommingMatches(const char* error, const std::vector<Match>& matches, int matchNumTotal);
void onGetClosedMatches(const char* error, const std::vector<Match>& matches, int matchNumTotal);
void onSubmitScore(const char* error, int matchid, int score);
void onGetRanks(const char* error, const std::vector<Rank>& ranks, int matchId, int offset);



#endif //__DMS_H__