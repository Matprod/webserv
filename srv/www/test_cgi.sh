#!/bin/bash

echo "Content-Type: text/html"
echo ""
echo "<html><head><title>Test CGI Bash</title></head><body>"
echo "<h1>Test CGI Bash - Succès!</h1>"
echo "<h2>Variables d'environnement CGI:</h2>"
echo "<ul>"

# Afficher les variables d'environnement importantes
for var in REQUEST_METHOD QUERY_STRING CONTENT_LENGTH CONTENT_TYPE SCRIPT_NAME SCRIPT_FILENAME PATH_INFO SERVER_NAME SERVER_PORT SERVER_PROTOCOL GATEWAY_INTERFACE; do
    value="${!var:-Non défini}"
    echo "<li><strong>$var:</strong> $value</li>"
done

echo "</ul>"
# Afficher les données POST si présentes
if [ "$REQUEST_METHOD" = "POST" ]; then
    if [ -n "$CONTENT_LENGTH" ] && [ "$CONTENT_LENGTH" -gt 0 ]; then
        echo "<h2>Données POST reçues:</h2>"
        echo "<pre>"
        head -c "$CONTENT_LENGTH"
        echo "</pre>"
    fi
fi

echo "</body></html>"
