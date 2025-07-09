/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:29:23 by allan             #+#    #+#             */
/*   Updated: 2025/07/09 15:04:58 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//Handle Server Connection Request MAIN LOOP

int ServerLoop(const std::vector<ServerConfig>& servers) {
	while (true) {
	    // Step 1: Wait for activity (using select/poll/epoll/kqueue)
	    // Step 2: If it's a listening socket => accept a new connection
	    // Step 3: If it's a client socket => read request data
	    // Step 4: Parse request
	    // Step 5: Build and send response
	    // Step 6: Close connection (if necessary)
	}
}
