from dmsDev import *

@devBluePrint.route('/dms/dev')
def devview():
    if devisLogin():
        return render_template('dmsDevMainMenu.html', email=session['email'])
    else:
        return render_template('dmsDevLogin.html')
    
@devBluePrint.route('/dms/dev/register')
def devregisterview():
    return render_template('dmsDevRegister.html')
	
@devBluePrint.route('/dms/dev/game')
def devgameview():
    return render_template('dmsDevGame.html')

@devBluePrint.route('/dms/dev/match')
def devmatchview():
    return render_template('dmsDevMatch.html')

def getgames():
    games = g.cache.get('games%d' % g.userid)
    if not games:
        try:
            rows = g.db.iter('SELECT game_id, name FROM Games WHERE developer_id=%s order by game_id DESC', g.userid)
            games = [{'id':row['game_id'], 'name':row['name']} for row in rows]
            g.cache.set('games%d' % g.userid, games, CACHE_KEEP_SECOND)
        except:
            return None;
    return games

@devBluePrint.route('/dms/dev/match/add')
def devmatchaddview():
    if (not devisLogin()):
        return render_template('dmsDevLogin.html')
    games = getgames()
    if not games:
        return abort(500)
    return render_template('dmsDevMatchAdd.html', games=games)

@devBluePrint.route('/dms/dev/match/edit')
def devmatcheditview():
    if (not devisLogin()):
        return render_template('dmsDevLogin.html')
    data={}
    data['matchid'] = request.args.get('matchid', type=int)
    data['matchname'] = request.args.get('matchname', type=unicode)
    data['gameid'] = request.args.get('gameid', type=int)
    data['date'] = request.args.get('date', type=unicode)
    data['games'] = getgames()
    return render_template('dmsDevMatchEdit.html', data=data)

