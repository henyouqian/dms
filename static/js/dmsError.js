DMSERR_NONE     = 0
DMSERR_OTHER    = 1000
DMSERR_PARAM    = 1001
DMSERR_EXIST    = 1002
DMSERR_PASSWORD = 1003
DMSERR_LOGIN    = 1004
DMSERR_SQL      = 1005
DMSERR_RANGE    = 1006
DMSERR_SECRET   = 1007
DMSERR_NOTMATCH = 1008
DMSERR_APPID    = 1009

function errorProc(err){
    if (err==0){
        return;
    }else if (err==DMSERR_OTHER){
        alert('DMSERR_OTHER');
    }else if (err==DMSERR_PARAM){
        alert('DMSERR_PARAM');
    }else if (err==DMSERR_EXIST){
        alert('DMSERR_EXIST');
    }else if (err==DMSERR_PASSWORD){
        alert('DMSERR_PASSWORD');
    }else if (err==DMSERR_LOGIN){
        alert('DMSERR_LOGIN');
        window.location.href='/dms/dev';
    }else if (err==DMSERR_SQL){
        alert('DMSERR_SQL');
    }else if (err==DMSERR_RANGE){
        alert('DMSERR_RANGE');
    }else if (err==DMSERR_SECRET){
        alert('DMSERR_SECRET');
    }else if (err==DMSERR_NOTMATCH){
        alert('DMSERR_NOTMATCH');
    }else if (err==DMSERR_APPID){
        alert('DMSERR_APPID');
    }else{
        alert('unknown error');
    }
}