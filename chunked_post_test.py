import socket

host = 'localhost'
port = 3434
path = '/cgi-bin/test.py'

# Corps de la requête encodée en chunks HTTP
chunks = [
    "4\r\ntest\r\n",
    "3\r\n=42\r\n",
    "0\r\n\r\n"
]

# Construction de la requête HTTP manuelle
request = (
    f"POST {path} HTTP/1.1\r\n"
    f"Host: {host}:{port}\r\n"
    f"Transfer-Encoding: chunked\r\n"
    f"Content-Type: application/x-www-form-urlencoded\r\n"
    f"Connection: close\r\n"
    f"\r\n"
    + "".join(chunks)
)

# Connexion socket vers ton serveur HTTP
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((host, port))
    s.sendall(request.encode())
    response = b""
    while True:
        data = s.recv(1024)
        if not data:
            break
        response += data

# Affiche la réponse brute du serveur
print(response.decode())
