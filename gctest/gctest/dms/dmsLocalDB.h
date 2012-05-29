#ifndef __DMS_LOCALDB_H__
#define __DMS_LOCALDB_H__

struct sqlite3;
struct DmsRank;

class DmsLocalDB{
public:
    DmsLocalDB();
    ~DmsLocalDB();
    void addTimeline(const std::vector<DmsRank>& ranks);
    void getTimeline(std::vector<DmsRank>& ranks, int offset, int limit);
    void setToprankidUnread(int topRankId, int unread);
    int getTopRankId();
    int getUnread();
    void setUserid(int userid);
    int getUserid();
    void setGcid(const char* gcid);
    const char* getGcid();
    
private:
    bool setKVInt(const char* k, int v);
    bool setKVString(const char* k, const char* v);
    bool getKVInt(const char* k, int &v, int defaultV = 0);
    bool getKVString(const char* k, std::string& str, const char* defaultStr = "");
    
    sqlite3* _db;
    int _topRankId;
    int _unread;
    int _userid;
    std::string _gcid;
};

#endif //__DMS_LOCALDB_H__