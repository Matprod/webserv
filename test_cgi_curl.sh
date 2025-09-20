#!/bin/bash

echo "=== Tests CGI avec curl ==="
echo "Assurez-vous que votre serveur webserv est en cours d'exécution sur le port 3434"
echo ""

# Test 1: CGI Python
echo "1. Test CGI Python:"
curl -s "http://localhost:3434/test_cgi.py" | head -20
echo -e "\n"

# Test 2: CGI Bash
echo "2. Test CGI Bash:"
curl -s "http://localhost:3434/test_cgi.sh" | head -20
echo -e "\n"

# Test 3: CGI PHP
echo "3. Test CGI PHP:"
curl -s "http://localhost:3434/test_cgi.php" | head -20
echo -e "\n"

# Test 4: CGI avec query string
echo "4. Test CGI avec query string:"
curl -s "http://localhost:3434/test_cgi.py?param1=value1&param2=value2" | head -20
echo -e "\n"

# Test 5: CGI POST
echo "5. Test CGI POST:"
curl -s -X POST -d "name=test&value=hello" "http://localhost:3434/test_cgi.py" | head -20
echo -e "\n"

# Test 6: Extension non supportée
echo "6. Test extension non supportée:"
curl -s "http://localhost:3434/test_cgi.txt"
echo -e "\n"

echo "Tests terminés!"
