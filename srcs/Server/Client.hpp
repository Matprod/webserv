/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 15:12:35 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/16 19:51:39 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"

void handle_new_connection(int serverFd, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, time_t>& lastActivity);
void close_client(int fd, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, std::string>& clientBuffers, std::map<int, time_t>& lastActivity);
Request handle_client_request(int fd, std::vector<pollfd>& fds, int& i,
	std::map<int, bool>& isServerFd,
	std::map<int, std::string>& clientBuffers,
	std::map<int, time_t>& lastActivity);
