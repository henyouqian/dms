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
    
    class MsgGetTodayGames : public lw::HTTPMsg{
    public:
        MsgGetTodayGames()
        :lw::HTTPMsg("/dmsapi/user/gettodaygames", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?appid=" << _pd->appId;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            
        }
    };
    
    class MsgStartGame : public lw::HTTPMsg{
    public:
        MsgStartGame(int gameid)
        :lw::HTTPMsg("/dmsapi/user/startgame", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?gameid=" << gameid;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            
        }
    };
    
    class MsgSubmitScore : public lw::HTTPMsg{
    public:
        MsgSubmitScore(int gameid, int score)
        :lw::HTTPMsg("/dmsapi/user/submitscore", _pd->pHttpClient, false){
            std::stringstream ss;
            ss << "?gameid=" << gameid << "&score=" << score;
            addParam(ss.str().c_str());
        }
        virtual void onRespond(){
            
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
}

void dmsDestroy(){
    lwassert(_pd);
    delete _pd->pHttpClient;
    delete _pd;
    _pd = NULL;
}

void dmsMain(){
    
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

void dmsSubmitScore(int gameid, int score){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgSubmitScore(gameid, score);
    pMsg->send();
}

void dmsGetTimeline(int offset){
    lwassert(_pd);
    lw::HTTPMsg* pMsg = new MsgGetTimeline(offset);
    pMsg->send();
}
