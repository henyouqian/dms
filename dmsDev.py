from flask import *
from tornado.database import Connection
import MySQLdb
import pylibmc
import hashlib
import uuid
from dmsError import *
import dmsConfig

PASSWORDAPPEND = 'henyouqianhenyouqianD#$%894'
HEART_BEET_SECOND = 3600
CACHE_KEEP_SECOND = HEART_BEET_SECOND + 60
GAME_MAX_PER_DEVELOPER = 20
APP_MAX_PER_DEVELOPER = 10

devBluePrint = Blueprint('dev', __name__)

def devisLogin():
    if g.userid:
        return True
    return False

@devBluePrint.before_request
def before_request():
    if ( 'devtoken' in session ):
        token = session['devtoken']
        g.userid = g.mc.get(token)
    else:
        g.userid = None

@devBluePrint.route('/dmsapi/dev/register')
def devregister():
    email = request.args.get('email', type=unicode)
    password = request.args.get('password', type=unicode)
    if ( email==None or password==None ):
        return jsonify(error=DMSERR_PARAM)
    #todo: check email and password
    shapw = hashlib.sha256(password+PASSWORDAPPEND).hexdigest()
    secretkey = uuid.uuid4().hex;
    try:
        g.db.execute('INSERT INTO account_db.Developers (email, password, secret_key) VALUES(%s, %s, %s)', email, shapw, secretkey)
    except:
        return jsonify(error=DMSERR_EXIST)
    session['developer_email'] = email
    row = g.db.get('SELECT developer_id FROM account_db.Developers WHERE email=%s', email)
    session['developer_id'] = row
    
    return jsonify(error=DMSERR_NONE, developerid=session['developer_id'], developeremail=session['developer_email'])

@devBluePrint.route('/dmsapi/dev/getdeveloperid')
def devgetdeveloperid():
    return jsonify(developerid=g.userid)
    
@devBluePrint.route('/dmsapi/dev/login')
def devlogin():
    email = request.args.get('email', type=unicode)
    password = request.args.get('password', type=unicode)
    if ( email==None or password==None ):
        return jsonify(error=DMSERR_PARAM)
    row = g.db.get('SELECT password, developer_id FROM account_db.Developers WHERE email=%s', email)
    if ( row == None ):
        return jsonify(error=DMSERR_EXIST)
    else:
        if ( row['password'] == hashlib.sha256(password+PASSWORDAPPEND).hexdigest() ):
            if ( 'devtoken' in session ):
                g.mc.delete(session['devtoken'])
            session.clear()
            token = uuid.uuid4().hex
            session['devtoken'] = token
            g.mc.set(token, row['developer_id'], CACHE_KEEP_SECOND)
            session['email'] = email
            return jsonify(error=DMSERR_NONE)
        else:
            return jsonify(error=DMSERR_PASSWORD)
    
@devBluePrint.route('/dmsapi/dev/logout')
def devlogout():
    session.clear()
    return jsonify(error=DMSERR_NONE)

@devBluePrint.route('/dmsapi/dev/heartbeat')
def devheartbeat():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    token = session['devtoken']
    g.mc.delete(token)
    token = uuid.uuid4().hex
    session['devtoken'] = token
    g.mc.set(token, g.userid, CACHE_KEEP_SECOND)
    return jsonify(error=DMSERR_NONE, token=token)

@devBluePrint.route('/dmsapi/dev/getsecretkey')
def devgetsecretkey():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    row = g.db.get('SELECT secret_key FROM account_db.Developers WHERE developer_id=%s', g.userid)
    return jsonify(error=DMSERR_NONE, secretkey=row['secret_key']);

###apps
@devBluePrint.route('/dmsapi/dev/app/get')
def devgetapps():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    try:
        rows = g.db.iter('SELECT app_id, name FROM Apps WHERE developer_id=%s ORDER BY app_id ASC', g.userid)
        data = [{'id':row['app_id'], 'name':row['name']} for row in rows]
        return jsonify(error=DMSERR_NONE, data=data)
    except:
        return jsonify(error=DMSERR_SQL)
        
@devBluePrint.route('/dmsapi/dev/app/add')
def devaddapp():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    name = request.args.get('name', type=unicode)
    if ( name==None ):
        return jsonify(error=DMSERR_PARAM)
    row = g.db.get('SELECT COUNT(*) FROM Apps WHERE developer_id=%s', g.userid)
    if row['COUNT(*)'] >= APP_MAX_PER_DEVELOPER:
        return jsonify(error=DMSERR_RANGE)
    try:
        g.db.execute('INSERT INTO Apps (developer_id, name) VALUES(%s, %s)', g.userid, name)
    except MySQLdb.IntegrityError as e:
        if ( e.args[0] == 1062 ):
            return jsonify(error=DMSERR_EXIST)
        else:
            return jsonify(error=DMSERR_SQL)
    except:
        return jsonify(error=DMSERR_SQL)
    return jsonify(error=DMSERR_NONE)
    
@devBluePrint.route('/dmsapi/dev/app/delete')
def devdeleteapp():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    appid = request.args.get('id', type=int)
    if ( appid==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        g.db.execute('DELETE FROM Apps WHERE developer_id=%s AND app_id=%s', g.userid, appid)
    except:
        return jsonify(error=DMSERR_SQL)
    return jsonify(error=DMSERR_NONE)

###games
@devBluePrint.route('/dmsapi/dev/game/get')
def devgetgames():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    appid = request.args.get('appid', type=int)
    if ( appid==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        rows = g.db.iter('SELECT game_id, name, score_order, app_id FROM Games WHERE developer_id=%s AND app_id=%s order by game_id ASC', g.userid, appid)
        data = [{'id':row['game_id'], 'name':row['name'], 'order':row['score_order'], 'appid':row['app_id']} for row in rows]
        return jsonify(error=DMSERR_NONE, data=data)
    except:
        return jsonify(error=DMSERR_SQL)

@devBluePrint.route('/dmsapi/dev/game/add')
def devaddgame():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    name = request.args.get('name', type=unicode)
    order = request.args.get('order', type=int)
    appid = request.args.get('appid', type=int)
    if ( name==None or order==None or appid==None ):
        return jsonify(error=DMSERR_PARAM)
    row = g.db.get('SELECT COUNT(*) FROM Games WHERE developer_id=%s', g.userid)
    if row['COUNT(*)'] >= GAME_MAX_PER_DEVELOPER:
        return jsonify(error=DMSERR_RANGE)
    try:
        g.db.execute('INSERT INTO Games (developer_id, name, score_order, app_id) VALUES(%s, %s, %s, %s)', g.userid, name, order, appid)
    except MySQLdb.IntegrityError as e:
        if ( e.args[0] == 1062 ):
            return jsonify(error=DMSERR_EXIST)
        else:
            return jsonify(error=DMSERR_SQL)
    except:
        return jsonify(error=DMSERR_SQL)
    g.mc.delete('games%d' % g.userid)
    return jsonify(error=DMSERR_NONE)

@devBluePrint.route('/dmsapi/dev/game/edit')
def deveditgame():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    gameid = request.args.get('id', type=int)
    name = request.args.get('name', type=unicode)
    order = request.args.get('order', type=int)
    appid = request.args.get('appid', type=int)
    if ( gameid==None or name==None or order==None or appid==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        g.db.execute('UPDATE Games SET name=%s, score_order=%s, app_id=%s WHERE developer_id=%s AND game_id=%s', name, order, appid, g.userid, gameid)
    except MySQLdb.IntegrityError as e:
        if ( e.args[0] == 1062 ):
            return jsonify(error=DMSERR_EXIST, name=name)
        else:
            return jsonify(error=DMSERR_SQL)
    except:
        return jsonify(error=DMSERR_SQL)
    return jsonify(error=DMSERR_NONE)

@devBluePrint.route('/dmsapi/dev/game/delete')
def devdeletegame():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    gameid = request.args.get('id', type=int)
    if ( gameid==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        g.db.execute('DELETE FROM Games WHERE developer_id=%s AND game_id=%s', g.userid, gameid)
    except:
        return jsonify(error=DMSERR_SQL)
    return jsonify(error=DMSERR_NONE)

###matchs
@devBluePrint.route('/dmsapi/dev/match/add')
def devmatchadd():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    name = request.args.get('name', type=unicode)
    date = request.args.get('date', type=unicode)
    gameid = request.args.get('gameid', type=int)
    if ( name==None or date==None or gameid==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        g.db.execute('INSERT INTO Matches (developer_id, name, date, game_id) VALUES(%s, %s, %s, %s)', g.userid, name, date, gameid)
    except:
        return jsonify(error=DMSERR_SQL)
    return jsonify(error=DMSERR_NONE)
	
@devBluePrint.route('/dmsapi/dev/match/edit')
def devmatchedit():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    matchid = request.args.get('matchid', type=int)
    name = request.args.get('name', type=unicode)
    date = request.args.get('date', type=unicode)
    gameid = request.args.get('gameid', type=int)
    if ( matchid==None or name==None or date==None or gameid==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        g.db.execute('UPDATE Matches SET name=%s, date=%s, game_id=%s WHERE developer_id=%s AND match_id=%s', name, date, gameid, g.userid, matchid)
    except:
        return jsonify(error=DMSERR_SQL)
    return jsonify(error=DMSERR_NONE)
	
@devBluePrint.route('/dmsapi/dev/match/delete')
def devmatchdelete():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    matchid = request.args.get('matchid', type=int)
    if ( matchid==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        g.db.execute('DELETE FROM Matches WHERE developer_id=%s AND match_id=%s', g.userid, matchid)
    except:
        return jsonify(error=DMSERR_SQL)
    return jsonify(error=DMSERR_NONE)

@devBluePrint.route('/dmsapi/dev/match/get')
def devmatchget():
    if (not devisLogin()):
        return jsonify(error=DMSERR_LOGIN)
    offset = request.args.get('offset', type=int)
    limit = request.args.get('limit', type=int)
    limit = max(1, min(50, limit))
    if ( offset==None or limit==None ):
        return jsonify(error=DMSERR_PARAM)
    try:
        rows = g.db.iter('SELECT m.match_id, m.name, m.date, m.game_id, g.name FROM Matches AS m INNER JOIN Games AS g ON m.game_id=g.game_id WHERE m.developer_id=%s ORDER BY m.date DESC LIMIT %s OFFSET %s', g.userid,limit, offset)
        data = [{'id':row['m.match_id'], 'name':row['m.name'], 'date':str(row['m.date']), 'gameid':row['m.game_id'], 'gamename':row['g.name']} for row in rows]
        return jsonify(error=DMSERR_NONE, data=data)
    except:
        return jsonify(error=DMSERR_SQL)
