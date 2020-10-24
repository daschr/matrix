#!/usr/bin/python3
from http.server import HTTPServer, SimpleHTTPRequestHandler
import ssl
import sys

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} [key] [cert]", file=sys.stderr)
    sys.exit(1)

httpd = HTTPServer(('0.0.0.0', 4443), SimpleHTTPRequestHandler)

httpd.socket = ssl.wrap_socket (httpd.socket, 
        keyfile=sys.argv[1], 
        certfile=sys.argv[2], server_side=True)

httpd.serve_forever()
