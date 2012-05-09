#include "dms.h"
#include "dmsError.h"
#include "cJSON.h"

Rank::Rank(int _row, int _rank, int _score, const char* _userName)
:row(_row), rank(_rank), score(_score){
    userName = _userName?_userName:"";
}

namespace {
    
    struct Data{
        Data():pHttpClient(NULL), isLogin(false){}
        std::string gcid;
        bool isLogin;
        std::vector<Rank> ranks;
        lw::HTTPClient* pHttpClient;
        int appId;
        DmsCallback* pCallback;
        std::string gameStartToken;
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
            ss << "?gcid=" << gcid << "&secretkey=" << SECRET_KEY << "&username=" << username;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            int error = DMSERR_NONE;
            cJSON *json=parseMsg(_buff.c_str(), error);
            const char* gcid = NULL;
            const char* datetime = NULL;
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
            std::stringstream ss;
            ss << "?gcid=[2, 4, 654]";
            addParam(ss.str().c_str());
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
            std::stringstream ss;
            ss << "?appid=" << _pd->appId;
            addParam(ss.str().c_str());
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
            cJSON *json=parseMsg(_buff.c_str(), error);
            const char* token = NULL;
            if ( error == DMSERR_NONE ){
                token = getJsonString(json, "token", error);
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
            int error = DMSERR_NONE;
            cJSON *json=parseMsg(_buff.c_str(), error);
            int gameid = -1;
            int score = 0;
            if ( error == DMSERR_NONE ){
                gameid = getJsonInt(json, "gameid", error);
                score = getJsonInt(json, "score", error);
            }
            onSubmitScore(error, gameid, score);
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

void dmsInit(int appid){
    lwassert(_pd==NULL);
    _pd = new Data;
    _pd->pHttpClient = new lw::HTTPClient("127.0.0.1:8000");
    _pd->pHttpClient->enableHTTPS(false);
    _pd->appId = appid;
    _pd->pCallback = NULL;
}

void dmsDestroy(){
    lwassert(_pd);
    delete _pd->pHttpClient;
    delete _pd;
    _pd = NULL;
}

void dmsMain(){
    
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

void dmsGetTimeline(int offset){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgGetTimeline(offset);
    pMsg->send();
}

void onLogin(int error, const char* gcid, const char* datetime){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }else{
        lwinfo("login:" << gcid);
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
    }else{
        lwinfo("heart beat");
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onHeartBeat(error);
    }
}

void onGetTodayGames(int error, const std::vector<DmsGame>& games){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }else{
        
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onGetTodayGames(error, games);
    }
}

void onStartGame(int error, const char* token, int gameid){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }else{
        _pd->gameStartToken = token;
        lwinfo("start game:" << token);
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onStartGame(error, gameid);
    }
}

void onSubmitScore(int error, int gameid, int score){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }else{
        lwinfo("submit score: gameid=" << gameid << " score=" << score);
    }
    if ( _pd->pCallback ){
        _pd->pCallback->onSubmitScore(error, gameid, score);
    }
}
