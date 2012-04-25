from tornado.database import Connection
from flask import Flask, g
import pylibmc
import dmsConfig
import dmsUser
import dmsDev
import dmsDevView

app = Flask(__name__)
app.debug = True
app.secret_key = '0dc36da3afbb475dbcf27ce2d071c4eb'

app.register_blueprint(dmsUser.userBluePrint)
app.register_blueprint(dmsDev.devBluePrint)

def init():
    app.mc = pylibmc.Client(['127.0.0.1'])

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

from tornado.wsgi import WSGIContainer
from tornado.httpserver import HTTPServer
from tornado.ioloop import IOLoop

if __name__ == '__main__':
    init()
    
    if app.debug:
        app.run(host='0.0.0.0', port=8000)
    else:
        http_server = HTTPServer(WSGIContainer(app))
        http_server.bind(8000)
        http_server.start(0)
        IOLoop.instance().start()