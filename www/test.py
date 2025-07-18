#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html")
print()

print("<html><body>")
print("<h1>Hello depuis un script CGI !</h1>")
print("<p>PATH_INFO = {}</p>".format(os.environ.get("PATH_INFO", "")))
print("</body></html>")
