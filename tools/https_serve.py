#!/usr/bin/python3
from http.server import HTTPServer, SimpleHTTPRequestHandler
import ssl


httpd = HTTPServer(('0.0.0.0', 4443), SimpleHTTPRequestHandler)

httpd.socket = ssl.wrap_socket (httpd.socket, 
        keyfile="misc/key.pem", 
        certfile='misc/cert.pem', server_side=True)

httpd.serve_forever()
