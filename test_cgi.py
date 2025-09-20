#!/usr/bin/env python3
"""
Script de test pour la fonction executeCGI
Teste différents scénarios CGI avec des requêtes HTTP simulées
"""

import socket
import time
import sys

def send_http_request(host, port, request):
    """Envoie une requête HTTP et retourne la réponse"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))
        sock.send(request.encode())
        
        response = b""
        while True:
            data = sock.recv(4096)
            if not data:
                break
            response += data
        
        sock.close()
        return response.decode()
    except Exception as e:
        print(f"Erreur de connexion: {e}")
        return None

def test_cgi_python():
    """Test CGI avec un script Python"""
    print("=== Test CGI Python ===")
    request = """GET /test_cgi.py HTTP/1.1\r
Host: localhost:3434\r
User-Agent: TestClient/1.0\r
\r
"""
    response = send_http_request("localhost", 3434, request)
    if response:
        print("Réponse reçue:")
        print(response[:500] + "..." if len(response) > 500 else response)
        return "Content-Type: text/html" in response and "Test CGI Python" in response
    return False

def test_cgi_bash():
    """Test CGI avec un script Bash"""
    print("\n=== Test CGI Bash ===")
    request = """GET /test_cgi.sh HTTP/1.1\r
Host: localhost:3434\r
User-Agent: TestClient/1.0\r
\r
"""
    response = send_http_request("localhost", 3434, request)
    if response:
        print("Réponse reçue:")
        print(response[:500] + "..." if len(response) > 500 else response)
        return "Content-Type: text/html" in response and "Test CGI Bash" in response
    return False

def test_cgi_php():
    """Test CGI avec un script PHP"""
    print("\n=== Test CGI PHP ===")
    request = """GET /test_cgi.php HTTP/1.1\r
Host: localhost:3434\r
User-Agent: TestClient/1.0\r
\r
"""
    response = send_http_request("localhost", 3434, request)
    if response:
        print("Réponse reçue:")
        print(response[:500] + "..." if len(response) > 500 else response)
        return "Content-Type: text/html" in response and "Test CGI PHP" in response
    return False

def test_cgi_post():
    """Test CGI avec une requête POST"""
    print("\n=== Test CGI POST ===")
    post_data = "name=test&value=hello"
    request = f"""POST /test_cgi.py HTTP/1.1\r
Host: localhost:3434\r
User-Agent: TestClient/1.0\r
Content-Type: application/x-www-form-urlencoded\r
Content-Length: {len(post_data)}\r
\r
{post_data}"""
    
    response = send_http_request("localhost", 3434, request)
    if response:
        print("Réponse reçue:")
        print(response[:500] + "..." if len(response) > 500 else response)
        return "Content-Type: text/html" in response and post_data in response
    return False

def test_cgi_query_string():
    """Test CGI avec query string"""
    print("\n=== Test CGI Query String ===")
    request = """GET /test_cgi.py?param1=value1&param2=value2 HTTP/1.1\r
Host: localhost:3434\r
User-Agent: TestClient/1.0\r
\r
"""
    response = send_http_request("localhost", 3434, request)
    if response:
        print("Réponse reçue:")
        print(response[:500] + "..." if len(response) > 500 else response)
        return "Content-Type: text/html" in response and "param1=value1" in response
    return False

def test_cgi_invalid_extension():
    """Test CGI avec une extension non supportée"""
    print("\n=== Test CGI Extension Non Supportée ===")
    request = """GET /test_cgi.txt HTTP/1.1\r
Host: localhost:3434\r
User-Agent: TestClient/1.0\r
\r
"""
    response = send_http_request("localhost", 3434, request)
    if response:
        print("Réponse reçue:")
        print(response[:500] + "..." if len(response) > 500 else response)
        return "404" in response and "CGI Extension Not Supported" in response
    return False

def main():
    print("Démarrage des tests CGI...")
    print("Assurez-vous que votre serveur webserv est en cours d'exécution sur le port 3434")
    print("Appuyez sur Entrée pour continuer...")
    input()
    
    tests = [
        ("Python CGI", test_cgi_python),
        ("Bash CGI", test_cgi_bash),
        ("PHP CGI", test_cgi_php),
        ("POST CGI", test_cgi_post),
        ("Query String CGI", test_cgi_query_string),
        ("Extension Non Supportée", test_cgi_invalid_extension)
    ]
    
    results = []
    for test_name, test_func in tests:
        try:
            result = test_func()
            results.append((test_name, result))
            print(f"✓ {test_name}: {'SUCCÈS' if result else 'ÉCHEC'}")
        except Exception as e:
            print(f"✗ {test_name}: ERREUR - {e}")
            results.append((test_name, False))
    
    print("\n=== RÉSULTATS FINAUX ===")
    success_count = sum(1 for _, result in results if result)
    total_count = len(results)
    
    for test_name, result in results:
        status = "✓ SUCCÈS" if result else "✗ ÉCHEC"
        print(f"{test_name}: {status}")
    
    print(f"\nTotal: {success_count}/{total_count} tests réussis")
    
    if success_count == total_count:
        print("🎉 Tous les tests CGI ont réussi !")
    else:
        print("⚠️  Certains tests ont échoué. Vérifiez les logs du serveur.")

if __name__ == "__main__":
    main()
