/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:09:14 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/13 21:36:02 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "LocationConfig.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>

class ServerConfig { // Need to be in canonical form ?
public:
	int port;
	std::string host;
	std::vector<std::string> server_names;
	std::string root;
	std::map<int, std::string> error_pages;
	unsigned long max_body_size;
	std::vector<LocationConfig> locations;
	std::vector<std::string> index;
	
	int socketFd;
	int ServerLoop(const std::vector<ServerConfig>& servers);
	
	ServerConfig();
	virtual ~ServerConfig();
};
