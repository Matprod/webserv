/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:29:23 by allan             #+#    #+#             */
/*   Updated: 2025/07/15 20:23:55 by Matprod          ###   ########.fr       */
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
	std::map<int, std::string> clientBuffers;
	std::map<int, time_t> lastActivity;
	while (true) {
		//Step 0 reset the timer for client
		time_t now = time(NULL);
		for (std::vector<pollfd>::iterator it = fds.begin(); it != fds.end(); ) {
			if (!isServerFd[it->fd]) { // only for clients
				time_t last = lastActivity[it->fd];
				if (now - last > CLIENT_TIMEOUT) { // timeout 
					std::cout << "Client " << it->fd << " inactif, fermeture.\n";
					close(it->fd);
					clientBuffers.erase(it->fd);
					lastActivity.erase(it->fd);
					isServerFd.erase(it->fd);
					it = fds.erase(it);
					continue;
				}
			}
			++it;
		}
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
					lastActivity[clientFd] = time(NULL);
					if (clientFd < 0)
						continue ;	//Handle Error
					else {
						pollfd newClientFd;
						newClientFd.fd = clientFd;	
						newClientFd.events = POLLIN;	
						newClientFd.revents = 0;	
						fds.push_back(newClientFd);	//{serverFd, EVENT (What to look for), REVENT (What actually happened)} 
						isServerFd[clientFd] = false;
						std::cout << "New Client Socket Created: " << clientFd << std::endl;
					}
				}
				else {
					Request req;
					std::cout << "Parsing request" << std::endl;
					int result = parse_request(fds[i].fd, req, clientBuffers, lastActivity);

					if (result == REQUEST_ERROR) {
						std::cerr << "Erreur de parsing de la requête. Fermeture de la connexion.\n";
						clientBuffers.erase(fds[i].fd);
						close(fds[i].fd);
						isServerFd.erase(fds[i].fd);
						fds.erase(fds.begin() + i);
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
							close(fds[i].fd);
							lastActivity.erase(fds[i].fd);
							isServerFd.erase(fds[i].fd);
							fds.erase(fds.begin() + i);
							--i;
						}
						else {
							std::cout << "Connexion maintenue en keep-alive\n";
							// Ne rien faire : on garde la socket ouverte
							// Le client pourra envoyer une nouvelle requête
}

					}
					else if (result == REQUEST_INCOMPLETE) {
						std::cout << "Requête incomplète, on attend la suite...\n";
						// Pour l’instant, on ferme la connexion, mais tu peux gérer un buffer persistant par client pour continuer la lecture plus tard
						/* clientBuffers.erase(fds[i].fd);
						close(fds[i].fd);
						isServerFd.erase(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i; */
					}
					// Step 5: Build and send response
					
					// Step 6: Close connection (if necessary)
						/* std::cout << "Client Fd closing: " << fds[i].fd << std::endl;
						close(fds[i].fd);
						isServerFd.erase(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i; */	
					
				}
			}
		
		}
	}
}
