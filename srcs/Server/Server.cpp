/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:29:23 by allan             #+#    #+#             */
/*   Updated: 2025/07/15 13:56:25 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//Handle Server Connection Request MAIN LOOP

int serverLoop(const std::vector<ServerConfig>& servers) {
	std::vector<pollfd> fds;
	std::map<int, bool> isServerFd;
	for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
		if (it->socketFd > 0) {
			pollfd newServerFd;
			newServerFd.fd = it->socketFd;	
			newServerFd.events = POLLIN;	
			newServerFd.revents = 0;	
			fds.push_back(newServerFd);	//{serverFd, EVENT (What to look for), REVENT (What actually happened)} 
			isServerFd[it->socketFd] = true;
			//POLLIN means: We are waiting for the Fd to be ready to listen. Means two diff things depending on the fd:
			//Server Fd: A new Client is trying to connect (use accept())
			//Client Fd: There is data from the client waiting to be read (use recv())
		}
	}

	
	while (true) {
	    // Step 1: Wait for activity (using select/poll/epoll/kqueue)
		int ready = poll(fds.data(), fds.size(), -1); //-1 = Poll wait indefinitly for an Event
		if (ready < 0) {
			perror("poll");
			continue; //Handle Error
		}
		
		for (int i = 0; i < static_cast<int>(fds.size()); ++i) {
			if (fds[i].revents & POLLIN) {
				
	    		// Step 2: If it's a listening socket => accept a new connection
				if (isServerFd[fds[i].fd] == true) {
					int clientFd = accept(fds[i].fd, 0, 0); // ClientFd is used to read/write data from/to client
					if (clientFd < 0)
						continue ;	//Handle Error
					else {
						pollfd newClientFd;
						newClientFd.fd = fds[i].fd;	
						newClientFd.events = POLLIN;	
						newClientFd.revents = 0;	
						fds.push_back(newClientFd);	//{serverFd, EVENT (What to look for), REVENT (What actually happened)} 
						isServerFd[clientFd] = false;
						std::cout << "New Client Socket Created: " << clientFd << std::endl;
					}
				}
				
	    		// Step 3: If it's a client socket => read request data
				else {
					char buffer[4096];
					ssize_t bytesRead = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
					std::cout << "New Data Received" << std::endl;
					if (bytesRead <= 0) {
						std::cout << "Client Fd closing: " << fds[i].fd << std::endl;
						close(fds[i].fd);
						isServerFd.erase(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;	
					} else {
						buffer[bytesRead] = '\0';

					// Step 4: Parse request GET - POST - DELETE
					// Step 5: Build and send response
					
					// Step 6: Close connection (if necessary)
						std::cout << "Client Fd closing: " << fds[i].fd << std::endl;
						close(fds[i].fd);
						isServerFd.erase(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;	
						}
				}
			}
		
		}
	}
}
