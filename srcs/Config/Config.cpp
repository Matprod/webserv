/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 11:00:25 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/11 16:48:19 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

LocationConfig::LocationConfig() : autoindex(false), redirect_status(0), has_root(false), has_alias(false) {}
LocationConfig::~LocationConfig() {}

LocationConfig::LocationConfig(const LocationConfig& src) {
	*this = src;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& rhs) {
	if (this != &rhs) {
		this->root = rhs.root;
		this->alias = rhs.alias;
		this->has_alias = rhs.has_alias;
		this->allow_methods = rhs.allow_methods;
		this->autoindex = rhs.autoindex;
		
		this->path = rhs.path;
		this->upload_path = rhs.upload_path;
		this->cgi_extensions = rhs.cgi_extensions;
		this->index = rhs.index;
		this->redirect_status = rhs.redirect_status;
		this->redirect_url = rhs.redirect_url;
	}	
	
	return *this;
}

ServerConfig::ServerConfig() : port(8080), max_body_size(1048576), socketFd(0), has_root(false), autoindex(false){}

ServerConfig::~ServerConfig() {
	if (socketFd > 0)
		close (socketFd);
}

Config::Config(const std::string& path) {
	this->error = 0;
	if (parseFile(path) == ERROR)
		this->error = 1;	
	if (serversHaveRoot() == ERROR)
		this->error = 1;
}

Config::~Config() {}

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
