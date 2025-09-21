/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:29:23 by allan             #+#    #+#             */
/*   Updated: 2025/09/21 16:59:34 by adebert          ###   ########.fr       */
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

	while (true && !g_stop) {
		check_timeouts(fds, lastActivity, clientBuffers, isServerFd, clientFdToServerConfig);

		int ready = poll(fds.data(), fds.size(), -1);
		if (ready < 0) {
			perror("poll");
			continue;
		}

		for (int i = 0; i < static_cast<int>(fds.size()); ++i) {
			if (fds[i].revents & POLLIN) {
				if (isServerFd[fds[i].fd]) {
					const ServerConfig *config = pollFdToServerConfig[fds[i].fd];
					if (!config) {
						std::cerr << "ERROR POINTER 1" << std::endl;
						exit(1);
					}
					handle_new_connection(fds[i].fd, config, fds, isServerFd, lastActivity, clientFdToServerConfig);
				}
				else {
					Request req;
					req.config = clientFdToServerConfig[fds[i].fd];
					if (!req.config) {
						std::cerr << "ERROR POINTER 1" << std::endl;
						exit(1);
					}
				 	int parse_status = handle_client_request(fds[i].fd, fds, i, isServerFd, clientBuffers, lastActivity, req, clientFdToServerConfig);
					//std::cout << "REQUEST AFTER PARSER:\n" << req << std::endl;
					if (parse_status == REQUEST_OK) {
						Response res = buildResponse(req, servers);
						std::string rawResponse = res.responseToString();
						std::cout << "RESPONSE:\n" << rawResponse << std::endl;
						send(fds[i].fd, rawResponse.c_str(), rawResponse.size(), 0); //THE ONLY SEND FOR EACH CLIENT
						if (res.closingConnection == true) {
							close_client(fds[i].fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
							--i;
							continue;
						}
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
		if (g_stop) {
			std::cout << "Arrêt du serveur, fermeture des sockets... ligne " << __LINE__ << std::endl;
			for (size_t i = 0; i < fds.size(); ++i) {
				close(fds[i].fd);
			}
			fds.clear();
		}
	}
}
