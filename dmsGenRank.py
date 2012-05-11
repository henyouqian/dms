import MySQLdb

def genRank(date):
    conn = MySQLdb.connect(host="localhost",user="root",passwd="Nmmgb808313",db="dms_db",charset = "utf8")
    cur=conn.cursor()
	
    datestr = str(date)
    cur.execute('DELETE FROM Ranks WHERE date=%s', datestr)
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
        cur.execute('''INSERT INTO Ranks (user_id, game_id, date, time, row, rank, score) 
                        SELECT user_id, game_id, date, time
                            , @rownum := @rownum + 1
                            , @rank := IF(@prev_val!=score,@rownum,@rank)
                            , @prev_val := score
                        FROM Scores
                        WHERE game_id=%s AND date=%s AND score!=0
                        ORDER BY score '''+order+' , time ASC', (gameid, datestr))
        conn.commit()
