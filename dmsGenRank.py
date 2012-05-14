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
        cur.execute('''INSERT INTO Ranks (user_id, game_id, date, time, row, rank, score, user_name, nationality, idx_for_user) 
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

    cur.execute('SELECT user_id FROM account_db.Users')
    for row in cur.fetchall():
        userid = row[0]
        cur.execute('UPDATE UserDatas SET unread = unread+(SELECT count(*) FROM Ranks WHERE user_id=%s AND idx_for_user=0) WHERE user_id=%s', (userid, userid))
        cur.execute('SET @idx = (SELECT MAX(idx_for_user) FROM Ranks WHERE user_id=%s)', userid)
        cur.execute('UPDATE Ranks SET idx_for_user=(@idx := @idx+1) WHERE user_id=%s AND idx_for_user=0 ORDER BY time ASC', userid )
        conn.commit()