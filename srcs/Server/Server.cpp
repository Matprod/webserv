/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:29:23 by allan             #+#    #+#             */
/*   Updated: 2025/07/16 19:51:50 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void setup_pollfds(const std::vector<ServerConfig>& servers, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd) {
	for (size_t i = 0; i < servers.size(); ++i) {
		if (servers[i].socketFd > 0) {
			pollfd pfd;
			pfd.fd = servers[i].socketFd;
			pfd.events = POLLIN;
			pfd.revents = 0;
			fds.push_back(pfd);
			isServerFd[servers[i].socketFd] = true;
		}
	}
}

void check_timeouts(std::vector<pollfd>& fds,
	std::map<int, time_t>& lastActivity,
	std::map<int, std::string>& clientBuffers,
	std::map<int, bool>& isServerFd) {

	time_t now = time(NULL);
	for (size_t i = 0; i < fds.size(); ) {
		int fd = fds[i].fd;
		if (!isServerFd[fd]) {
			if (now - lastActivity[fd] > CLIENT_TIMEOUT) {
				std::cout << "Client " << fd << " inactif, fermeture.\n";
				close_client(fd, fds, isServerFd, clientBuffers, lastActivity);
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

	setup_pollfds(servers, fds, isServerFd);

	while (true) {
		check_timeouts(fds, lastActivity, clientBuffers, isServerFd);

		int ready = poll(fds.data(), fds.size(), -1);
		if (ready < 0) {
			perror("poll");
			continue;
		}

		for (int i = 0; i < static_cast<int>(fds.size()); ++i) {
			if (fds[i].revents & POLLIN) {
				if (isServerFd[fds[i].fd]) {
					handle_new_connection(fds[i].fd, fds, isServerFd, lastActivity);
				}
				else {
					Request req = handle_client_request(fds[i].fd, fds, i, isServerFd, clientBuffers, lastActivity);
					Response res = buildResponse(req);
					std::string rawResponse = res.responseToString();
					send(fds[i].fd, rawResponse.c_str(), rawResponse.size(), 0);
				}
			}
		}
	}
}
