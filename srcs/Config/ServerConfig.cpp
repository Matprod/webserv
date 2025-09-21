/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:07:43 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/21 17:10:00 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "Config.hpp"

static int is_valid_ipv4(const char *ip) {
	regex_t regex;
	int result;
	const char *ipv4_pattern =
	"^((25[0-5]|(2[0-4][0-9])|(1[0-9]{2})|([1-9]?[0-9]))\\.){3}"
	"(25[0-5]|(2[0-4][0-9])|(1[0-9]{2})|([1-9]?[0-9]))$";
	
	result = regcomp(&regex, ipv4_pattern, REG_EXTENDED);
	result = regexec(&regex, ip, 0, NULL, 0);
	if (result != 0)
		return(ERROR);
	regfree(&regex);
	return(SUCCESS);
}	

bool Config::parseServerDirective(const std::string& directive, const std::vector<std::string>& values, ServerConfig* srv) {
	if (directive == "allow_methods") {
		if (values.size() >= 1) {
			for (size_t i = 0; i < values.size(); ++i)
				srv->allow_methods.insert(values[i]);
		} else
			srv->allow_methods.insert("NONE");
	} else if (directive == "autoindex" || directive == "directory_listing") {
		if (values.size() != 1) {
			std::cerr << "Invalid " << directive << " directive" << std::endl;
			return ERROR;
		}
		if (values[0] == "on" || values[0] == "on;")
			srv->autoindex = true;
		else if (values[0] == "off" || values[0] == "off;")
			srv->autoindex = false;
		else {
			std::cerr << "Invalid " << directive << " directive value: " << values[0] << std::endl;
			return ERROR;
		}
	} else if (directive == "listen") {
		if (values.size() != 1) {
			std::cerr << "Invalid listen directive" << std::endl;
			return ERROR;
		}
		std::string value = values[0];
		std::string ip;
		std::string port_str;
		size_t colon_pos = value.find(':');

		if (colon_pos == std::string::npos) {
			std::vector<std::string> parts = split(value, '.');
			if (parts.size() == 4) {
				ip = value;
				port_str = DEFAULT_PORT;
			} else {
				port_str = value;
				ip = "";
			}
		} else {
			ip = value.substr(0, colon_pos);
			port_str = value.substr(colon_pos + 1);
		}

		if (!ip.empty()) {
			if (is_valid_ipv4(ip.c_str()) == ERROR) {
				std::cerr << "Invalid IP in listen directive: " << ip << std::endl;
				return ERROR;
			} else {
				srv->host = ip;
			}
		} else {
			srv->host = "INADDR_ANY";
		}

		char* endptr;
		srv->port = strtol(port_str.c_str(), &endptr, 10);
		if (*endptr != '\0' || port_str.empty() || srv->port < 0 || srv->port > 65535) {
			std::cerr << "Invalid port in listen directive: " << port_str << std::endl;
			return ERROR;
		}
	} else if (directive == "server_name") {
		for (size_t i = 0; i < values.size(); ++i)
			srv->server_names.push_back(values[i]);
	} else if (directive == "root") {
		if (values.size() != 1) {
			std::cerr << "Invalid root directive" << std::endl;
			return ERROR;
		}
		if (!srv->root.empty()) {
			std::cerr << "Error: Duplicate root directive in server" << std::endl;
			return ERROR;
		}
		srv->root = values[0];
		srv->has_root = true;
	} else if (directive == "error_page") {
		if (values.size() < 2) {
			std::cerr << "Invalid error_page directive" << std::endl;
			return ERROR;
		}
		char* endptr;
		int code = strtol(values[0].c_str(), &endptr, 10);
		if (*endptr != '\0') {
			std::cerr << "Error: Invalid error_page directive code" << std::endl;
			return ERROR;
		}
		srv->error_pages[code] = values[1];
	} else if (directive == "client_max_body_size") {
		if (values.size() != 1) {
			std::cerr << "Invalid client_max_body_size directive" << std::endl;
			return ERROR;
		}
		size_t size = parseSize(values[0]);
		if (size == ERROR) return ERROR;
		srv->max_body_size = size;
	} else if (directive == "index") {
		for (size_t i = 0; i < values.size(); ++i)
			srv->index.push_back(values[i]);
	} else {
		std::cerr << "Error: Unknown server directive: " << directive << std::endl;
		return ERROR;
	}
	return 0;
}


std::ostream &operator<<(std::ostream &o, const ServerConfig&i) {
	o << i.socketFd << std::endl
	<< i.root << std::endl
	<< i.port << std::endl;
	return o;	
}

ServerConfig::ServerConfig()
	: port(8080)
	, host("")
	, root("")
	, max_body_size(1048576)
	, socketFd(0)
	, has_root(false)
	, autoindex(false)
{
	server_names.clear();
	error_pages.clear();
	locations.clear();
	index.clear();
	allow_methods.clear();
}

ServerConfig::ServerConfig(const ServerConfig& src)
	: port(src.port)
	, host(src.host)
	, server_names(src.server_names)
	, root(src.root)
	, error_pages(src.error_pages)
	, max_body_size(src.max_body_size)
	, locations(src.locations)
	, index(src.index)
	, allow_methods(src.allow_methods)
	, socketFd(src.socketFd)
	, has_root(src.has_root)
	, autoindex(src.autoindex)
{
}

ServerConfig::~ServerConfig() {
	if (socketFd > 0)
		close (socketFd);
}

ServerConfig& ServerConfig::operator=(const ServerConfig& rhs) {
	if (this != &rhs) {
		port = rhs.port;
		host = rhs.host;
		server_names = rhs.server_names;
		root = rhs.root;
		error_pages = rhs.error_pages;
		max_body_size = rhs.max_body_size;
		locations = rhs.locations;
		index = rhs.index;
		allow_methods = rhs.allow_methods;
		socketFd = rhs.socketFd;
		has_root = rhs.has_root;
		autoindex = rhs.autoindex;
	}
	return *this;
}

