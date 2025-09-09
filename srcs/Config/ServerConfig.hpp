/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:09:14 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/09 14:02:14 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "LocationConfig.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>

class ServerConfig {
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
	int ServerLoop(std::vector<ServerConfig>& servers);//Need to be implemented? 
	
	ServerConfig();//Need to be implemented?
	virtual ~ServerConfig();//Need to be implemented?
};

std::ostream &operator<<(std::ostream &o, const ServerConfig&i);