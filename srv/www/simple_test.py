#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print("")
print("<h1>Test CGI Simple</h1>")
print("<p>Si vous voyez ceci, le CGI fonctionne !</p>")
print("<p>Méthode:", os.environ.get('REQUEST_METHOD', 'N/A'), "</p>")
print("<p>URI:", os.environ.get('REQUEST_URI', 'N/A'), "</p>")
