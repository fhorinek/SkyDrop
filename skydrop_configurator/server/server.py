#!/usr/bin/python

import SimpleHTTPServer
import SocketServer
import webbrowser
import threading
import os
import StringIO
import json

PORT = 9999

class SkyDropCfgServer(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def __init__(self, request, client_address, server):
        SimpleHTTPServer.SimpleHTTPRequestHandler.__init__(self, request, client_address, server)
        
    def exec_cmd(self, path):
        
        params = os.path.basename(path).split(":")
        
        if params[1] == "list":
            try:
                flist = os.listdir(os.path.dirname(path))
            except os.error:
                self.send_error(404, "No permission to list directory")
                return None
            flist.sort(key=lambda a: a.lower())
            
            data = json.dumps(flist)
            buf = StringIO.StringIO(data)
            
            self.send_response(200)
            self.send_header("Content-type", "text/plain")
            self.send_header("Content-Length", len(data))
            self.end_headers()            
            
            return buf
            
        return self.send_error(501, "Unknown cmd")
            
            
        
    def send_head(self):
        """Common code for GET and HEAD commands.

        This sends the response code and MIME headers.

        Return value is either a file object (which has to be copied
        to the outputfile by the caller unless the command was HEAD,
        and must be closed by the caller under all circumstances), or
        None, in which case the caller has nothing further to do.

        """
        path = self.translate_path(self.path)
        f = None
        if os.path.isdir(path):
            if not self.path.endswith('/'):
                # redirect browser - doing basically what apache does
                self.send_response(301)
                self.send_header("Location", self.path + "/")
                self.end_headers()
                return None
            for index in "index.html", "index.htm":
                index = os.path.join(path, index)
                if os.path.exists(index):
                    path = index
                    break
            else:
                return self.list_directory(path)
        ctype = self.guess_type(path)
        
        name = os.path.basename(path)
        if name.find("cmd:") == 0:
            return self.exec_cmd(path)
        
        try:
            # Always read in binary mode. Opening files in text mode may cause
            # newline translations, making the actual size of the content
            # transmitted *less* than the content-length!
            f = open(path, 'rb')
        except IOError:
            self.send_error(404, "File not found")
            return None
        try:
            self.send_response(200)
            self.send_header("Content-type", ctype)
            fs = os.fstat(f.fileno())
            self.send_header("Content-Length", str(fs[6]))
            self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
            self.end_headers()
            return f
        except:
            f.close()
            raise        
        

def ServerThread():
    global PORT
    
    Handler = SkyDropCfgServer
    httpd = SocketServer.TCPServer(("localhost", PORT), Handler)
    
    print "serving at port", PORT
    httpd.serve_forever()
    
t1 = threading.Thread(target=ServerThread)
t1.start()

webbrowser.open_new("http://localhost:%d" % PORT)

t1.join()