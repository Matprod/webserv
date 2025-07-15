/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:17:11 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/12 20:18:44 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

bool Config::parseDirective(const std::vector<std::string>& tokens, ServerConfig* current_server, LocationConfig* current_location) {
	if (tokens.empty()) {
		std::cerr << "Error: No directive in config" << std::endl;
		return ERROR;
	}

	std::string directive = tokens[0];
	std::vector<std::string> values(tokens.begin() + 1, tokens.end());

	if (directive == "}")
		return 0;

	if (current_location)
		return parseLocationDirective(directive, values, current_location);

	if (current_server)
		return parseServerDirective(directive, values, current_server);

	std::cerr << "Error: Directive outside server block: " << directive << std::endl;
	return ERROR;
}

unsigned long Config::parseSize(const std::string& size_str) const {
	std::string num_str = size_str;
	char unit = '\0';
	size_t valid_end = num_str.find_last_not_of("0123456789KMG");
    if (valid_end != std::string::npos && valid_end + 1 < num_str.length()) {
        num_str = num_str.substr(0, valid_end + 1);
    }
	if (!num_str.empty() && isalpha(num_str[num_str.length() - 1])) {
        unit = toupper(num_str[num_str.length() - 1]);
        num_str.erase(num_str.length() - 1);
    }
	char* endptr;
	unsigned long num = strtoul(num_str.c_str(), &endptr, 10);
	if (*endptr != '\0' || num_str.empty()) {
		std::cout << "Error: Invalid size format: " << size_str << std::endl;
		return(ERROR);
	}
	if (unit == 'K') num *= 1024;
	else if (unit == 'M') num *= 1024 * 1024;
	else if (unit == 'G') num *= 1024 * 1024 * 1024;
	else if (unit != '\0') {
		std::cout << "Error: Invalid size unit: " << size_str << std::endl;
		 return(ERROR);
	}
	return num;
}


bool Config::parseFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		std::cerr << "Unable to open config file: " << path << std::endl;
		return ERROR;
	}

	std::string line;
	ServerConfig* current_server = NULL;
	LocationConfig* current_location = NULL;
	std::vector<std::string> current_location_paths;
	std::set<std::string> all_server_names;

	while (std::getline(file, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;

		std::vector<std::string> tokens = split(line, ' ');
		if (tokens.empty())
			continue;

		if (tokens[0] == "server") {
			if (current_server) { // Vérification des server_names du bloc précédent
				for (size_t i = 0; i < current_server->server_names.size(); ++i) {
					if (all_server_names.find(current_server->server_names[i]) != all_server_names.end()) {
						std::cerr << "Duplicate server_name: " << current_server->server_names[i] << " across multiple servers" << std::endl;
						return ERROR;
					}
					all_server_names.insert(current_server->server_names[i]);
				}
			}
		servers.push_back(ServerConfig());
		current_server = &servers.back();
		current_location = NULL;
		current_location_paths.clear();
		} else if (tokens[0] == "location" && tokens.size() >= 2) {
			if (!current_server) {
				std::cerr << "Directive outside server block: location" << std::endl;
				return ERROR;
			}
			current_server->locations.push_back(LocationConfig());
			current_location = &current_server->locations.back();
			current_location->path = tokens[1];
			if (std::find(current_location_paths.begin(), current_location_paths.end(), current_location->path) != current_location_paths.end()) {
				std::cerr << "Duplicate location path: " << current_location->path << " in server" << std::endl;
				return ERROR;
			}
		current_location_paths.push_back(current_location->path);
		} else if (current_location) {
			if (parseDirective(tokens, NULL, current_location) == ERROR) {
			return ERROR;
		}
		} else if (current_server) {
			if (parseDirective(tokens, current_server, NULL) == ERROR) {
			return ERROR;
		}
		} else {
			std::cerr << "Directive outside server block: " << tokens[0] << std::endl;
			return ERROR;
		}
	}

	if (current_server) {
		for (size_t i = 0; i < current_server->server_names.size(); ++i) {
			if (all_server_names.find(current_server->server_names[i]) != all_server_names.end()) {
				std::cerr << "Duplicate server_name: " << current_server->server_names[i] << " across multiple servers" << std::endl;
				return ERROR;
			}
			all_server_names.insert(current_server->server_names[i]);
		}
	}
	file.close();
	return (0);
}