#include "dmsLocalDB.h"
#include "sqlite3.h"
#include "lwFileSys.h"
#include <stdio.h>

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
                if ( version && strcmp(version, "1.0")==0 ){
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
}

DmsLocalDB::~DmsLocalDB(){
    sqlite3_close(_db);
}