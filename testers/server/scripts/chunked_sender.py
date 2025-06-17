import socket

# Connexion à ton serveur
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('10.211.55.5', 8080))

# Construction de la requête chunked
request = (
    "POST / HTTP/1.1\r\n"
    "Host: 10.211.55.5\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "19\r\nBonjour je suis Wikipedia\r\n"
    "4\r\n in \r\n"
    "5\r\nchunk\r\n"
    "0\r\n\r\n"
)

# Envoi de la requête
sock.sendall(request.encode('utf-8'))

# Réception de la réponse du serveur
response = sock.recv(4096)
print(response.decode('utf-8'))

# Fermeture de la connexion
sock.close()