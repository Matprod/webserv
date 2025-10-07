#!/usr/bin/python3
import cgi
import os

print("Content-Type: text/html\r\n\r\n")
print("<html><body>")
print("<h2>Python CGI Test</h2>")
print("<p>Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "")))
if os.environ.get("REQUEST_METHOD") == "POST":
    form = cgi.FieldStorage()ewrre
    for key in form.keys():werwer
        print("<p>{}: {}</p>".format(key, form[key].value))
print("</body></html>")we rwer werwe rwer 
