#!/usr/bin/python

import SimpleHTTPServer
import SocketServer
import webbrowser
import threading
import os
import StringIO
import json
import posixpath
import urllib
import cgi
import binascii

# import logging

PORT = 8080

class SkyDropCfgServer(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def __init__(self, request, client_address, server):
        SimpleHTTPServer.SimpleHTTPRequestHandler.__init__(self, request, client_address, server)
        
        self.post_data = False
        
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
        
        if params[1] == "save":
            path = os.path.abspath(os.path.dirname(path))
            print "PATH", path
            
            if (self.post_data == False):
                return self.send_error(501, "post data is empty")
            
            try:
                f = open(path, "w")
                f.write(self.post_data)
                f.close()
            except Exception, e:
                return self.send_error(501, str(e))
                
            
            data = json.dumps({"res":"ok"})
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
            
    def do_GET(self):
        """Serve a GET request."""
        f = self.send_head()
        if f:
            try:
                self.copyfile(f, self.wfile)
            finally:
                f.close()

    def do_HEAD(self):
        """Serve a HEAD request."""
        f = self.send_head()
        if f:
            f.close()            
            
    def do_POST(self):
        form = cgi.FieldStorage(
            fp=self.rfile,
            headers=self.headers,
            environ={'REQUEST_METHOD':'POST',
                     'CONTENT_TYPE':self.headers['Content-Type'],
                     })
        
        self.post_data = binascii.a2b_base64(form.file.read())

        self.do_GET()    
            
    def translate_path(self, path):
        """Translate a /-separated PATH to the local filename syntax.

        Components that mean special things to the local file system
        (e.g. drive or directory names) are ignored.  (XXX They should
        probably be diagnosed.)

        """
        
#         print path
        
        # abandon query parameters
        path = path.split('?',1)[0]
        path = path.split('#',1)[0]
        # Don't forget explicit trailing slash when normalizing. Issue17324
        trailing_slash = path.rstrip().endswith('/')
        path = posixpath.normpath(urllib.unquote(path))
        words = path.split('/')
        words = filter(None, words)
        path = os.path.join(os.getcwd(), "..", "app")
        for word in words:
            drive, word = os.path.splitdrive(word)
            head, word = os.path.split(word)
            if word == "up_dir":
                path = os.path.join(path, "..")
                continue
            if word in (os.curdir, os.pardir): continue
            path = os.path.join(path, word)
        if trailing_slash:
            path += '/'
        return path

        

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