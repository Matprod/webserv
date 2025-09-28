#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r\n\r\n")
print("<html><head><title>Test CGI Python</title></head><body>")
print("<h1>Test CGI Python - Succès!</h1>")
print("<h2>Variables d'environnement CGI:</h2>")
print("<ul>")

# Afficher les variables d'environnement importantes
env_vars = [
    'REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_LENGTH', 'CONTENT_TYPE',
    'SCRIPT_NAME', 'SCRIPT_FILENAME', 'PATH_INFO', 'SERVER_NAME',
    'SERVER_PORT', 'SERVER_PROTOCOL', 'GATEWAY_INTERFACE'
]

for var in env_vars:
    value = os.environ.get(var, 'Non défini')
    print(f"<li><strong>{var}:</strong> {value}</li>")

print("</ul>")

# Afficher les données POST si présentes
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"<h2>Données POST reçues:</h2>")
        print(f"<pre>{post_data}</pre>")

print("</body></html>")
