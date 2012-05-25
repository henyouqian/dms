#include "dms.h"
#include "dmsError.h"
#include "cJSON.h"
#include "dmsLocalDB.h"

void onLogin(int error, const char* gcid, const char* datetime);
void onLogout();
void onHeartBeat(int error);
void onGetTodayGames(int error, const std::vector<DmsGame>& games);
void onStartGame(int error, const char* token, int gameid);
void onSubmitScore(int error, int gameid, int score);
void onGetUnread(int error, int unread, int topid);
void onGetTimeline(int error, std::vector<DmsRank>& ranks);

@class DmsMain;

namespace {
    struct Data{
        Data():pHttpClient(NULL), isLogin(false){}
        std::string gcid;
        bool isLogin;
        lw::HTTPClient* pHttpClient;
        DmsCallback* pCallback;
        std::string gameStartToken;
        DmsMain* dmsMain;
        DmsLocalDB* pLocalDB;
    };
    
    Data* _pd = NULL;
    void netErrorCallback(){
        lwinfo("netErrorCallback");
        _pd->isLogin = false;
    }
}

@interface DmsMain : NSObject {
@private
    int _tHeartBeat;
}
@end

@implementation DmsMain

- (id)init
{
    if ( self =[super init] ){
        [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(timerAdvanced:) userInfo:nil repeats:YES];
        _tHeartBeat = 0;
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)timerAdvanced:(NSTimer *)timer{
    if ( _pd->isLogin ){
        if ( _tHeartBeat >= HEART_BEAT_SECOND ){
            ++_tHeartBeat;
            dmsHeartBeat();
            _tHeartBeat = 0;
        }
    }else{
        
    }
}

@end

namespace {
    
    cJSON* parseMsg(const char* strMsg, int& error){
        error = DMSERR_NONE;
        cJSON *json=cJSON_Parse(strMsg);
        if ( !json ){
            error = DMSERR_JSON;
        }else{
            cJSON *jerror = cJSON_GetObjectItem(json, "error");
            if ( jerror && jerror->valueint){
                error = jerror->valueint;
            }
        }
        return json;
    }
    int getJsonInt(cJSON* json, const char* key, int& error){
        if ( error != DMSERR_NONE ){
            return 0;
        }
        cJSON* jint=cJSON_GetObjectItem(json, key);
        if ( !jint ){
            error = DMSERR_JSON;
        }else{
            if ( jint->type == cJSON_Number ){
                return jint->valueint;
            }else{
                error = DMSERR_JSON;
            }
        }
        return 0;
    }
    bool getJsonBool(cJSON* json, const char* key, int& error){
        if ( error != DMSERR_NONE ){
            return false;
        }
        cJSON* jitem=cJSON_GetObjectItem(json, key);
        if ( !jitem ){
            error = DMSERR_JSON;
        }else{
            if ( jitem->type == cJSON_False ){
                return false;
            }else if ( jitem->type == cJSON_True ){
                return true;
            }else{
                error = DMSERR_JSON;
            }
        }
        return false;
    }
    const char* getJsonString(cJSON* json, const char* key, int& error){
        if ( error != DMSERR_NONE ){
            return NULL;
        }
        cJSON* jobj=cJSON_GetObjectItem(json, key);
        if ( !jobj ){
            error = DMSERR_JSON;
        }else{
            if ( jobj->type == cJSON_String ){
                return jobj->valuestring;
            }else{
                error = DMSERR_JSON;
            }
        }
        return NULL;
    }
    
    cJSON* getJsonArray(cJSON* json, const char* key, int& error){
        if ( error != DMSERR_NONE ){
            return NULL;
        }
        cJSON* jobj=cJSON_GetObjectItem(json, key);
        if ( !jobj ){
            error = DMSERR_JSON;
        }else{
            if ( jobj->type == cJSON_Array ){
                return jobj;
            }else{
                error = DMSERR_JSON;
            }
        }
        return NULL;
    }
    
    class MsgLogin : public lw::HTTPMsg{
    public:
        MsgLogin(const char* gcid, const char* username)
        :lw::HTTPMsg("/dmsapi/user/login", _pd->pHttpClient, true){
            std::stringstream ss;
            ss << "?gcid=" << gcid << "&appsecretkey=" << APP_SECRET_KEY << "&username=" << username;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            const char* gcid = NULL;
            const char* datetime = NULL;
            cJSON *json=parseMsg(_buff.c_str(), error);
            if ( error == DMSERR_NONE ){
                gcid = getJsonString(json, "gcid", error);
                if ( gcid ){
                    _pd->gcid = gcid;
                    _pd->isLogin = true;
                }
            }
            if ( error == DMSERR_NONE ){
                datetime = getJsonString(json, "time", error);
            }
            onLogin(error, gcid, datetime);
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
            const char* datetime = NULL;
            cJSON *json=parseMsg(_buff.c_str(), error);
            if ( error == DMSERR_NONE ){
                datetime = getJsonString(json, "games", error);
                lwinfo(datetime);
            }
            onHeartBeat(error);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetTodayGames : public lw::HTTPMsg{
    public:
        MsgGetTodayGames()
        :lw::HTTPMsg("/dmsapi/user/gettodaygames", _pd->pHttpClient, false){
            
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            cJSON *json=parseMsg(_buff.c_str(), error);
            cJSON *jgames = NULL;
            if ( error == DMSERR_NONE ){
                jgames = getJsonArray(json, "games", error);
            }
            std::vector<DmsGame> games;
            if ( error == DMSERR_NONE ){
                int sz = cJSON_GetArraySize(jgames);
                for ( int i = 0; i < sz; ++i ){
                    DmsGame game;
                    cJSON* jitem = cJSON_GetArrayItem(jgames,i);
                    game.gameid = getJsonInt(jitem, "gameid", error);
                    if ( error == DMSERR_NONE ){
                        game.score = getJsonInt(jitem, "score", error);
                    }
                    if ( error == DMSERR_NONE ){
                        const char* str = getJsonString(jitem, "time", error);
                        if ( str ){
                            game.time = str;
                        }
                    }
                    if ( error == DMSERR_NONE ){
                        games.push_back(game);
                    }
                }
            }
            onGetTodayGames(error, games);
            cJSON_Delete(json);
        }
    };
    
    class MsgStartGame : public lw::HTTPMsg{
    public:
        MsgStartGame(int gameid)
        :lw::HTTPMsg("/dmsapi/user/startgame", _pd->pHttpClient, false), _gameID(gameid){
            std::stringstream ss;
            ss << "?gameid=" << gameid;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            const char* token = NULL;
            cJSON *json=parseMsg(_buff.c_str(), error);
            if ( error == DMSERR_NONE ){
                token = getJsonString(json, "token", error);
                if ( token ){
                    _pd->gameStartToken = token;
                }
            }
            onStartGame(error, token, _gameID);
            cJSON_Delete(json);
        }
    private:
        int _gameID;
    };
    
    class MsgSubmitScore : public lw::HTTPMsg{
    public:
        MsgSubmitScore(int gameid, int score)
        :lw::HTTPMsg("/dmsapi/user/submitscore", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?token=" << _pd->gameStartToken << "&gameid=" << gameid << "&score=" << score;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            _pd->gameStartToken.clear();
            int error = DMSERR_NONE;
            int gameid = -1;
            int score = 0;
            cJSON *json=parseMsg(_buff.c_str(), error);
            if ( error == DMSERR_NONE ){
                gameid = getJsonInt(json, "gameid", error);
                score = getJsonInt(json, "score", error);
            }
            onSubmitScore(error, gameid, score);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetUnread : public lw::HTTPMsg{
    public:
        MsgGetUnread()
        :lw::HTTPMsg("/dmsapi/user/getunread", _pd->pHttpClient, false){
            
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            int unread = 0;
            int topid = 0;
            cJSON *json=parseMsg(_buff.c_str(), error);
            if ( error == DMSERR_NONE ){
                unread = getJsonInt(json, "unread", error);
            }
            if ( error == DMSERR_NONE ){
                topid = getJsonInt(json, "topid", error);
            }
            onGetUnread(error, unread, topid);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetTimeline : public lw::HTTPMsg{
    public:
        MsgGetTimeline(int offset, int limit)
        :lw::HTTPMsg("/dmsapi/user/gettimeline", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?offset=" << offset << "&limit=" << limit;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            cJSON *json=parseMsg(_buff.c_str(), error);
            DmsRank rank;
            std::vector<DmsRank> ranks;
            if ( error == DMSERR_NONE ){
                cJSON *jRanks = getJsonArray(json, "ranks", error);
                if ( jRanks && error == DMSERR_NONE ){
                    int sz = cJSON_GetArraySize(jRanks);
                    for ( int i = 0; i < sz; ++i ){
                        cJSON* jRank = cJSON_GetArrayItem(jRanks, i);
                        rank.idx = getJsonInt(jRank, "idx", error);
                        rank.userid = getJsonInt(jRank, "userid", error);
                        rank.gameid = getJsonInt(jRank, "gameid", error);
                        rank.row = getJsonInt(jRank, "row", error);
                        rank.rank = getJsonInt(jRank, "rank", error);
                        rank.score = getJsonInt(jRank, "score", error);
                        rank.nationality = getJsonInt(jRank, "nationality", error);
                        const char* date = getJsonString(jRank, "date", error);
                        const char* time = getJsonString(jRank, "time", error);
                        const char* username = getJsonString(jRank, "username", error);
                        if ( error == DMSERR_NONE ){
                            rank.date = date;
                            rank.time = time;
                            rank.username = username;
                            ranks.push_back(rank);
                        }
                    }
                }
            }
            onGetTimeline(error, ranks);
            cJSON_Delete(json);
        }
    };
    
}//namespace

void dmsInit(){
    lwassert(_pd==NULL);
    _pd = new Data;
    _pd->pHttpClient = new lw::HTTPClient("127.0.0.1:8000");
    _pd->pHttpClient->enableHTTPS(false);
    _pd->pCallback = NULL;
    _pd->dmsMain = [[DmsMain alloc] init];
    _pd->pLocalDB = new DmsLocalDB();
    
    lw::setHTTPErrorCallback(netErrorCallback);
}

void dmsDestroy(){
    lwassert(_pd);
    delete _pd->pHttpClient;
    [_pd->dmsMain release];
    delete _pd->pLocalDB;
    delete _pd;
    _pd = NULL;
}

void dmsSetCallback(DmsCallback* pCallback){
    lwassert(_pd);
    _pd->pCallback = pCallback;
}

void dmsLogin(const char* gcid, const char* username){
    lwassert(_pd);
    _pd->gcid.clear();
    _pd->gameStartToken.clear();
    lw::HTTPMsg* pMsg = new MsgLogin(gcid, username);
    pMsg->send();
}

void dmsLogout(){
    lwassert(_pd);
    _pd->gcid.clear();
    _pd->isLogin = false;
    _pd->gameStartToken.clear();
    lw::HTTPMsg* pMsg = new MsgLogout();
    pMsg->send();
}

void dmsPing(){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgPing();
    pMsg->send();
}

void dmsHeartBeat(){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgHeartBeat();
    pMsg->send();
}

void dmsGetTodayGames(){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgGetTodayGames();
    pMsg->send();
}

void dmsStartGame(int gameid){
    lwassert(_pd);
    _pd->gameStartToken.clear();
    lw::HTTPMsg* pMsg = new MsgStartGame(gameid);
    pMsg->send();
}

bool dmsSubmitScore(int gameid, int score){
    lwassert(_pd);
    if ( _pd->gameStartToken.empty() ){
        lwerror("dmsStartGame first");
        if ( _pd->pCallback ){
            _pd->pCallback->onError("dmsStartGame first");
        }
        return false;
    }
    lw::HTTPMsg* pMsg = new MsgSubmitScore(gameid, score);
    pMsg->send();
    return true;
}

void dmsGetUnread(){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgGetUnread();
    pMsg->send();
}

void dmsGetTimeline(int offset, int limit){
    lwassert(_pd);
    if ( offset < 0 || limit <=0 ){
        lwerror("param error");
        return;
    }
    lw::HTTPMsg* pMsg = new MsgGetTimeline(offset, limit);
    pMsg->send();
}

void onLogin(int error, const char* gcid, const char* datetime){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onLogin(error, gcid, datetime);
    }
}

void onLogout(){
    lwinfo("logout");
    if ( _pd->pCallback ){
        _pd->pCallback->onLogout();
    }
}

void onHeartBeat(int error){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onHeartBeat(error);
    }
}

void onGetTodayGames(int error, const std::vector<DmsGame>& games){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onGetTodayGames(error, games);
    }
}

void onStartGame(int error, const char* token, int gameid){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onStartGame(error, gameid);
    }
}

void onSubmitScore(int error, int gameid, int score){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onSubmitScore(error, gameid, score);
    }
}

void onGetUnread(int error, int unread, int topid){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onGetUnread(error, unread, topid);
    }
}

void onGetTimeline(int error, std::vector<DmsRank>& ranks){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onGetTimeline(error, ranks);
    }
}
