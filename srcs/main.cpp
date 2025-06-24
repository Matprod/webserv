/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 10:58:05 by Matprod           #+#    #+#             */
/*   Updated: 2025/06/24 18:13:41 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"

void printConfig(const Config& config) {
    const std::vector<ServerConfig>& servers = config.getServers();
    if (servers.empty()) {
        std::cout << "Aucun serveur configure." << std::endl;
        return;
    }

    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerConfig& server = servers[i];
        std::cout << "Serveur " << i + 1 << ":" << std::endl;
        std::cout << "  Port: " << server.port << std::endl;
        std::cout << "  Host: " << (server.host.empty() ? "INADDR_ANY" : server.host) << std::endl;
        std::cout << "  Server Names: ";
        for (size_t j = 0; j < server.server_names.size(); ++j) {
            std::cout << server.server_names[j];
            if (j < server.server_names.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
        std::cout << "  Root: " << server.root << std::endl;
        std::cout << "  Index: ";
        for (size_t j = 0; j < server.index.size(); ++j) {
            std::cout << server.index[j];
            if (j < server.index.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
        std::cout << "  Max Body Size: " << server.max_body_size << " bytes" << std::endl;
        std::cout << "  Error Pages: ";
        for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); it != server.error_pages.end(); ++it) {
            std::cout << it->first << " -> " << it->second << ", ";
        }
        std::cout << std::endl;

        std::cout << "  Locations:" << std::endl;
        for (size_t j = 0; j < server.locations.size(); ++j) {
			if (j >= 1)
				std::cout << std::endl;
            const LocationConfig& loc = server.locations[j];
            std::cout << "    Path: " << loc.path << std::endl;
            std::cout << "    Allow Methods: ";
            for (std::set<std::string>::const_iterator it = loc.allow_methods.begin(); it != loc.allow_methods.end(); ++it) {
                std::cout << *it << ", ";
            }
            std::cout << std::endl;
            std::cout << "    Root: " << loc.root << std::endl;
            std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
            std::cout << "    Index: ";
            for (size_t k = 0; k < loc.index.size(); ++k) {
                std::cout << loc.index[k];
                if (k < loc.index.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
            std::cout << "    Redirect Status: " << (loc.redirect_status) << std::endl; // À corriger avec stringstream (voir ci-dessous)
            std::cout << "    Redirect URL: " << (loc.redirect_url.empty() ? "none" : loc.redirect_url) << std::endl;
            std::cout << "    CGI Extensions: ";
            for (std::map<std::string, std::string>::const_iterator it = loc.cgi_extensions.begin(); it != loc.cgi_extensions.end(); ++it) {
                std::cout << it->first << " -> " << it->second << ", ";
            }
            std::cout << std::endl;
            std::cout << "    Upload Path: " << (loc.upload_path.empty() ? "none" : loc.upload_path) << std::endl;
            std::cout << "    Alias: " << (loc.alias.empty() ? "none" : loc.alias) << std::endl; // Nouvel affichage pour alias
        }
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[]) {
	std::string config_path = (argc > 1) ? argv[1] : "default.conf";
	Config config(config_path);

	std::cout << "Configuration parsee a partir de : " << config_path << std::endl;
	printConfig(config);

	return 0;
}

/* int main(int argc, char *argv[]) {
    std::string config_path = (argc > 1) ? argv[1] : "config/default.conf";
    Config config(config_path);

    // Créer le socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Erreur: socket" << std::endl;
        return 1;
    }
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configurer l'adresse
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config.getPort());

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Erreur: bind" << std::endl;
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Erreur: listen" << std::endl;
        close(server_fd);
        return 1;
    }

    // Initialiser poll
    struct pollfd fds[100];
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    int nfds = 1;

    std::cout << "Serveur en écoute sur le port " << config.getPort() << "..." << std::endl;

    while (true) {
        if (poll(fds, nfds, -1) < 0) {
            std::cerr << "Erreur: poll" << std::endl;
            break;
        }
        for (int i = 0; i < nfds; ++i) {
            if (fds[i].revents & POLLIN) {
                if (i == 0) { // Nouvelle connexion
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd >= 0) {
                        fcntl(client_fd, F_SETFL, O_NONBLOCK);
                        fds[nfds].fd = client_fd;
                        fds[nfds].events = POLLIN | POLLOUT;
                        nfds++;
                    }
                } else { // Données d'un client
                    char buffer[1024] = {0};
                    ssize_t bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes <= 0) {
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                    } else {
                        std::cout << "Requête: " << buffer << std::endl;
                        if (fds[i].revents & POLLOUT) {
                            std::string response = "HTTP/1.1 200 OK\r\n"
                                                  "Content-Type: text/html\r\n"
                                                  "Content-Length: 13\r\n"
                                                  "\r\n"
                                                  "<h1>Hello!</h1>";
                            send(fds[i].fd, response.c_str(), response.length(), 0);
                            close(fds[i].fd);
                            fds[i] = fds[nfds - 1];
                            nfds--;
                            i--;
                        }
                    }
                }
            }
        }
    }
    close(server_fd);
    return 0;
} */

/* int main() {
	// Créer un socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cerr << "Erreur: socket creation failed" << std::endl;
		return 1;
	}

	// Configurer le socket en mode non bloquant
	fcntl(server_fd, F_SETFL, O_NONBLOCK);

	// Activer l'option SO_REUSEADDR
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// Configurer l'adresse du serveur
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // Écoute sur toutes les interfaces
	address.sin_port = htons(8080);	   // Port 8080

	// Lier le socket
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		std::cerr << "Erreur: bind failed" << std::endl;
		close(server_fd);
		return 1;
	}

	// Mettre le socket en mode écoute
	if (listen(server_fd, 10) < 0) {
		std::cerr << "Erreur: listen failed" << std::endl;
		close(server_fd);
		return 1;
	}

	std::cout << "Serveur en écoute sur le port 8080..." << std::endl;

	// Boucle principale
	while (true) {
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) {
			continue; // Ignorer si non bloquant
		}

		// Lire la requête
		char buffer[1024] = {0};
		ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
		if (bytes_read > 0) {
			std::cout << "Requête reçue : " << buffer << std::endl;

			// Réponse HTTP simple
			std::string response = "HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html\r\n"
									"Content-Length: 13\r\n"
									"\r\n"
									"<h1>Hello!</h1>";
			send(client_fd, response.c_str(), response.length(), 0);
		}

		// Fermer la connexion client
		close(client_fd);
	}

	// Fermer le socket serveur (jamais atteint dans cet exemple)
	close(server_fd);
	return 0;
} */