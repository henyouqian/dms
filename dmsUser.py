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
    if g.userid:
        return True
    return False

@userBluePrint.before_request
def before_request():
    g.userid = None
    if ( 'usertoken' in session ):
        token = session['usertoken']
        cachedata = g.mc.get(token)
        if cachedata:
            g.userid = cachedata[0]
            g.appid = cachedata[1]

@userBluePrint.route('/dmsapi/user/login')
def userlogin():
    gcid = request.args.get('gcid', type=unicode)
    appsecretkey = request.args.get('appsecretkey', type=unicode)
    username = request.args.get('username', type=unicode)
    if ( gcid==None or appsecretkey==None or username==None or username=='' ):
        return jsonify(error=DMSERR_PARAM)
    if userisLogin():
        token = session['usertoken']
        g.mc.delete(token)
    session.clear()

    row = g.db.get('SELECT user_id, name FROM account_db.Users WHERE gamecenter_id=%s', gcid)
    if (row == None):
        g.db.execute('INSERT INTO account_db.Users (gamecenter_id, name) VALUES(%s, %s)', gcid, username)
        row = g.db.get('SELECT user_id, name FROM account_db.Users WHERE gamecenter_id=%s', gcid)
    userid=row['user_id']
    usernameindb = row['name']
    if ( username != usernameindb ):
        g.db.execute('UPDATE account_db.Users SET name=%s WHERE gamecenter_id=%s', username, gcid)

    row = g.db.get('SELECT app_id FROM Apps WHERE secret_key=%s', appsecretkey)
    if ( row == None ):
        return jsonify(error=DMSERR_SECRET)
    appid = row['app_id']
    
    row = g.db.get('SELECT last_read FROM AppUserDatas WHERE user_id=%s AND app_id=%s', userid, appid)
    if ( row == None ):
        g.db.execute('INSERT INTO AppUserDatas (user_id, app_id, last_read, last_write) VALUES (%s, %s, %s, %s)', userid, appid, 0, 0)
    
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [userid, appid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)
    return jsonify(error=DMSERR_NONE, gcid=gcid, time=str(datetime.datetime.utcnow()))

@userBluePrint.route('/dmsapi/user/logout')
def userlogout():
    if userisLogin():
        token = session['usertoken']
        g.mc.delete(token)
    session.clear()
    return jsonify(error=DMSERR_NONE)

@userBluePrint.route('/dmsapi/user/heartbeat')
def userheartbeat():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    token = session['usertoken']
    g.mc.delete(token)
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [g.userid, g.appid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)
    return jsonify(error=DMSERR_NONE)

@userBluePrint.route('/dmsapi/user/gettodaygames')
def usergettodaygames():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    rows = g.db.iter('SELECT g.game_id, s.score, s.time FROM Games AS g INNER JOIN Scores AS s on g.game_id=s.game_id AND s.date=UTC_DATE() AND s.user_id=%s AND g.app_id=%s', g.userid, g.appid)
    games = [{'gameid':row['game_id'], 'score':row['score'], 'time':str(row['time'])} for row in rows]
    return jsonify(error=DMSERR_NONE, games=games)

@userBluePrint.route('/dmsapi/user/startgame')
def userstartgame():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    gameid = request.args.get('gameid', type=int)
    if ( gameid==None ):
        return jsonify(error=DMSERR_PARAM)
    row = g.db.get('SELECT game_id, app_id FROM Games WHERE game_id=%s', gameid)
    if row == None:
        return jsonify(error=DMSERR_NOTMATCH)
    if row['app_id'] != g.appid:
        return jsonify(error=DMSERR_APPID)
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

@userBluePrint.route('/dmsapi/user/hasunread')
def hasunread():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    row = g.db.get('SELECT last_read, last_write FROM AppUserDatas WHERE user_id=%s AND app_id=%s', g.userid, g.appid)
    if row == None:
        return jsonify(error=DMSERR_SQL)
    hasUnread = True;
    if row['last_read'] == row['last_write']:
        hasUnread = False;
    return jsonify(error=DMSERR_NONE, hasunread=hasUnread)

@userBluePrint.route('/dmsapi/user/getunread')
def getunread():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    row = g.db.get('SELECT last_read, last_write FROM AppUserDatas WHERE user_id=%s AND app_id=%s', g.userid, g.appid)
    if row == None:
        return jsonify(error=DMSERR_SQL)
    lastread = row['last_read']
    lastwrite = row['last_write']
    return jsonify(error=DMSERR_NONE, unread=lastwrite-lastread)

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
    


