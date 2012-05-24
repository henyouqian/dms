import MySQLdb

def genRank(date):
    conn = MySQLdb.connect(host="localhost",user="root",passwd="Nmmgb808313",db="dms_db",charset = "utf8")
    cur=conn.cursor()
	
    datestr = str(date)
    cur.execute('DELETE FROM Ranks WHERE date=%s', datestr)
    conn.commit()
    cur.execute('SELECT game_id, score_order FROM Games')
    i = 0
    for row in cur.fetchall():
        print i
        i += 1;
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
        cur.execute('''INSERT INTO Ranks (user_id, game_id, app_id, date, time, row, rank, score, user_name, nationality, idx_app_user) 
                        SELECT s.user_id, s.game_id
                            , (SELECT app_id FROM Games AS g WHERE g.game_id=s.game_id )
                            , s.date, s.time
                            , @rownum := @rownum + 1
                            , @rank := IF(@prev_val!=s.score,@rownum,@rank)
                            , @prev_val := s.score
                            , u.name
                            , u.nationality
                            , s.idx_app_user
                        FROM Scores AS s
                        LEFT JOIN account_db.Users AS u
                        ON s.user_id=u.user_id
                        WHERE s.game_id=%s AND s.date=%s AND s.score!=0
                        ORDER BY score '''+order+' , time ASC', (gameid, datestr))
        conn.commit()
    print 'Rank done!'
    
    # i = 0
    # cur.execute('SELECT app_id FROM Apps')
    # for app in cur.fetchall():
    #     appid = app[0]
    #     cur.execute('SELECT user_id FROM account_db.Users')
    #     for user in cur.fetchall():
    #         if ( i % 10 == 0 ):
    #             print i
    #         i += 1;
    #         userid = user[0]
    #         cur.execute('SELECT MAX(idx_app_user) FROM Ranks WHERE app_id=%s AND user_id=%s', (appid, userid))
    #         maxid = cur.fetchone()[0]
    #         cur.execute('SET @idx = %s', (maxid,))
    #         cur.execute('''UPDATE Ranks SET idx_app_user=(@idx := @idx+1) 
    #                     WHERE user_id=%s AND idx_app_user=0 AND app_id=%s
    #                     ORDER BY time ASC''', (userid, appid) )
    #         cur.execute('UPDATE AppUserDatas SET last_write=@idx WHERE user_id=%s AND app_id=%s AND last_write<@idx', (userid, appid))
    #         conn.commit()
    # print 'Rank idx done!'
    
    conn.close()

from datetime import datetime, timedelta
def genToday():
    d = datetime.utcnow().date()
    genRank(d)

def genRankOffset(offset):
    d = datetime.utcnow().date()
    dlt = timedelta(days=offset)
    d = d + dlt
    genRank(d)