#!/usr/bin/python3
import cgi
import os
import sys

print("Content-Type: text/html\r\n\r\n")

method = os.environ.get("REQUEST_METHOD", "")

print("<html><head><title>Form CGI</title></head><body>")
print("<h2>Interactive Form CGI</h2>")
print("<p>Method: <strong>{}</strong></p>".format(method))

if method == "POST":
    # Lire les données POST
    form = cgi.FieldStorage()
    
    print("<h3>Received POST Data:</h3>")
    print("<ul>")
    
    if form:
        for key in form.keys():
            value = form[key].value
            print("<li><strong>{}:</strong> {}</li>".format(key, value))
    else:
        print("<li>No form data received</li>")
    
    print("</ul>")
    
    # Afficher aussi le contenu brut
    content_length = os.environ.get("CONTENT_LENGTH", "0")
    if content_length != "0":
        print("<h3>Raw POST Body:</h3>")
        print("<pre>{}</pre>".format(sys.stdin.read(int(content_length))))
else:
    # GET - afficher un formulaire
    print("<h3>Submit a form:</h3>")
    print("<form method='POST' action='/form_cgi.py'>")
    print("  <label>Name: <input type='text' name='name' /></label><br/>")
    print("  <label>Email: <input type='email' name='email' /></label><br/>")
    print("  <label>Message: <textarea name='message'></textarea></label><br/>")
    print("  <button type='submit'>Submit</button>")
    print("</form>")

print("</body></html>")

