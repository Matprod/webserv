/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 11:00:25 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/18 18:12:50 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config()
	: error(false) {}
	
Config::Config(const std::string& path) {
	this->error = 0;
	if (parseFile(path) == ERROR)
		this->error = 1;
}
Config::~Config() {}

Config::Config(const Config& other)
	: servers(other.servers),
	  error(other.error) {}

Config& Config::operator=(const Config& other) {
	if (this != &other) {
		servers = other.servers;
		error = other.error;
	}
	return *this;
}

std::vector<ServerConfig>& Config::getServers() {
	return servers;
}

const std::vector<ServerConfig>& Config::getServers() const {
	return servers;
}

std::string Config::trim(const std::string& str) const {
	size_t first = str.find_first_not_of(" \t\n;");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\n;");
	return str.substr(first, last - first + 1);
}

std::vector<std::string> Config::split(const std::string& str, char delimiter) const {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter)) {
		token = trim(token);
		if (!token.empty()) {
			tokens.push_back(token);
		}
	}
	return(tokens);
}

