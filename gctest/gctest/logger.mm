#include "logger.h"
#include "dms.h"
#include "dmsError.h"
#include "lwUtil.h"

Logger::Logger(UITextView* pTextView):_pTextView(pTextView){
    _str = [[NSMutableString alloc] init];
    lw::srand();
}

Logger::~Logger(){
    [_str release];
}

void Logger::addLog(const char* log){
    std::string str = "+";
    str.append(log);
    str.append("\n");
    [_str insertString:[NSString stringWithUTF8String:str.c_str()] atIndex:0];
    _pTextView.text = _str;
}

void Logger::onError(const char* text){
    std::string str = "onError:";
    str.append(text);
    addLog(str.c_str());
}

void Logger::onLogin(int error, const char* gcid, const char* datetime){
    std::stringstream ss;
    if ( error ){
        ss << "onLogin:error=" << getDmsErrorString(error);
    }else{
        ss << "onLogin:gcid=" << gcid << ", datatime=" << datetime;
    }
    addLog(ss.str().c_str());
}

void Logger::onLogout(){
    addLog("onLogout");
}

void Logger::onHeartBeat(int error){
    std::stringstream ss;
    if ( error ){
        ss << "onLogin:error=" << getDmsErrorString(error);
    }else{
        ss << "onHeartBeat";
    }
    addLog(ss.str().c_str());
}

void Logger::onGetTodayGames(int error, const std::vector<DmsGame>& games){
    std::stringstream ss;
    if ( error ){
        ss << "onGetTodayGames:error=" << getDmsErrorString(error);
    }else{
        ss << "onGetTodayGames:\n";
        std::vector<DmsGame>::const_iterator it = games.begin();
        std::vector<DmsGame>::const_iterator itend = games.end();
        for ( int i = 1; it != itend; ++it, ++i ){
            ss << "  " << i << ": id=" << it->gameid << ", score=" << it->score << "\n";
        }
    }
    addLog(ss.str().c_str());
}

void Logger::onStartGame(int error, int gameid){
    std::stringstream ss;
    if ( error ){
        ss << "onStartGame:error=" << getDmsErrorString(error);
    }else{
        ss << "onStartGame:gameid=" << gameid;
    }
    addLog(ss.str().c_str());
}

void Logger::onSubmitScore(int error, int gameid, int score){
    std::stringstream ss;
    if ( error ){
        ss << "onSubmitScore:error=" << getDmsErrorString(error);
    }else{
        ss << "onSubmitScore:gameid=" << gameid << ",score=" << score;
    }
    addLog(ss.str().c_str());
}

