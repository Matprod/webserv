/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:09:14 by Matprod           #+#    #+#             */
/*   Updated: 2025/10/18 16:33:44 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#define MAX_BODY_SIZE 1048576
class LocationConfig;

class ServerConfig {
public:
	int port;
	int countport;
	std::string host;
	std::vector<std::string> server_names;
	std::string root;
	std::map<int, std::string> error_pages;
	unsigned long max_body_size;
	std::vector<LocationConfig> locations;
	std::string index;
	std::string upload_path;
	
	std::set<std::string> allow_methods;
	int socketFd;
	
	bool has_root;
	bool autoindex;
	
	int ServerLoop(const std::vector<ServerConfig>& servers); 
	
	ServerConfig();
	ServerConfig(const ServerConfig& src);
	virtual ~ServerConfig();
	ServerConfig& operator=(const ServerConfig& rhs);
};

std::ostream &operator<<(std::ostream &o, const ServerConfig&i);