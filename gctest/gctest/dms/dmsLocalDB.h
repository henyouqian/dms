#ifndef __DMS_LOCALDB_H__
#define __DMS_LOCALDB_H__

struct sqlite3;

class DmsLocalDB{
public:
    DmsLocalDB();
    ~DmsLocalDB();
    
private:
    sqlite3* _db;
};

#endif //__DMS_LOCALDB_H__