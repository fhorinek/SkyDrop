#!/usr/bin/python

import SimpleHTTPServer
import SocketServer

PORT = 9999

Handler = SimpleHTTPServer.SimpleHTTPRequestHandler

httpd = SocketServer.TCPServer(("localhost", PORT), Handler)

print "serving at port", PORT
httpd.serve_forever()