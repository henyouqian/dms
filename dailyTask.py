import time
import datetime
import MySQLdb

class g:
	pass

def main():
	g.conn = MySQLdb.connect(host="localhost",user="root",passwd="verygame123",db="dms",charset = "utf8")
	g.cur=g.conn.cursor()
	currtime = datetime.datetime.utcnow()
	
	doWork(currtime)
	print('done')
	'''
	while 1:
		utcnow = datetime.datetime.utcnow()
		if ( currtime.minute != utcnow.minute ):
			doWork(currtime)
			currtime = utcnow
		time.sleep(1)
	'''
def doWork(currtime):
	datestr = str(currtime.date())
	g.cur.execute('SELECT m.match_id, g.score_lower_better FROM Matches AS m INNER JOIN Games AS g ON m.game_id=g.game_id WHERE m.date=%s', datestr)
	for row in g.cur.fetchall():
		matchid=row[0]
		lowbetter=row[1]
		order='DESC'
		comp='MAX'
		if (lowbetter):
			order='ASC'
			comp='MIN'
		g.cur.execute('SET @rownum = 0, @rank = 1, @prev_val = NULL')
		g.cur.execute('''INSERT INTO Ranks (user_id, match_id, row, rank, score, time) 
						SELECT user_id, match_id
							 , @rownum := @rownum + 1
							 , @rank := IF(@prev_val!=score,@rownum,@rank)
							 , @prev_val := score
							 , time
						FROM Scores
						WHERE match_id=%s AND failed=1
						ORDER BY score '''+order, matchid)
		g.cur.execute('UPDATE Matches SET score_best=(SELECT '+comp+'(score) from Scores where match_id=%s) WHERE match_id=%s', (matchid, matchid))
		g.conn.commit()
if __name__ == '__main__':
	main()