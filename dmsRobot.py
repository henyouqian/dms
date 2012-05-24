import MySQLdb
import random
from datetime import datetime, timedelta

def genUser(count):
    conn = MySQLdb.connect(host="localhost",user="root",passwd="Nmmgb808313",db="dms_db",charset = "utf8")
    cur=conn.cursor()
    for i in xrange(0,count):
        if ( i % 1000 == 0 ):
            print i
        name = 'robot%s' % (i+1000000)
        try:
            cur.execute('INSERT INTO account_db.Users (gamecenter_id, name, nationality) VALUES(%s, %s, 99)', (name, name))
            conn.commit()
        except Exception, e:
            pass
    conn.close()

def genScore(dayoffset, gameid, count):
    conn = MySQLdb.connect(host="localhost",user="root",passwd="Nmmgb808313",db="dms_db",charset = "utf8")
    cur=conn.cursor()
    
    d = datetime.utcnow().date()
    dlt = timedelta(days=dayoffset)
    date = str(d + dlt)
    cur.execute('SELECT app_id FROM Games WHERE game_id=%s', (gameid,));
    appid = cur.fetchone()[0]
    for i in xrange(0,count):
        if ( i % 100 == 0 ):
            print i
        name = 'robot%s' % (i+1000000)
        cur.execute('SELECT user_id FROM account_db.Users WHERE gamecenter_id=%s', (name,));
        userid = cur.fetchone()[0]
        try:
            cur.execute('INSERT INTO AppUserDatas (user_id, app_id, last_read, last_write) VALUES (%s, %s, %s, %s)', (userid, appid, 0, 0))
            conn.commit()
        except:
            pass

        try:
            score = random.randint(1, 9999)
            cur.execute('SELECT last_write FROM AppUserDatas WHERE user_id=%s AND app_id=%s', (userid, appid))
            lastwrite = cur.fetchone()[0]
            cur.execute('''INSERT INTO Scores (user_id, game_id, date, time, score, idx_app_user) VALUES(
                        %s, %s, %s, UTC_TIME(), %s, %s)'''
                        , (userid, gameid, date, score, lastwrite+1))
            cur.execute('UPDATE AppUserDatas SET last_write=last_write+1 WHERE user_id=%s AND app_id=%s', (userid, appid))
            conn.commit()
        except:
            pass
		
    conn.close()
	