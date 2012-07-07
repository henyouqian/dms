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
TIMELINE_LIMIT = 50
GETRANK_LIMIT = 50

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
        g.db.execute('INSERT INTO account_db.Users (gamecenter_id, name, nationality) VALUES(%s, %s, 0)', gcid, username)
        row = g.db.get('SELECT user_id, name FROM account_db.Users WHERE gamecenter_id=%s', gcid)
    userid=row['user_id']
    usernameindb = row['name']
    if ( username != usernameindb ):
        g.db.execute('UPDATE account_db.Users SET name=%s WHERE gamecenter_id=%s', username, gcid)

    row = g.db.get('SELECT app_id FROM Apps WHERE secret_key=%s', appsecretkey)
    if ( row == None ):
        return jsonify(error=DMSERR_SECRET)
    appid = row['app_id']
    
    lastread = 0
    lastwrite = 0
    row = g.db.get('SELECT last_read, last_write FROM AppUserDatas WHERE user_id=%s AND app_id=%s', userid, appid)
    if ( row == None ):
        g.db.execute('INSERT INTO AppUserDatas (user_id, app_id, last_read, last_write) VALUES (%s, %s, %s, %s)', userid, appid, 0, 0)
    else:
        lastread = row['last_read']
        lastwrite = row['last_write']
    
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [userid, appid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)

    row = g.db.get('SELECT MAX(idx_app_user) as topid from Ranks where user_id=%s and app_id=%s', userid, appid);
    topid = row['topid']
    if topid == None:
        topid = 0
    return jsonify(error=DMSERR_NONE, userid=userid, datetime=str(datetime.datetime.utcnow()), topid=topid, unread=topid-lastread)

@userBluePrint.route('/dmsapi/user/heartbeat')
def userheartbeat():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)

    row = g.db.get('SELECT last_read, last_write FROM AppUserDatas WHERE user_id=%s AND app_id=%s', g.userid, g.appid)
    if row == None:
        return jsonify(error=DMSERR_SQL)
    lastread = row['last_read']
    lastwrite = row['last_write']

    token = session['usertoken']
    g.mc.delete(token)
    token = uuid.uuid4().hex
    session['usertoken'] = token
    cachedata = [g.userid, g.appid]
    g.mc.set(token, cachedata, CACHE_KEEP_SECOND)

    row = g.db.get('SELECT MAX(idx_app_user) as topid from Ranks where user_id=%s and app_id=%s', g.userid, g.appid);
    topid = row['topid']
    if topid == None:
        topid = 0
    return jsonify(error=DMSERR_NONE, datetime=str(datetime.datetime.utcnow()), topid=topid, unread=topid-lastread)

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

    try:
        g.db.execute('INSERT INTO Scores (user_id, game_id, date, time, score, idx_app_user) VALUES (%s, %s, UTC_DATE(), UTC_TIME(), 0, 0)', g.userid, gameid)
        key = uuid.uuid4().hex
        value = gameid
        g.mc.set(key, value, GAME_KEEP_SECOND)
        return jsonify(error=DMSERR_NONE, token=key)
    except:
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
            row = g.db.get('SELECT last_write FROM AppUserDatas WHERE user_id=%s AND app_id=%s', g.userid, g.appid)
            lastwrite = row['last_write']
            lastwrite += 1
            g.db.execute('REPLACE INTO Scores (user_id, game_id, date, time, score, idx_app_user) values(%s, %s, UTC_DATE(), UTC_TIME(), %s, %s)', g.userid, gameid, score, lastwrite)
            g.db.execute('UPDATE AppUserDatas SET last_write=last_write+1 WHERE user_id=%s AND app_id=%s', g.userid, g.appid)
            return jsonify(error=DMSERR_NONE, gameid=gameid, score=score)

@userBluePrint.route('/dmsapi/user/gettimeline')
def usergettimeline():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    topid = request.args.get('topid', type=int)
    limit = request.args.get('limit', type=int)
    if ( topid==None or topid <= 0 or limit==None or limit <= 0 ):
        return jsonify(error=DMSERR_PARAM)
    limit = min(limit, TIMELINE_LIMIT)

    rows = g.db.iter('''SELECT user_id, game_id, date, row, rank, score, time, user_name, nationality, idx_app_user FROM Ranks
                        WHERE user_id=%s AND app_id=%s AND idx_app_user<=%s AND idx_app_user>%s
                        ORDER BY idx_app_user DESC LIMIT %s'''
                        , g.userid, g.appid, topid, topid-limit, limit)
    ranks = [{'idx':row['idx_app_user'], 'userid':row['user_id'], 'gameid':row['game_id'], 'date':str(row['date']), 'row':row['row'], 'rank':row['rank'], 'score':row['score'], 'time':str(row['time']), 'username':row['user_name'], 'nationality':row['nationality'] } for row in rows]

    return jsonify(error=DMSERR_NONE, ranks=ranks)

@userBluePrint.route('/dmsapi/user/getranks')
def usergetranks():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    gameid = request.args.get('gameid', type=int)
    date = request.args.get('date', type=unicode)
    offset = request.args.get('offset', type=int)
    limit = request.args.get('limit', type=int)
    if ( offset==None or offset < 0 or limit==None or limit <= 0 ):
        return jsonify(error=DMSERR_PARAM)
    limit = min(limit, GETRANK_LIMIT)
    offset += 1

    rows = g.db.iter('''SELECT user_id, game_id, date, row, rank, score, time, user_name, nationality, idx_app_user FROM Ranks
                        WHERE game_id=%s AND date=%s AND row>=%s AND row<%s
                        ORDER BY row ASC'''
                        , gameid, date, offset, offset+limit)
    ranks = [{'idx':row['idx_app_user'], 'userid':row['user_id'], 'gameid':row['game_id'], 'date':str(row['date']), 'row':row['row'], 'rank':row['rank'], 'score':row['score'], 'time':str(row['time']), 'username':row['user_name'], 'nationality':row['nationality'] } for row in rows]
    
    return jsonify(error=DMSERR_NONE, ranks=ranks)
    
@userBluePrint.route('/dmsapi/user/allread')
def userallread():
    if (not userisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    g.db.execute('UPDATE AppUserDatas SET last_read=last_write WHERE user_id=%s AND app_id=%s', g.userid, g.appid)
