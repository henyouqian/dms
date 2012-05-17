#include "dms.h"
#include "dmsError.h"
#include "cJSON.h"
#include "sqlite3.h"
#include "lwFileSys.h"

void onLogin(int error, const char* gcid, const char* datetime);
void onLogout();
void onHeartBeat(int error);
void onGetTodayGames(int error, const std::vector<DmsGame>& games);
void onStartGame(int error, const char* token, int gameid);
void onSubmitScore(int error, int gameid, int score);
void onGetUnread(int error, int unread);

Rank::Rank(int _row, int _rank, int _score, const char* _userName)
:row(_row), rank(_rank), score(_score){
    userName = _userName?_userName:"";
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
    ++_tHeartBeat;
    if ( _tHeartBeat >= HEART_BEAT_SECOND ){
        dmsHeartBeat();
        _tHeartBeat = 0;
    }
}

@end

namespace {
    
    struct Data{
        Data():pHttpClient(NULL), isLogin(false){}
        std::string gcid;
        bool isLogin;
        std::vector<Rank> ranks;
        lw::HTTPClient* pHttpClient;
        DmsCallback* pCallback;
        std::string gameStartToken;
        DmsMain* dmsMain;
        sqlite3* db;
    };
    
    Data* _pd = NULL;
    
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
            cJSON *json=parseMsg(_buff.c_str(), error);
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
            cJSON *json=parseMsg(_buff.c_str(), error);
            if ( error == DMSERR_NONE ){
                unread = getJsonInt(json, "unread", error);
            }
            onGetUnread(error, unread);
            cJSON_Delete(json);
        }
    };
    
    class MsgGetTimeline : public lw::HTTPMsg{
    public:
        MsgGetTimeline(int offset)
        :lw::HTTPMsg("/dmsapi/user/gettimeline", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?offset=" << offset;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            
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
    
    std::string docDir = lw::getDocDir();
    docDir += "/dms.sqlite";
	FILE* pf = fopen(docDir.c_str(), "rb");
	if ( pf == NULL ){
		pf = fopen(_f("dms.sqlite"), "rb");
		lwassert(pf);
		fseek(pf, 0, SEEK_END);
		int len = ftell(pf);
		char* buf = new char[len];
		fseek(pf, 0, SEEK_SET);
		fread(buf, len, 1, pf);
		fclose(pf);
		FILE* pOut = fopen(docDir.c_str(), "wb");
		lwassert(pOut);
		fwrite(buf, len, 1, pOut);
		fclose(pOut);
        delete [] buf;
	}
    
	int r = sqlite3_open(docDir.c_str(), &(_pd->db));
	lwassert(r == SQLITE_OK);
}

void dmsDestroy(){
    lwassert(_pd);
    delete _pd->pHttpClient;
    [_pd->dmsMain release];
    sqlite3_close(_pd->db);
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

void dmsGetTimeline(int offset){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgGetTimeline(offset);
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

void onGetUnread(int error, int unread){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onGetUnread(error, unread);
    }
}
