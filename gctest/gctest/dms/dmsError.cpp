#include "dmsError.h"

const char* getDmsErrorString(int error){
    if ( error == DMSERR_NONE ){
        return "DMSERR_NONE";
    }else if ( error == DMSERR_SERVERDATA ){
        return "DMSERR_SERVERDATA";
    }else if ( error == DMSERR_OTHER ){
        return "DMSERR_OTHER";
    }else if ( error == DMSERR_PARAM ){
        return "DMSERR_PARAM";
    }else if ( error == DMSERR_EXIST ){
        return "DMSERR_EXIST";
    }else if ( error == DMSERR_PASSWORD ){
        return "DMSERR_PASSWORD";
    }else if ( error == DMSERR_LOGIN ){
        return "DMSERR_LOGIN";
    }else if ( error == DMSERR_SQL ){
        return "DMSERR_SQL";
    }else if ( error == DMSERR_RANGE ){
        return "DMSERR_RANGE";
    }else if ( error == DMSERR_SECRET ){
        return "DMSERR_SECRET";
    }else{
        return "DMSERR_UNKNOWN";
    }
}