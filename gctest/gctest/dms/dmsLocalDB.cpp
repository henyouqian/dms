#include "dmsLocalDB.h"
#include "sqlite3.h"
#include "lwFileSys.h"
#include "dms.h"
#include <stdio.h>

#define DMS_VERSION "1.0"
#define TOP_RANK_ID "top_rank_id"
#define UNREAD "unread"
#define USER_ID "user_id"
#define GC_ID "gc_id"

void copyDB(const char* targetPath){
    FILE* pf = fopen(_f("dms.sqlite"), "rb");
    lwassert(pf);
    fseek(pf, 0, SEEK_END);
    int len = ftell(pf);
    char* buf = new char[len];
    fseek(pf, 0, SEEK_SET);
    fread(buf, len, 1, pf);
    fclose(pf);
    FILE* pOut = fopen(targetPath, "wb");
    lwassert(pOut);
    fwrite(buf, len, 1, pOut);
    fclose(pOut);
    delete [] buf;
}

DmsLocalDB::DmsLocalDB(){
    std::string targetPath = lw::getDocDir();
    targetPath += "/dms.sqlite";
	FILE* pf = fopen(targetPath.c_str(), "rb");
    bool needCopy = false;
	if ( pf == NULL ){
		needCopy = true;
	}else{
        int r = sqlite3_open(targetPath.c_str(), &_db);
        lwassert(r == SQLITE_OK);
        sqlite3_stmt* pStmt = NULL;
        std::stringstream ss;
        ss << "SELECT value FROM KVDatas WHERE key='version'";
        r = sqlite3_prepare_v2(_db, ss.str().c_str(), -1, &pStmt, NULL);
        lwassert(r == SQLITE_OK);
        while ( 1 ){
            r = sqlite3_step(pStmt);
            if ( r == SQLITE_ROW ){
                const char* version = (const char*)sqlite3_column_text(pStmt, 0);
                if ( version && strcmp(version, DMS_VERSION)==0 ){
                    needCopy = false;
                }else{
                    needCopy = true;
                }
                break;
            }else if ( r == SQLITE_DONE ){
                break;
            }else{
                break;
            }
        }
        sqlite3_finalize(pStmt);
        fclose(pf);
    }
    if ( needCopy ){
        copyDB(targetPath.c_str());
        int r = sqlite3_open(targetPath.c_str(), &_db);
        lwassert(r == SQLITE_OK);
    }
    
    getKVInt(TOP_RANK_ID, _topRankId, 0);
    getKVInt(UNREAD, _unread, 0);
    getKVInt(USER_ID, _userid, 0);
    getKVString(GC_ID, _gcid, "");
}

DmsLocalDB::~DmsLocalDB(){
    sqlite3_close(_db);
}

bool DmsLocalDB::setKVInt(const char* k, int v){
    lwassert(k);
    std::stringstream ss;
    ss << "REPLACE INTO KVDatas (key, value) values('" << k << "', " << v << ");";
    int r = sqlite3_exec(_db, ss.str().c_str(), NULL, NULL, NULL);
	if ( r != SQLITE_OK ){
        lwerror(r);
        return false;
    }
    return true;
}

bool DmsLocalDB::setKVString(const char* k, const char* v){
    lwassert(k && v);
    std::stringstream ss;
    ss << "REPLACE INTO KVDatas (key, value) values('" << k << "', '" << v << "');";
    int r = sqlite3_exec(_db, ss.str().c_str(), NULL, NULL, NULL);
	if ( r != SQLITE_OK ){
        lwerror(r);
        return false;
    }
    return true;
}

bool DmsLocalDB::getKVInt(const char* k, int &v, int defaultV){
    v = defaultV;
    sqlite3_stmt* pStmt = NULL;
    std::stringstream ss;
    ss << "SELECT value FROM KVDatas WHERE key='" << k << "';";
    int r = sqlite3_prepare_v2(_db, ss.str().c_str(), -1, &pStmt, NULL);
    lwassert(r == SQLITE_OK);
    bool b = false;
    while ( 1 ){
        r = sqlite3_step(pStmt);
        if ( r == SQLITE_ROW ){
            v = sqlite3_column_int(pStmt, 0);
            b = true;
            break;
        }else if ( r == SQLITE_DONE ){
            break;
        }else{
            break;
        }
    }
    sqlite3_finalize(pStmt);
    return b;
}

bool DmsLocalDB::getKVString(const char* k, std::string& str, const char* defaultStr){
    str = defaultStr;
    sqlite3_stmt* pStmt = NULL;
    std::stringstream ss;
    ss << "SELECT value FROM KVDatas WHERE key='" << k << "';";
    int r = sqlite3_prepare_v2(_db, ss.str().c_str(), -1, &pStmt, NULL);
    lwassert(r == SQLITE_OK);
    bool b = false;
    while ( 1 ){
        r = sqlite3_step(pStmt);
        if ( r == SQLITE_ROW ){
            str = (const char*)sqlite3_column_text(pStmt, 0);
            b = true;
            break;
        }else if ( r == SQLITE_DONE ){
            break;
        }else{
            break;
        }
    }
    sqlite3_finalize(pStmt);
    return b;
}

void DmsLocalDB::addTimeline(const std::vector<DmsRank>& ranks){
    std::stringstream ss;
	ss << "BEGIN TRANSACTION;";
    std::vector<DmsRank>::const_iterator it = ranks.begin();
    std::vector<DmsRank>::const_iterator ite = ranks.end();
    for ( ; it != ite; ++it ){
        ss << "REPLACE INTO Ranks (user_id, game_id, date, time, row, rank, score, user_name, nationality, idx_app_user) VALUES ("
            << it->userid << ", "
            << it->gameid << ", '"
            << it->date << "', '"
            << it->time << "', "
            << it->row << ", "
            << it->rank << ", "
            << it->score << ", '"
            << it->username << "', "
            << it->nationality << ", "
            << it->idx
            <<");";
    }
    ss << "COMMIT;";
	int r = sqlite3_exec(_db, ss.str().c_str(), NULL, NULL, NULL);
	if ( r != SQLITE_OK ){
        lwerror("sqlerror:r=" << r << ", sql=" << ss.str().c_str());
    }
}

void DmsLocalDB::getTimeline(std::vector<DmsRank>& ranks, int offset, int limit){
    sqlite3_stmt* pStmt = NULL;
    std::stringstream ss;
    ss << "SELECT user_id, game_id, date, time, row, rank, score, user_name, nationality, idx_app_user FROM Ranks WHERE user_id=" << _userid << " AND idx_app_user <= " << _topRankId-offset << " ORDER BY idx_app_user DESC LIMIT " << limit << ";" ;
    int r = sqlite3_prepare_v2(_db, ss.str().c_str(), -1, &pStmt, NULL);
    
    if ( r != SQLITE_OK ){
        lwerror("sqlerror:" << ss.str().c_str());
        return;
    }
    while ( 1 ){
        r = sqlite3_step(pStmt);
        if ( r == SQLITE_ROW ){
            DmsRank rank;
            rank.idx = 
            rank.userid = sqlite3_column_int(pStmt, 0);
            rank.gameid = sqlite3_column_int(pStmt, 1);
            rank.date = (const char*)sqlite3_column_text(pStmt, 2);
            rank.time = (const char*)sqlite3_column_text(pStmt, 3);
            rank.row = sqlite3_column_int(pStmt, 4);
            rank.rank = sqlite3_column_int(pStmt, 5);
            rank.score = sqlite3_column_int(pStmt, 6);
            rank.username = (const char*)sqlite3_column_text(pStmt, 7);
            rank.nationality = sqlite3_column_int(pStmt, 8);
            rank.idx = sqlite3_column_int(pStmt, 9);
            ranks.push_back(rank);
        }else if ( r == SQLITE_DONE ){
            break;
        }else{
            break;
        }
    }
    sqlite3_finalize(pStmt);
}

void DmsLocalDB::setToprankidUnread(int topRankId, int unread){
    std::stringstream ss;
    if ( topRankId != _topRankId ){
        _topRankId = topRankId;
        setKVInt(TOP_RANK_ID, topRankId);
    }
    if ( unread != _unread ){
        _unread = unread;
        setKVInt(UNREAD, unread);
    }
}

int DmsLocalDB::getTopRankId(){
    return _topRankId;
}

int DmsLocalDB::getUnread(){
    return _unread;
}

void DmsLocalDB::setUserid(int userid){
    if ( _userid != userid ){
        _userid = userid;
        setKVInt(USER_ID, userid);
    }
}

int DmsLocalDB::getUserid(){
    return _userid;
}

void DmsLocalDB::setGcid(const char* gcid){
    if ( _gcid.compare(gcid) != 0 ){
        _gcid = gcid;
        setKVString(GC_ID, gcid);
    }
}

const char* DmsLocalDB::getGcid(){
    return _gcid.c_str();
}