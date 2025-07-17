/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 15:12:35 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/17 19:02:39 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"

void handle_new_connection(int serverFd, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, time_t>& lastActivity);
void close_client(int fd, std::vector<pollfd>& fds, std::map<int, bool>& isServerFd, std::map<int, std::string>& clientBuffers, std::map<int, time_t>& lastActivity);
int handle_client_request(int fd, std::vector<pollfd>& fds, int& i,
	std::map<int, bool>& isServerFd,
	std::map<int, std::string>& clientBuffers,
	std::map<int, time_t>& lastActivity,
	Request& req);