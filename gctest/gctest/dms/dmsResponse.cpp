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

void onGetTodayMatches(const char* error, const std::vector<Match>& matches, int matchNum){
    if ( error ){
        lwerror(error);
    }else{
        lwinfo("today matches:" << matchNum);
        std::vector<Match>::const_iterator it = matches.begin();
        std::vector<Match>::const_iterator itEnd = matches.end();
        for ( ; it != itEnd; ++it ){
            lwinfo(it->matchName << " " << it->gameName << " " << it->date );
        }
    }
}

void onGetCommingMatches(const char* error, const std::vector<Match>& matches, int matchNum){
    if ( error ){
        lwerror(error);
    }else{
        lwinfo("comming matches:" << matchNum);
        std::vector<Match>::const_iterator it = matches.begin();
        std::vector<Match>::const_iterator itEnd = matches.end();
        for ( ; it != itEnd; ++it ){
            lwinfo(it->matchName << " " << it->gameName << " " << it->date );
        }
    }
}

void onGetClosedMatches(const char* error, const std::vector<Match>& matches, int matchNum){
    if ( error ){
        lwerror(error);
    }else{
        lwinfo("closed matches:" << matchNum);
        std::vector<Match>::const_iterator it = matches.begin();
        std::vector<Match>::const_iterator itEnd = matches.end();
        for ( ; it != itEnd; ++it ){
            lwinfo(it->matchName << " " << it->gameName << " " << it->date );
        }
    }
}

void onSubmitScore(const char* error, int matchid, int score){
    if ( error ){
        lwerror(error);
    }else{
        lwinfo("score submit:" << matchid << "|" << score);
    }
}

void onGetRanks(const char* error, const std::vector<Rank>& ranks, int matchId, int offset){
    if ( error ){
        lwerror(error);
    }else{
        lwinfo("ranks:");
        std::vector<Rank>::const_iterator it = ranks.begin();
        std::vector<Rank>::const_iterator itEnd = ranks.end();
        for ( ; it != itEnd; ++it ){
            lwinfo(it->userName << "|" << it->row << "|" << it->rank<< "|" << it->score );
        }
    }
}

