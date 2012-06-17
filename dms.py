from tornado.database import Connection
from flask import Flask, g
import pylibmc
import thread
import datetime
import time
import sys
import dmsConfig
import dmsUser
import dmsDev
import dmsDevView
import dmsGenRank

app = Flask(__name__)
app.secret_key = '0dc36da3afbb475dbcf27ce2d071c4eb'

app.register_blueprint(dmsUser.userBluePrint)
app.register_blueprint(dmsDev.devBluePrint)

def init():
    app.mc = pylibmc.Client(['127.0.0.1'])
    app.rankDate = datetime.datetime.utcnow().date()
    thread.start_new_thread(genRankThread,())

@app.before_request
def beforRequest():
    g.db = Connection(dmsConfig.host,
                      dmsConfig.dms_db,
                      dmsConfig.user,
                      dmsConfig.passwd)
    g.mc = app.mc

@app.after_request
def afterRequest(response):
    g.db.close()
    return response

def genRankThread():
    while True:
        d = datetime.datetime.utcnow().date()
        if ( d != app.rankDate ):
            app.rankDate = d
            print 'genRank begin:date=%s' % str(app.rankDate)
            dmsGenRank.genRank(app.rankDate)
            print 'genRank end:date=%s' % str(app.rankDate)
        time.sleep(10)

from tornado.wsgi import WSGIContainer
from tornado.httpserver import HTTPServer
from tornado.ioloop import IOLoop

if __name__ == '__main__':
    init()
    argv = sys.argv
    if ('-d' in argv):
        app.run(host='0.0.0.0', port=8000)
    else:
        http_server = HTTPServer(WSGIContainer(app))
        http_server.bind(8000)
        http_server.start(0)
        IOLoop.instance().start()