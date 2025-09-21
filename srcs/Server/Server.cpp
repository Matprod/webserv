/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:29:23 by allan             #+#    #+#             */
/*   Updated: 2025/09/21 16:05:12 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void setup_pollfds(const std::vector<ServerConfig>& servers, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, const ServerConfig*> &pollFdToServerConfig) {
	for (size_t i = 0; i < servers.size(); ++i) {
		if (servers[i].socketFd >= 0) {
			pollfd pfd = {};
			pfd.fd = servers[i].socketFd;
			pfd.events = POLLIN;
			pfd.revents = 0;
			fds.push_back(pfd);
			isServerFd[servers[i].socketFd] = true;
			pollFdToServerConfig[servers[i].socketFd] = &servers[i];
		}
	}
}

void check_timeouts(std::vector<pollfd>& fds,
	std::map<int, time_t>& lastActivity,
	std::map<int, std::string>& clientBuffers,
	std::map<int, bool>& isServerFd,
	std::map<int, const ServerConfig*>& clientFdToServerConfig) {

	time_t now = time(NULL);
	for (size_t i = 0; i < fds.size(); ) {
		int fd = fds[i].fd;
		if (isServerFd.find(fd) != isServerFd.end() && !isServerFd[fd]) {
			std::map<int, time_t>::iterator it = lastActivity.find(fd);
            if (it != lastActivity.end() && now - it->second > CLIENT_TIMEOUT) {
                std::cout << "Client " << fd << " inactive, closing.\n";
                close_client(fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
                continue;
			}
		}
		++i;
	}
}

int serverLoop(const std::vector<ServerConfig>& servers) {
	std::vector<pollfd> fds;
	std::map<int, bool> isServerFd;
	std::map<int, std::string> clientBuffers;
	std::map<int, time_t> lastActivity;
	std::map<int, const ServerConfig*> pollFdToServerConfig;
	std::map<int, const ServerConfig*> clientFdToServerConfig;

	setup_pollfds(servers, fds, isServerFd, pollFdToServerConfig);
	std::cout << "[DEBUG] Après setup_pollfds, ligne " << __LINE__ << std::endl;

	while (true && !g_stop) {
		std::cout << "[DEBUG] Début boucle principale, ligne " << __LINE__ << std::endl;
		check_timeouts(fds, lastActivity, clientBuffers, isServerFd, clientFdToServerConfig);
		std::cout << "[DEBUG] Après check_timeouts, ligne " << __LINE__ << std::endl;

		int ready = poll(fds.data(), fds.size(), -1);
		std::cout << "[DEBUG] Après poll, ready=" << ready << ", ligne " << __LINE__ << std::endl;
		if (ready < 0) {
			perror("poll");
			std::cout << "[DEBUG] poll < 0, continue, ligne " << __LINE__ << std::endl;
			continue;
		}

		for (int i = 0; i < static_cast<int>(fds.size()); ) {
			std::cout << "[DEBUG] Boucle fd, i=" << i << ", fd=" << fds[i].fd << ", revents=" << fds[i].revents << ", ligne " << __LINE__ << std::endl;
			if (fds[i].revents & POLLIN) {
				std::cout << "[DEBUG] POLLIN détecté, fd=" << fds[i].fd << ", ligne " << __LINE__ << std::endl;
				if (isServerFd[fds[i].fd]) {
					std::cout << "[DEBUG] fd est un serverFd, ligne " << __LINE__ << std::endl;
					const ServerConfig *config = pollFdToServerConfig[fds[i].fd];
					if (!config) {
						std::cerr << "ERROR POINTER 1, ligne " << __LINE__ << std::endl;
						exit(1);
					}
					std::cout << "[DEBUG] handle_new_connection, fd=" << fds[i].fd << ", ligne " << __LINE__ << std::endl;
					handle_new_connection(fds[i].fd, config, fds, isServerFd, lastActivity, clientFdToServerConfig);
				}
				else {
					std::cout << "[DEBUG] fd est un clientFd, ligne " << __LINE__ << std::endl;
					Request req;
					req.config = clientFdToServerConfig[fds[i].fd];
					if (!req.config) {
						std::cerr << "ERROR POINTER 1, ligne " << __LINE__ << std::endl;
						exit(1);
					}
					std::cout << "[DEBUG] handle_client_request, fd=" << fds[i].fd << ", ligne " << __LINE__ << std::endl;
					int parse_status = handle_client_request(fds[i].fd, fds, i, isServerFd, clientBuffers, lastActivity, req, clientFdToServerConfig);
					if (parse_status == REQUEST_OK) {
						std::cout << "[DEBUG] REQUEST_OK, ligne " << __LINE__ << std::endl;
						Response res;
						res.closingConnection = true;
						res = buildResponse(req, servers);
						std::string rawResponse = res.responseToString();
						std::cout << "RESPONSE:\n" << rawResponse << std::endl;
						send(fds[i].fd, rawResponse.c_str(), rawResponse.size(), 0);
						if (res.closingConnection == true) {
							std::cout << "[DEBUG] closingConnection, close_client, fd=" << fds[i].fd << ", ligne " << __LINE__ << std::endl;
							close_client(fds[i].fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
							continue; // Ne pas incrémenter i, car le fd courant a été supprimé
						}
					}
					else if (parse_status == REQUEST_INCOMPLETE) {
						std::cout << "[DEBUG] REQUEST_INCOMPLETE, ligne " << __LINE__ << std::endl;
						++i;
						continue;
					}
					else if (parse_status == REQUEST_ERROR) {
						std::cout << "[DEBUG] REQUEST_ERROR, ligne " << __LINE__ << std::endl;
						++i;
						continue;
					}
				}
				// Réinitialiser les événements après traitement
				fds[i].revents = 0;
				++i;
			} else {
				++i;
			}
		}
		// Nettoyage des fd lors de l'arrêt par signal
		if (g_stop) {
			std::cout << "Arrêt du serveur, fermeture des sockets... ligne " << __LINE__ << std::endl;
			for (size_t i = 0; i < fds.size(); ++i) {
				close(fds[i].fd);
			}
			fds.clear();
		}
	}
}
