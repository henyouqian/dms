from flask import *
from tornado.database import Connection
import pylibmc
import hashlib
import uuid
from dmsError import *
import dmsConfig
import datetime

HEART_BEET_SECOND = 600
CACHE_KEEP_SECOND = HEART_BEET_SECOND + 60
GAME_KEEP_SECOND = 600+20

userBluePrint = Blueprint('user', __name__)

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
            g.appid = cachedata[2]

@userBluePrint.route('/dmsapi/user/heartbeat')
def userheartbeat():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    token = session['usertoken']
    g.mc.delete(token)
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [g.userid, g.developerid, g.appid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)

    row = g.db.get('SELECT unread FROM UserDatas WHERE user_id=%s', g.userid)
    unread = row['unread']
    return jsonify(error=DMSERR_NONE, unread=unread)

@userBluePrint.route('/dmsapi/user/login')
def userlogin():
    gcid = request.args.get('gcid', type=unicode)
    secretkey = request.args.get('secretkey', type=unicode)
    username = request.args.get('username', type=unicode)
    appid = request.args.get('appid', type=int)
    if ( gcid==None or secretkey==None or username==None or username=='' or appid==None ):
        return jsonify(error=DMSERR_PARAM)
    if userisLogin():
        token = session['usertoken']
        g.mc.delete(token)
    session.clear()

    row = g.db.get('SELECT user_id, name FROM account_db.Users WHERE gamecenter_id=%s', gcid)
    if (row == None):
        g.db.execute('INSERT INTO account_db.Users (gamecenter_id, name) VALUES(%s, %s)', gcid, username)
        row = g.db.get('SELECT user_id, name FROM account_db.Users WHERE gamecenter_id=%s', gcid)
        g.db.execute('INSERT INTO UserDatas (user_id, unread_from, cash) VALUES(%s, 0, 1), 0)', row['user_id'])
    userid=row['user_id']
    usernameindb = row['name']
    if ( username != usernameindb ):
        g.db.execute('UPDATE account_db.Users SET name=%s WHERE gamecenter_id=%s', username, gcid)

    row = g.db.get('SELECT developer_id FROM account_db.Developers WHERE secret_key=%s', secretkey)
    if ( row == None ):
        return jsonify(error=DMSERR_SECRET)
    developerid = row['developer_id']
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [userid, developerid, appid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)
    return jsonify(error=DMSERR_NONE, gcid=gcid, time=str(datetime.datetime.utcnow()))

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
    rows = g.db.iter('SELECT g.game_id, s.score, s.time FROM Games AS g INNER JOIN Scores AS s on g.game_id=s.game_id AND s.date=UTC_DATE() AND s.user_id=%s AND g.app_id=%s AND g.developer_id=%s', g.userid, g.appid, g.developerid)
    games = [{'gameid':row['game_id'], 'score':row['score'], 'time':str(row['time'])} for row in rows]
    return jsonify(error=DMSERR_NONE, games=games)

@userBluePrint.route('/dmsapi/user/startgame')
def userstartgame():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    gameid = request.args.get('gameid', type=int)
    if ( gameid==None ):
        return jsonify(error=DMSERR_PARAM)
    row = g.db.get('SELECT game_id FROM Games WHERE game_id=%s AND developer_id=%s', gameid, g.developerid)
    if row == None:
        return jsonify(error=DMSERR_NOTMATCH)
    row = g.db.get('SELECT score FROM Scores WHERE user_id=%s AND game_id=%s AND date=UTC_DATE()', g.userid, gameid)
    if row == None:
        g.db.execute('INSERT INTO Scores (user_id, game_id, date, time, score) VALUES (%s, %s, UTC_DATE(), UTC_TIME(), 0)', g.userid, gameid)
        key = uuid.uuid4().hex
        value = gameid
        g.mc.set(key, value, GAME_KEEP_SECOND)
        return jsonify(error=DMSERR_NONE, token=key)
    else:
        return jsonify(error=DMSERR_EXIST)
    
@userBluePrint.route('/dmsapi/user/submitscore')
def usersubmitscore():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    token = request.args.get('token', type=unicode)
    gameid = request.args.get('gameid', type=int)
    score = request.args.get('score', type=int)
    if ( token==None or token =='' or gameid==None or score==None ):
        return jsonify(error=DMSERR_PARAM)
    token = token.encode('utf8')
    mcgameid = g.mc.get(token)
    if mcgameid == None:
        return jsonify(error=DMSERR_EXIST)
    else:
        g.mc.delete(token);
        if gameid != mcgameid:
            return jsonify(error=DMSERR_NOTMATCH)
        else:
            g.db.execute('REPLACE INTO Scores (user_id, game_id, date, time, score) values(%s, %s, UTC_DATE(), UTC_TIME(), %s)', g.userid, gameid, score)
            return jsonify(error=DMSERR_NONE, gameid=gameid, score=score)

@userBluePrint.route('/dmsapi/user/getunread')
def getunread():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    row = g.db.get('SELECT unread FROM UserDatas WHERE user_id=%s', g.userid)
    if row:
        return jsonify(error=DMSERR_NONE, num=row['unread'])
    else:
        return jsonify(error=DMSERR_SQL)

@userBluePrint.route('/dmsapi/user/gettimeline')
def gettimeline():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    offset = request.args.get('offset', type=int)
    if ( offset==None ):
        return jsonify(error=DMSERR_PARAM)
    rows = g.db.iter('''SELECT user_id, game_id, date, row, score, time, user_name, nationality FROM Ranks
                        WHERE user_id=%s AND ''')
    
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


