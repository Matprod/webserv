/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:09:14 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/11 16:00:07 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
class LocationConfig;

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
	
	std::set<std::string> allow_methods;
	int socketFd;
	
	bool has_root;
	bool autoindex;
	
	int ServerLoop(const std::vector<ServerConfig>& servers); 
	
	ServerConfig();
	virtual ~ServerConfig();
};

std::ostream &operator<<(std::ostream &o, const ServerConfig&i);