/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 13:55:38 by allan             #+#    #+#             */
/*   Updated: 2025/07/09 18:02:09 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

void setupSockets(std::vector<ServerConfig>& config) {
	for (std::vector<ServerConfig>::iterator it = config.begin(); it != config.end(); ++it) {
		//STEP 1: Create the Socket
		int socketFd = socket(AF_INET, SOCK_STREAM, 0);		//Will be use for an IPV4 addr (AF_INET), using TCP (SOCK_STREAM)
		if (socketFd < 0) {
			std::cerr << "Error: Socket Creation Failed" << std::endl;
			continue;
		}

		//STEP 2: Securize the reusability of the socket: If a crash occurs and an IP address was recently bound, makes it available
		int opt = 1;
		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			std::cerr << "Error: setsockopt Failed" << std::endl;
			close(socketFd);
			continue;
		}
		
		//STEP 3: Prepare a Snippet for IPV4 Address Socket
		struct sockaddr_in addrIpv4;
		memset(&addrIpv4, 0, sizeof(addrIpv4)); 	//Initialize the struct to 0
		addrIpv4.sin_family = AF_INET; 				//Precise the address type: AF_INET == IPV4
		addrIpv4.sin_port = htons(it->port); 		//Set the Port Number, while transforming its bytes ordering from little-endian to big-endian;
		addrIpv4.sin_addr.s_addr = INADDR_ANY;  	//INADDR_ANY == Can bind to any available network Interface

		
		//STEP 4: Binds our Socket to a Port (IPV4 Address)
		if (bind(socketFd, (struct sockaddr*)&addrIpv4, sizeof(addrIpv4)) < 0) {
			std::cerr << "Error: Socket Binding Failed" << std::endl;
			close(socketFd);
			continue;
		}

		//STEP 5: Start Listening (Mark a socket as passive, ready to accept incoming TCP connections), SOMAXCON = Max Number of pending connection request allowed
		if (listen(socketFd, SOMAXCONN) < 0) {
			std::cerr << "Error: Listen Setup Failed" << std::endl;
			close(socketFd);
			continue;
		}
		
		it->socketFd = socketFd;	
	}
}