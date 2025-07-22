#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print()

print("""
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>CGI Python Test</title>
    <style>
        body {
            background-color: #f4f4f4;
            font-family: 'Segoe UI', sans-serif;
            margin: 50px;
            color: #333;
        }
        h1 {
            color: #007ACC;
        }
        .env {
            margin-top: 30px;
            padding: 20px;
            background-color: #fff;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        .env h2 {
            margin-bottom: 10px;
        }
        .env pre {
            font-size: 1.5em;
            background-color: #f9f9f9;
            padding: 10px;
            overflow-x: auto;
            border-left: 4px solid #007ACC;
        }
    </style>
</head>
<body>
    <h1>✅ Test CGI Python réussi</h1>
    <p>Ce script a été exécuté par le serveur via CGI.</p>

    <div class="env">
        <h2>Variables d'environnement CGI :</h2>
        <pre>""")

for key, value in sorted(os.environ.items()):
    print(f"{key} = {value}")

print("""        </pre>
    </div>
</body>
</html>
""")
