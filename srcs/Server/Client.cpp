/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:42:06 by allan             #+#    #+#             */
/*   Updated: 2025/09/28 16:34:51 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

void handle_new_connection(int serverFd, const ServerConfig* config, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, time_t>& lastActivity, std::map<int, const ServerConfig*>& clientFdToServerConfig) {
	for (;;) {
    	sockaddr_in sa; socklen_t sl = sizeof(sa);
    	int cfd = accept (serverFd, (sockaddr*)&sa, &sl);
    	if (cfd < 0) {
    	    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
    	    if (errno == EINTR) continue;
    	    perror("accept");
    	    break;
    	}
	
    	// Make client non-blocking + cloexec
    	int fl = fcntl(cfd, F_GETFL, 0);
    	if (fl == -1 || fcntl(cfd, F_SETFL, fl | O_NONBLOCK) == -1) {
    	    perror("fcntl O_NONBLOCK (client)");
    	    close(cfd);
    	    continue;
    	}
    	int fdfl = fcntl(cfd, F_GETFD, 0);
    	if (fdfl != -1) fcntl(cfd, F_SETFD, fdfl | FD_CLOEXEC);
	
    	pollfd p;
		p.fd = cfd;
		p.events = POLLIN;
		p.revents = 0;

    	fds.push_back(p);
    	isServerFd[cfd] = false;
    	lastActivity[cfd] = time(NULL);
    	clientFdToServerConfig[cfd] = config;
	}
}

void close_client(int fd, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, std::string>& clientBuffers, std::map<int, time_t>& lastActivity, std::map<int, const ServerConfig*>& clientFdToServerConfig) {
	std::cout << "Fermeture client " << fd << std::endl;
	close(fd);
	clientBuffers.erase(fd);
	isServerFd.erase(fd);
	lastActivity.erase(fd);
	clientFdToServerConfig.erase(fd);
	for (size_t i = 0; i < fds.size(); ++i) {
		if (fds[i].fd == fd) {
			fds.erase(fds.begin() + i);
			break;
		}
	}
}

int handle_client_request(int fd, std::vector<pollfd>& fds, int& i,
	std::map<int, bool>& isServerFd,
	std::map<int, std::string>& clientBuffers,
	std::map<int, time_t>& lastActivity,
	Request& req, std::map<int, const ServerConfig*>& clientFdToServerConfig)
{
	//std::cout << "Parsing request" << std::endl;
	int result = parse_request(fd, req, clientBuffers, lastActivity);
	if (result == ERROR_MAX_BODY_LENGTH)
	{
		std::cerr << "Error with max_body\n";
		close_client(fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig); //Here for 
		--i;
		return REQUEST_ERROR;
	}
	if (result == REQUEST_ERROR) {
		std::cerr << "Error of parsing for the request\n";
		close_client(fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig); //Here for 
		--i;
		return REQUEST_ERROR;
	}
	else if (result == REQUEST_OK) {
		std::cout << "Request received!" << std::endl;
		std::cout << "Method : " << req.method << "\n";
		std::cout << "URI : " << req.uri << "\n";
		std::cout << "Version : " << req.version << "\n";

		std::string connection_header = to_lower(req.headers["connection"]);
		if (req.version == "HTTP/1.1" && connection_header != "close")
			req.closeConnection = false;
		else
			req.closeConnection = true;
/* 
		//JE CROIS ON DOIT PAS FERMER MAINTENANT
		if (!keepAlive) {
			close_client(fd, fds, isServerFd, clientBuffers, lastActivity, clientFdToServerConfig);
			--i;
		}
		else
			std::cout << "Connexion in keep alive" << std::endl; */
	}
	else if (result == REQUEST_INCOMPLETE) {
		//std::cout << "Incomplete request..\n";
	}
	return result;
}
