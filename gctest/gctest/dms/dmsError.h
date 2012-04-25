#ifndef __DMS_ERROR_H__
#define __DMS_ERROR_H__

#define DMSERR_NONE         0
#define DMSERR_SERVERDATA   100
#define DMSERR_OTHER        1000
#define DMSERR_PARAM        1001
#define DMSERR_EXIST        1002
#define DMSERR_PASSWORD     1003
#define DMSERR_LOGIN        1004
#define DMSERR_SQL          1005
#define DMSERR_RANGE        1006
#define DMSERR_SECRET       1007


const char* getDmsErrorString(int error);

#endif //__DMS_ERROR_H__