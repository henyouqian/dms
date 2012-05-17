import MySQLdb

def genRank(date):
    conn = MySQLdb.connect(host="localhost",user="root",passwd="Nmmgb808313",db="dms_db",charset = "utf8")
    cur=conn.cursor()
	
    datestr = str(date)
    cur.execute('DELETE FROM Ranks WHERE date=%s', datestr)
    conn.commit()
    cur.execute('SELECT game_id, score_order FROM Games')
    for row in cur.fetchall():
        gameid = row[0]
        order = row[1]
        comp=''
        if ( order ):
            order = 'DESC'
            comp='MAX'
        else:
	        order = 'ASC'
	        comp='MIN'
        cur.execute('SET @rownum = 0, @rank = 1, @prev_val = NULL')
        cur.execute('''INSERT INTO Ranks (user_id, game_id, date, time, row, rank, score, user_name, nationality, idx_app_user) 
                        SELECT s.user_id, s.game_id, s.date, s.time
                            , @rownum := @rownum + 1
                            , @rank := IF(@prev_val!=s.score,@rownum,@rank)
                            , @prev_val := s.score
                            , u.name
                            , u.nationality
                            , 0
                        FROM Scores AS s
                        LEFT JOIN account_db.Users AS u
                        ON s.user_id=u.user_id
                        WHERE game_id=%s AND date=%s AND score!=0
                        ORDER BY score '''+order+' , time ASC', (gameid, datestr))
        conn.commit()

    cur.execute('SELECT app_id FROM Apps')
    for app in cur.fetchall():
        appid = app[0]
        cur.execute('SELECT user_id FROM account_db.Users')
        for user in cur.fetchall():
            userid = user[0]
            cur.execute('SELECT COUNT(*) FROM Ranks AS r WHERE (SELECT app_id FROM Games AS g WHERE g.game_id=r.game_id )=%s AND r.user_id=%s', (appid, userid))
            maxid = cur.fetchone
            cur.execute('SET @idx = %s', maxid)
            cur.execute('''UPDATE Ranks AS r SET idx_app_user=(@idx := @idx+1) 
                        WHERE r.user_id=%s AND r.idx_app_user=0 
                        AND (SELECT g.app_id FROM Games AS g WHERE g.game_id=r.game_id)=%s
                        ORDER BY time ASC''', (userid, appid) )
            cur.execute('UPDATE AppUserDatas SET last_write=@idx WHERE user_id=%s AND app_id=%s AND last_write<@idx', (userid, appid))
            conn.commit()

from datetime import datetime
def genToday():
    d = datetime.utcnow().date()
    genRank(d)
