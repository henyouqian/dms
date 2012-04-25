#include "dms.h"
#include "dmsError.h"
#include "cJSON.h"

const char* SERVER_DATA_ERROR = "server data error";

Match::Match(int _matchId, const char* _matchName, const char* _date, int _gameId, const char* _gameName, int _score, const char* _finishTime, int _row, int _rank)
:matchId(_matchId), gameId(_gameId), score(_score), row(_row), rank(_rank){
    matchName = _matchName?_matchName:"";
    date = _date?_date:"";
    gameName = _gameName?_gameName:"";
    finishTime = _finishTime?_finishTime:"";
}

Rank::Rank(int _row, int _rank, int _score, const char* _userName)
:row(_row), rank(_rank), score(_score){
    userName = _userName?_userName:"";
}

namespace {
    
    struct Data{
        Data():pHttpClient(NULL), isLogin(false){}
        std::string gcid;
        bool isLogin;
        std::vector<Match> todayMatches;
        std::vector<Match> commingMatches;
        std::vector<Match> closedMatches;
        std::vector<Rank> ranks;
        lw::HTTPClient* pHttpClient;
    };
    
    Data* _pd = NULL;
    
    class MsgLogin : public lw::HTTPMsg{
    public:
        MsgLogin(const char* gcid)
        :lw::HTTPMsg("/dmsapi/user/login", _pd->pHttpClient, true){
            std::stringstream ss;
            ss << "?gcid=" << gcid << "&secretkey=" << SECRET_KEY;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            const char* s = _buff.c_str();
            const char* gcid = NULL;
            cJSON *json=cJSON_Parse(s);
            if ( !json ){
                error = DMSERR_SERVERDATA;
            }else{
                cJSON *jerror = cJSON_GetObjectItem(json, "error");
                if ( jerror && jerror->valueint){
                    error = jerror->valueint;
                }else{
                    cJSON* jgcid=cJSON_GetObjectItem(json, "gcid");
                    if ( !jgcid ){
                        error = DMSERR_SERVERDATA;
                    }else{
                        gcid = jgcid->valuestring;
                        _pd->isLogin = true;
                    }
                }
            }
            onLogin(error, gcid);
            cJSON_Delete(json);
        }
    };

    
    class MsgLogout : public lw::HTTPMsg{
    public:
        MsgLogout()
        :lw::HTTPMsg("/dmsapi/user/logout", _pd->pHttpClient, false){
        }
        virtual void onRespond(){
            onLogout();
        }
    };
    
    class MsgHeartBeat : public lw::HTTPMsg{
    public:
        MsgHeartBeat()
        :lw::HTTPMsg("/dmsapi/user/heartbeat", _pd->pHttpClient, false){
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            const char* s = _buff.c_str();
            cJSON *json=cJSON_Parse(s);
            if ( !json ){
                error = DMSERR_SERVERDATA;
            }else{
                cJSON *jerror = cJSON_GetObjectItem(json, "error");
                if ( jerror && jerror->valueint){
                    error = jerror->valueint;
                }
            }
            onHeartBeat(error);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetTodayMatches : public lw::HTTPMsg{
    public:
        MsgGetTodayMatches(int offset, int limit)
        :lw::HTTPMsg("/dms/usergettodaymatches", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?offset=" << offset << "&limit=" << limit;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            _pd->todayMatches.clear();
            const char* s = _buff.c_str();
            cJSON *json=cJSON_Parse(s);
            cJSON *jerror = cJSON_GetObjectItem(json, "error");
            const char* error = NULL;
            int matchNum = 0;
            if ( jerror && jerror->valuestring ){
                error = jerror->valuestring;
            }else{
                cJSON* jdata = cJSON_GetObjectItem(json, "data");
                if ( jdata ){
                    cJSON* jMatchNum = cJSON_GetObjectItem(jdata, "matchnum");
                    cJSON* jMatches = cJSON_GetObjectItem(jdata, "matches");
                    if ( jMatches && jMatchNum ){
                        matchNum = jMatchNum->valueint;
                        int n = cJSON_GetArraySize(jMatches);
                        for ( int i = 0; i < n; ++i ){
                            cJSON* jmatch = cJSON_GetArrayItem(jMatches, i);
                            cJSON* jmatchid=cJSON_GetObjectItem(jmatch, "matchid");
                            cJSON* jmatchname=cJSON_GetObjectItem(jmatch, "matchname");
                            cJSON* jdate=cJSON_GetObjectItem(jmatch, "date");
                            cJSON* jgameid=cJSON_GetObjectItem(jmatch, "gameid");
                            cJSON* jgamename=cJSON_GetObjectItem(jmatch, "gamename");
                            cJSON* jscore=cJSON_GetObjectItem(jmatch, "score");
                            cJSON* jtime=cJSON_GetObjectItem(jmatch, "time");
                            if ( jmatchid && jmatchname && jdate && jgameid && jgamename && jscore && jtime ){
                                Match m(jmatchid->valueint, jmatchname->valuestring, jdate->valuestring, jgameid->valueint, jgamename->valuestring, jscore->valueint, jtime->valuestring, 0, 0);
                                _pd->todayMatches.push_back(m);
                            }else{
                                error = SERVER_DATA_ERROR;
                                _pd->todayMatches.clear();
                                break;
                            }
                        }
                    }else{
                        error = SERVER_DATA_ERROR;
                    }
                }else{
                    error = SERVER_DATA_ERROR;
                }
            }
            onGetTodayMatches(error, _pd->todayMatches, matchNum);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetCommingMatches : public lw::HTTPMsg{
    public:
        MsgGetCommingMatches(int offset, int limit)
        :lw::HTTPMsg("/dms/usergetcommingmatches", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?offset=" << offset << "&limit=" << limit;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            _pd->commingMatches.clear();
            const char* s = _buff.c_str();
            cJSON *json=cJSON_Parse(s);
            cJSON *jerror = cJSON_GetObjectItem(json, "error");
            const char* error = NULL;
            int matchNum = 0;
            if ( jerror && jerror->valuestring ){
                error = jerror->valuestring;
            }else{
                cJSON* jdata = cJSON_GetObjectItem(json, "data");
                if ( jdata ){
                    cJSON* jMatchNum = cJSON_GetObjectItem(jdata, "matchnum");
                    cJSON* jMatches = cJSON_GetObjectItem(jdata, "matches");
                    if ( jMatchNum && jMatches ){
                        matchNum = jMatchNum->valueint;
                        int n = cJSON_GetArraySize(jMatches);
                        for ( int i = 0; i < n; ++i ){
                            cJSON* jmatch = cJSON_GetArrayItem(jMatches, i);
                            cJSON* jmatchid=cJSON_GetObjectItem(jmatch, "matchid");
                            cJSON* jmatchname=cJSON_GetObjectItem(jmatch, "matchname");
                            cJSON* jdate=cJSON_GetObjectItem(jmatch, "date");
                            cJSON* jgameid=cJSON_GetObjectItem(jmatch, "gameid");
                            cJSON* jgamename=cJSON_GetObjectItem(jmatch, "gamename");
                            if ( jmatchid && jmatchname && jdate && jgameid && jgamename ){
                                Match m(jmatchid->valueint, jmatchname->valuestring, jdate->valuestring, jgameid->valueint, jgamename->valuestring, 0, "", 0, 0);
                                _pd->commingMatches.push_back(m);
                            }else{
                                error = SERVER_DATA_ERROR;
                                _pd->commingMatches.clear();
                                break;
                            }
                        }
                    }else{
                        error = SERVER_DATA_ERROR;
                    }
                }else{
                    error = SERVER_DATA_ERROR;
                }
            }
            onGetCommingMatches(error, _pd->commingMatches, matchNum);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetClosedMatches : public lw::HTTPMsg{
    public:
        MsgGetClosedMatches(int offset, int limit)
        :lw::HTTPMsg("/dms/usergetclosedmatches", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?offset=" << offset << "&limit=" << limit;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            _pd->closedMatches.clear();
            const char* s = _buff.c_str();
            cJSON *json=cJSON_Parse(s);
            cJSON *jerror = cJSON_GetObjectItem(json, "error");
            const char* error = NULL;
            int matchNum = 0;
            if ( jerror && jerror->valuestring ){
                error = jerror->valuestring;
            }else{
                cJSON* jdata = cJSON_GetObjectItem(json, "data");
                if ( jdata ){
                    cJSON* jMatchNum = cJSON_GetObjectItem(jdata, "matchnum");
                    cJSON* jMatches = cJSON_GetObjectItem(jdata, "matches");
                    if ( jMatches && jMatchNum ){
                        matchNum = jMatchNum->valueint;
                        int n = cJSON_GetArraySize(jMatches);
                        for ( int i = 0; i < n; ++i ){
                            cJSON* jmatch = cJSON_GetArrayItem(jMatches, i);
                            cJSON* jmatchid=cJSON_GetObjectItem(jmatch, "matchid");
                            cJSON* jmatchname=cJSON_GetObjectItem(jmatch, "matchname");
                            cJSON* jdate=cJSON_GetObjectItem(jmatch, "date");
                            cJSON* jgameid=cJSON_GetObjectItem(jmatch, "gameid");
                            cJSON* jgamename=cJSON_GetObjectItem(jmatch, "gamename");
                            cJSON* jscore=cJSON_GetObjectItem(jmatch, "score");
                            cJSON* jtime=cJSON_GetObjectItem(jmatch, "time");
                            cJSON* jrow=cJSON_GetObjectItem(jmatch, "row");
                            cJSON* jrank=cJSON_GetObjectItem(jmatch, "rank");
                            if ( jmatchid && jmatchname && jdate && jgameid && jgamename && jscore && jtime && jrow && jrank ){
                                Match m(jmatchid->valueint, jmatchname->valuestring, jdate->valuestring, jgameid->valueint, jgamename->valuestring, jscore->valueint, jtime->valuestring, jrow->valueint, jrank->valueint);
                                _pd->closedMatches.push_back(m);
                            }else{
                                error = SERVER_DATA_ERROR;
                                _pd->closedMatches.clear();
                                break;
                            }
                        }
                    }else{
                        error = SERVER_DATA_ERROR;
                    }
                }else{
                    error = SERVER_DATA_ERROR;
                }
            }
            onGetClosedMatches(error, _pd->closedMatches, matchNum);
            cJSON_Delete(json);
        }
    };
    
    class MsgSubmitScore : public lw::HTTPMsg{
    public:
        MsgSubmitScore(int matchid, int score)
        :lw::HTTPMsg("/dms/usersubmitscore", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?matchid=" << matchid << "&score=" << score;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            const char* s = _buff.c_str();
            cJSON *json=cJSON_Parse(s);
            cJSON *jerror = cJSON_GetObjectItem(json, "error");
            const char* error = NULL;
            int matchid = 0;
            int score = 0;
            if ( jerror && jerror->valuestring ){
                error = jerror->valuestring;
            }else{
                cJSON *jdata = cJSON_GetObjectItem(json, "data");
                if ( jdata ){
                    cJSON *jmatchid = cJSON_GetObjectItem(jdata, "matchid");
                    cJSON *jscore = cJSON_GetObjectItem(jdata, "score");
                    if ( jmatchid && jscore ){
                        matchid = jmatchid->valueint;
                        score = jscore->valueint;
                    }else{
                        error = SERVER_DATA_ERROR;
                    }
                }else{
                    error = SERVER_DATA_ERROR;
                }
            }
            onSubmitScore(error, matchid, score);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetRanks : public lw::HTTPMsg{
    public:
        MsgGetRanks(int matchid, int offset, int limit)
        :lw::HTTPMsg("/dms/usergetranks", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?matchid=" << matchid << "&offset=" << offset << "&limit=" << limit;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            _pd->ranks.clear();
            const char* s = _buff.c_str();
            cJSON *json=cJSON_Parse(s);
            cJSON *jerror = cJSON_GetObjectItem(json, "error");
            const char* error = NULL;
            int matchid = 0;
            int offset = 0;
            if ( jerror && jerror->valuestring ){
                error = jerror->valuestring;
            }else{
                cJSON *jdata = cJSON_GetObjectItem(json, "data");
                if ( jdata ){
                    cJSON *jmatchid = cJSON_GetObjectItem(jdata, "matchid");
                    cJSON *joffset = cJSON_GetObjectItem(jdata, "offset");
                    cJSON *jranks = cJSON_GetObjectItem(jdata, "ranks");
                    if ( jmatchid && joffset && jranks ){
                        matchid = jmatchid->valueint;
                        offset = joffset->valueint;
                        int n = cJSON_GetArraySize(jranks);
                        for ( int i = 0; i < n; ++i ){
                            cJSON* jobj = cJSON_GetArrayItem(jranks, i);
                            cJSON* jrow=cJSON_GetObjectItem(jobj, "row");
                            cJSON* jusername=cJSON_GetObjectItem(jobj, "username");
                            cJSON* jrank=cJSON_GetObjectItem(jobj, "rank");
                            cJSON* jscore=cJSON_GetObjectItem(jobj, "score");
                            if ( jrow && jusername && jrank && jscore ){
                                Rank rank(jrow->valueint, jrank->valueint, jscore->valueint, jusername->valuestring);
                                _pd->ranks.push_back(rank);
                            }else{
                                error = SERVER_DATA_ERROR;
                                _pd->ranks.clear();
                                break;
                            }
                        }
                    }else{
                        error = SERVER_DATA_ERROR;
                    }
                }
            }
            onGetRanks(error, _pd->ranks, matchid, offset);
            cJSON_Delete(json);
        }
    };
    
}//namespace

void dmsInit(){
    lwassert(_pd==NULL);
    _pd = new Data;
    _pd->pHttpClient = new lw::HTTPClient("127.0.0.1:8000");
    _pd->pHttpClient->enableHTTPS(false);
}

void dmsDestroy(){
    lwassert(_pd);
    delete _pd->pHttpClient;
    delete _pd;
    _pd = NULL;
}

void dmsLogin(const char* gcid){
    lwassert(_pd);
    _pd->gcid = gcid;
    lw::HTTPMsg* pMsg = new MsgLogin(gcid);
    pMsg->send();
}

void dmsLogout(){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgLogout();
    pMsg->send();
}

void dmsHeartBeat(){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgHeartBeat();
    pMsg->send();
}

bool dmsCheckLogin(){
    lwassert(_pd);
    if ( !_pd->isLogin ){
        lw::HTTPMsg* pMsg = new MsgLogin(_pd->gcid.c_str());
        pMsg->send();
        return false;
    }
    return true;
}

bool dmsGetTodayMatches(int offset, int limit){
    lwassert(_pd);
    if ( !dmsCheckLogin() ){
        return false;
    }
    lw::HTTPMsg* pMsg = new MsgGetTodayMatches(offset, limit);
    pMsg->send();
    return true;
}

bool dmsGetCommingMatches(int offset, int limit){
    lwassert(_pd);
    if ( !dmsCheckLogin() ){
        return false;
    }
    lw::HTTPMsg* pMsg = new MsgGetCommingMatches(offset, limit);
    pMsg->send();
    return true;
}

bool dmsGetClosedMatches(int offset, int limit){
    lwassert(_pd);
    if ( !dmsCheckLogin() ){
        return false;
    }
    lw::HTTPMsg* pMsg = new MsgGetClosedMatches(offset, limit);
    pMsg->send();
    return true;
}

bool dmsGetRanks(int matchid, int offset, int limit){
    lwassert(_pd);
    if ( !dmsCheckLogin() ){
        return false;
    }
    lw::HTTPMsg* pMsg = new MsgGetRanks(matchid, offset, limit);
    pMsg->send();
    return true;
}

bool dmsSubmitScore(int matchid, int score){
    lwassert(_pd);
    if ( !dmsCheckLogin() ){
        return false;
    }
    lw::HTTPMsg* pMsg = new MsgSubmitScore(matchid, score);
    pMsg->send();
    return true;
}

