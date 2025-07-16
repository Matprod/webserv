/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:42:06 by allan             #+#    #+#             */
/*   Updated: 2025/07/16 19:51:17 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

void handle_new_connection(int serverFd, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, time_t>& lastActivity) {
	int clientFd = accept(serverFd, 0, 0);
	if (clientFd >= 0) {
		pollfd clientPoll;
		clientPoll.fd = clientFd;
		clientPoll.events = POLLIN;
		clientPoll.revents = 0;
		fds.push_back(clientPoll);
		isServerFd[clientFd] = false;
		lastActivity[clientFd] = time(NULL);
		std::cout << "New Client Socket Created: " << clientFd << std::endl;
	}
}

void close_client(int fd, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, std::string>& clientBuffers, std::map<int, time_t>& lastActivity) {
	std::cout << "Fermeture client " << fd << std::endl;
	close(fd);
	clientBuffers.erase(fd);
	isServerFd.erase(fd);
	lastActivity.erase(fd);
	for (size_t i = 0; i < fds.size(); ++i) {
		if (fds[i].fd == fd) {
			fds.erase(fds.begin() + i);
			break;
		}
	}
}

Request handle_client_request(int fd, std::vector<pollfd>& fds, int& i,
	std::map<int, bool>& isServerFd,
	std::map<int, std::string>& clientBuffers,
	std::map<int, time_t>& lastActivity) {

	Request req;
	std::cout << "Parsing request" << std::endl;
	int result = parse_request(fd, req, clientBuffers, lastActivity);

	if (result == REQUEST_ERROR) {
		std::cerr << "Erreur de parsing de la requête.\n";
		close_client(fd, fds, isServerFd, clientBuffers, lastActivity);
		--i;
	}
	else if (result == REQUEST_OK) {
		std::cout << "Requête complète reçue !" << std::endl;
		std::cout << "Méthode : " << req.method << "\n";
		std::cout << "URI : " << req.uri << "\n";
		std::cout << "Version : " << req.version << "\n";

		std::string connection_header = to_lower(req.headers["connection"]);
		bool keepAlive = (req.version == "HTTP/1.1" && connection_header != "close");

		if (!keepAlive) {
			close_client(fd, fds, isServerFd, clientBuffers, lastActivity);
			--i;
		}
		else {
			std::cout << "Connexion maintenue en keep-alive\n";
			// keep taking request from the client
		}
	}
	else if (result == REQUEST_INCOMPLETE) {
		std::cout << "Requête incomplète, on attend la suite...\n";
		// buffer is saved
	}
	return req;
}