#!/bin/bash

echo "=== Debug CGI Test ==="
echo "1. Vérification de la compilation..."
if [ ! -f "./webserv" ]; then
    echo "❌ webserv n'existe pas, compilation..."
    make clean && make
    if [ $? -ne 0 ]; then
        echo "❌ Erreur de compilation"
        exit 1
    fi
fi

echo "✅ webserv compilé"

echo "2. Vérification des scripts CGI..."
for script in srv/www/test_cgi.py srv/www/test_cgi.sh srv/www/test_cgi.php; do
    if [ -f "$script" ]; then
        echo "✅ $script existe"
        if [ -x "$script" ]; then
            echo "✅ $script est exécutable"
        else
            echo "⚠️  $script n'est pas exécutable, correction..."
            chmod +x "$script"
        fi
    else
        echo "❌ $script n'existe pas"
    fi
done

echo "3. Test de lancement du serveur..."
echo "Lancement du serveur en arrière-plan..."
./webserv config/good/webserv.conf &
SERVER_PID=$!

echo "PID du serveur: $SERVER_PID"

echo "4. Attente du démarrage..."
sleep 3

echo "5. Vérification du port 3434..."
if netstat -tlnp 2>/dev/null | grep -q ":3434 "; then
    echo "✅ Serveur écoute sur le port 3434"
else
    echo "❌ Serveur n'écoute pas sur le port 3434"
    echo "Vérification des processus webserv:"
    ps aux | grep webserv | grep -v grep
    echo "Arrêt du serveur..."
    kill $SERVER_PID 2>/dev/null
    exit 1
fi

echo "6. Test CGI Python..."
response=$(curl -s "http://localhost:3434/test_cgi.py" 2>/dev/null)
if echo "$response" | grep -q "Test CGI Python"; then
    echo "✅ CGI Python fonctionne"
    echo "Réponse: $(echo "$response" | head -3)"
else
    echo "❌ CGI Python échoue"
    echo "Réponse: $response"
fi

echo "7. Test CGI Bash..."
response=$(curl -s "http://localhost:3434/test_cgi.sh" 2>/dev/null)
if echo "$response" | grep -q "Test CGI Bash"; then
    echo "✅ CGI Bash fonctionne"
    echo "Réponse: $(echo "$response" | head -3)"
else
    echo "❌ CGI Bash échoue"
    echo "Réponse: $response"
fi

echo "8. Test CGI PHP..."
response=$(curl -s "http://localhost:3434/test_cgi.php" 2>/dev/null)
if echo "$response" | grep -q "Test CGI PHP"; then
    echo "✅ CGI PHP fonctionne"
    echo "Réponse: $(echo "$response" | head -3)"
else
    echo "❌ CGI PHP échoue"
    echo "Réponse: $response"
fi

echo "9. Nettoyage..."
kill $SERVER_PID 2>/dev/null
echo "✅ Serveur arrêté"

echo "=== Test terminé ==="
