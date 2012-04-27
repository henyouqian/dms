#include "dms.h"
#include "dmsError.h"

void onLogin(int error, const char* gcid){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }else{
        lwinfo("login:" << gcid);
    }
}

void onLogout(){
    lwinfo("logout");
}

void onHeartBeat(int error){
    if ( error ){
        lwerror(getDmsErrorString(error));
    }else{
        lwinfo("heart beat");
    }
}
