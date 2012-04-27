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
        std::vector<int> gameIds;
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

void dmsAddGame(int gameid){
    lwassert(_pd);
    _pd->gameIds.push_back(gameid);
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
    
}

void dmsStartGame(int gameid){
    
}

void dmsSubmitScore(int gameid, int score){
    
}

void dmsGetTimeline(int offset){
    
}
