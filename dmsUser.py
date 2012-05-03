from flask import *
from tornado.database import Connection
import pylibmc
import hashlib
import uuid
from dmsError import *
import dmsConfig

HEART_BEET_SECOND = 600
CACHE_KEEP_SECOND = HEART_BEET_SECOND + 60

userBluePrint = Blueprint('user', __name__)

def connectAccountDb():
    return Connection(dmsConfig.host,
                      dmsConfig.account_db,
                      dmsConfig.user,
                      dmsConfig.passwd)

def userisLogin():
    if g.userid and g.developerid:
        return True
    return False

@userBluePrint.before_request
def before_request():
    g.userid = None
    g.developerid = None
    if ( 'usertoken' in session ):
        token = session['usertoken']
        cachedata = g.mc.get(token)
        if cachedata:
            g.userid = cachedata[0]
            g.developerid = cachedata[1]

@userBluePrint.route('/dmsapi/user/heartbeat')
def userheartbeat():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    token = session['usertoken']
    g.mc.delete(token)
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [g.userid, g.developerid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)
    return jsonify(error=DMSERR_NONE)

@userBluePrint.route('/dmsapi/user/login')
def userlogin():
    gcid = request.args.get('gcid', type=unicode)
    secretkey = request.args.get('secretkey', type=unicode)
    if ( gcid==None or secretkey==None ):
        return jsonify(error=DMSERR_PARAM)
    if userisLogin():
        token = session['usertoken']
        g.mc.delete(token)
    session.clear()
    accountdb = connectAccountDb()
    row = accountdb.get('SELECT user_id FROM Users WHERE gamecenter_id=%s', gcid)
    if (row == None):
        accountdb.execute('INSERT INTO Users (gamecenter_id) VALUES(%s)', gcid)
        row = accountdb.get('SELECT user_id FROM Users WHERE gamecenter_id=%s', gcid)
    userid=row['user_id']
    row = accountdb.get('SELECT developer_id FROM Developers WHERE secret_key=%s', secretkey)
    if ( row == None ):
        return jsonify(error=DMSERR_SECRET)
    developerid = row['developer_id']
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [userid, developerid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)
    return jsonify(error=DMSERR_NONE, gcid=gcid)

@userBluePrint.route('/dmsapi/user/logout')
def userlogout():
    if userisLogin():
        token = session['usertoken']
        g.mc.delete(token)
    session.clear()
    return jsonify(error=DMSERR_NONE)


@userBluePrint.route('/dmsapi/user/gettodaygames')
def usergettodaygames():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    appid = request.args.get('appid', type=int)
    rows = g.db.iter('SELECT g.game_id, s.score, s.time FROM Games AS g INNER JOIN Scores AS s on g.game_id=s.game_id  WHERE g.app_id=%s AND g.developer_id=%s AND s.date=UTC_DATE() AND s.user_id=%s', appid, g.developerid, g.userid)
    print 'xxxxxxxxxxxxxxxxxxx'
    for row in rows:
        print row;
    print 'yyyyyyyyyyyyyyyyyyy'
    return jsonify(error=DMSERR_NONE)

    
###match
@userBluePrint.route('/dmsapi/user/gettodaymatches')
def usergettodaymatches():
    if not userisLogin():
        return jsonify(error=DMSERR_LOGIN)
    offset = request.args.get('offset', type=int)
    limit = request.args.get('limit', type=int)
    if ( offset==None or limit==None ):
        return jsonify(error=DMSERR_PARAM)
    limit = max(1, min(50, limit))
    data = {'matchnum':0, 'matches':[]}
    rows = g.db.iter('SELECT m.match_id, m.name, m.date, m.game_id, g.name, s.score, s.time, m.score_sum, m.score_num FROM Matches AS m INNER JOIN Games AS g ON m.game_id=g.game_id LEFT JOIN Scores AS s ON m.match_id=s.match_id AND s.user_id=%s WHERE m.developer_id=%s AND m.date=UTC_DATE() LIMIT %s OFFSET %s', g.userid, g.developerid, limit, offset)
    for row in rows:
        time=None
        if row['s.time']:
            time=str(row['s.time'])
        scoreavg=0
        if row['m.score_num']:
            scoreavg=row['m.score_sum']/float(row['m.score_num'])
        data['matches'].append({'matchid':row['m.match_id'], 'matchname':row['m.name'], 'date':str(row['m.date']), 'gameid':row['m.game_id'], 'gamename':row['g.name'], 'score':row['s.score'], 'time':time, 'scoreavg':scoreavg, 'scorenum':row['m.score_num']});
    row = g.db.get('SELECT COUNT(*) FROM Matches WHERE developer_id=%s AND date=UTC_DATE()', g.developerid)
    if row:
        data['matchnum']=row['COUNT(*)']
    return jsonify(error=DMSERR_NONE, data=data);


