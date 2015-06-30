#!/usr/bin/python

import SimpleHTTPServer
import SocketServer
import webbrowser
import threading


PORT = 9999

def ServerThread():
    global PORT
    
    Handler = SimpleHTTPServer.SimpleHTTPRequestHandler
    httpd = SocketServer.TCPServer(("localhost", PORT), Handler)
    
    print "serving at port", PORT
    httpd.serve_forever()
    
t1 = threading.Thread(target=ServerThread)
t1.start()

webbrowser.open_new("http://localhost:%d" % PORT)

t1.join()