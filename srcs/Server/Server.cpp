/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:29:23 by allan             #+#    #+#             */
/*   Updated: 2025/10/18 14:59:42 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "../Cgi/CgiExecutor.hpp"
#include "../Response/Response.hpp"
#include "../Response/ResponseUtils.hpp"
#include "../Config/Config.hpp"
#include "../Request/Request.hpp"
#include <poll.h>
#include <map>
#include <iostream>
#include <ctime>
#include <unistd.h>

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

void serverLoop(const std::vector<ServerConfig>& servers) {
	std::vector<pollfd> fds;
	std::map<int, bool> isServerFd;
	std::map<int, std::string> clientBuffers;
	std::map<int, time_t> lastActivity;
	std::map<int, const ServerConfig*> pollFdToServerConfig;
	std::map<int, const ServerConfig*> clientFdToServerConfig;

	setup_pollfds(servers, fds, isServerFd, pollFdToServerConfig);

	while (true && !g_stop) {
		check_timeouts(fds, lastActivity, clientBuffers, isServerFd, clientFdToServerConfig);
		
		// Nettoyer les processus CGI zombies
		checkCgiProcesses();

		int ready = poll(fds.data(), fds.size(), 100); // Timeout de 100ms pour rester réactif
		if (ready < 0) {
			if (errno == EINTR) {
				if (g_stop) break;
				continue;
			}
			perror("poll");
			continue;
		}
		
		if (ready == 0) {
			// Timeout, continuer la boucle
			continue;
		}

		for (int i = 0; i < static_cast<int>(fds.size()); ++i) {
			// Vérifier si le client a fermé la connexion (POLLHUP) ou erreur (POLLERR)
			if ((fds[i].revents & (POLLHUP | POLLERR)) && !isServerFd[fds[i].fd]) {
				close_client(fds[i].fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
				--i;
				continue;
			}
			
			if (fds[i].revents & POLLIN) {
				if (isServerFd[fds[i].fd]) {
					const ServerConfig *config = pollFdToServerConfig[fds[i].fd];
					if (!config) {
						std::cerr << "Error Poll" << std::endl;
						exit(1);
					}
					handle_new_connection(fds[i].fd, config, fds, isServerFd, lastActivity, clientFdToServerConfig);
				}
				else {
					Request req;
					req.config = clientFdToServerConfig[fds[i].fd];
					if (!req.config) {
						std::cerr << "Error Poll" << std::endl;
						exit(1);
					}
				 	int parse_status = handle_client_request(fds[i].fd, fds, i, isServerFd, clientBuffers, lastActivity, req, clientFdToServerConfig);
					//std::cout << "REQUEST AFTER PARSER:\n" << req << std::endl;
					if (parse_status == REQUEST_OK) {
						// Traitement normal (CGI ou non-CGI)
						Response res = buildResponse(req);
						std::string rawResponse = res.responseToString();
						std::cout << "RESPONSE:\n" << rawResponse << std::endl;
						int result;
						result = send(fds[i].fd, rawResponse.c_str(), rawResponse.size(), 0); //THE ONLY SEND FOR EACH CLIENT
						if (result <= 0) {
							std::cout << "Error Sending response" << std::endl;
							close_client(fds[i].fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
							--i;
							continue;
						}
						
						// Mettre à jour le timestamp d'activité
						lastActivity[fds[i].fd] = time(NULL);
						
						if (res.closingConnection == true) {
							close_client(fds[i].fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
							--i;
							continue;
						}
					}
					else if (parse_status == ERROR_MAX_BODY_LENGTH) {
						Response res;
						res.createResponse(413, "", req.config->error_pages);
						std::string rawResponse = res.responseToString();
						std::cout << "RESPONSE:\n" << rawResponse << std::endl;
						int result;
						result = send(fds[i].fd, rawResponse.c_str(), rawResponse.size(), 0); //THE ONLY SEND FOR EACH CLIENT
						if (result <= 0)
							std::cout << "Error Sending response" << std::endl;
						close_client(fds[i].fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
						--i;
						continue;
					}
					else if (parse_status == REQUEST_INCOMPLETE) {
						// WAITING
						continue;
					}
					else if (parse_status == REQUEST_ERROR) {
						// CLOSED
						continue;
					}
				}
			}
		}
	}
	for (size_t i = 0; i < fds.size(); ++i) {
		std::cout << "Closing Fd:\t" << toString<int>(fds[i].fd) << std::endl;
		close(fds[i].fd);
	}
}
