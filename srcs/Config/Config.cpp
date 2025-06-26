/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 11:00:25 by Matprod           #+#    #+#             */
/*   Updated: 2025/06/26 21:17:22 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

LocationConfig::LocationConfig() : autoindex(false), redirect_status(0) {}
LocationConfig::~LocationConfig() {}

ServerConfig::ServerConfig() : port(8080), max_body_size(1048576) {}
ServerConfig::~ServerConfig() {}

Config::Config(const std::string& path) {
	this->error = 0;
    if (parseFile(path) == ERROR)
		this->error = 1;
}
Config::~Config() {}

std::string Config::trim(const std::string& str) const {
	size_t first = str.find_first_not_of(" \t\n");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t\n");
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
	return tokens;
}

unsigned long Config::parseSize(const std::string& size_str) const {
	std::string num_str = size_str;
	char unit = '\0';
	if (!num_str.empty() && isalpha(num_str[num_str.length() - 1])) {
		unit = toupper(num_str[num_str.length() - 1]);
		num_str.erase(num_str.length() - 1);
	}
	char* endptr;
	unsigned long num = strtoul(num_str.c_str(), &endptr, 10);
	if (*endptr != '\0' || num_str.empty()) {
		std::cout << "Invalid size format: " << size_str << std::endl;
		 return(ERROR);
	}
	if (unit == 'K') num *= 1024;
	else if (unit == 'M') num *= 1024 * 1024;
	else if (unit == 'G') num *= 1024 * 1024 * 1024;
	else if (unit != '\0') {
		std::cout << "Invalid size unit: " << size_str << std::endl;
		 return(ERROR);
	}
	return num;
}

bool Config::parseDirective(const std::vector<std::string>& tokens, ServerConfig* current_server, LocationConfig* current_location) {
    if (tokens.empty())
	{
		std::cout << "No directive in config" << std::endl;
		return(ERROR);
	}
    std::string directive = tokens[0];
    std::vector<std::string> values;
    for (size_t i = 1; i < tokens.size(); ++i) {
        values.push_back(tokens[i]);
    }
    if (directive == "}")
		return (0);
    if (current_location) {
        if (directive == "allow_methods") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_location->allow_methods.insert(values[i]);
            }
        } else if (directive == "root") {
            if (values.size() == 1) {
                if (!current_location->root.empty()) {
                    std::cout << "Duplicate root directive in location " << current_location->path << std::endl;
                    return(ERROR) ;
                }
                current_location->root = values[0];
            } else {
                std::cout << "Invalid root directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "autoindex") {
            if (values.size() == 1 && values[0] == "on") current_location->autoindex = true;
            else if (values.size() == 1 && values[0] == "off") current_location->autoindex = false;
            else {
                std::cout << "Invalid autoindex directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "index") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_location->index.push_back(values[i]);
            }
        } else if (directive == "cgi_extension") {
            if (values.size() == 2) {
                current_location->cgi_extensions[values[0]] = values[1];
            } else {
                std::cout << "Invalid cgi_extension directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "upload_path") {
            if (values.size() == 1) current_location->upload_path = values[0];
            else {
                std::cout << "Invalid upload_path directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "return") {
            if (values.size() == 2) {
                char* endptr;
                int status = strtol(values[0].c_str(), &endptr, 10);
                if (*endptr != '\0' || status < 300 || status > 399) {
                    std::cout << "Invalid return status code: " << values[0] << std::endl;
                     return(ERROR) ;
                }
                current_location->redirect_status = status;
                current_location->redirect_url = values[1];
            } else {
                std::cout << "Invalid return directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "alias") {
            if (values.size() == 1) {
                current_location->alias = values[0];
            } else {
                std::cout << "Invalid alias directive" << std::endl;
                 return(ERROR) ;
            }
        } else {
            std::cout << "Unknown location directive: " << directive << std::endl;
             return(ERROR) ;
        }
    } else if (current_server) {
        if (directive == "listen") {
            if (values.size() == 1) {
                std::string value = values[0];
                size_t colon_pos = value.find(':');
                if (colon_pos != std::string::npos) {
                    current_server->host = value.substr(0, colon_pos);
                    char* endptr;
                    current_server->port = strtol(value.substr(colon_pos + 1).c_str(), &endptr, 10);
                    if (*endptr != '\0') {
                        std::cout << "Invalid port in listen directive" << std::endl;
                         return(ERROR) ;
                    }
                } else {
                    char* endptr;
                    current_server->port = strtol(value.c_str(), &endptr, 10);
                    if (*endptr != '\0') {
                        std::cout << "Invalid port in listen directive" << std::endl;
                         return(ERROR) ;
                    }
                }
            } else {
                std::cout << "Invalid listen directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "server_name") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_server->server_names.push_back(values[i]);
            }
        } else if (directive == "root") {
            if (values.size() == 1) {
                if (!current_server->root.empty()) {
                    std::cout << "Duplicate root directive in server" << std::endl;
                     return(ERROR) ;
                }
                current_server->root = values[0];
            } else {
                std::cout << "Invalid root directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "error_page") {
            if (values.size() >= 2) {
                char* endptr;
                int code = strtol(values[0].c_str(), &endptr, 10);
                if (*endptr != '\0') {
                    std::cout << "Invalid error_page directive" << std::endl;
                     return(ERROR) ;
                }
                current_server->error_pages[code] = values[1];
            } else {
                std::cout << "Invalid error_page directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "client_max_body_size") {
            if (values.size() == 1) {
                current_server->max_body_size = parseSize(values[0]);
            } else {
                std::cout << "Invalid client_max_body_size directive" << std::endl;
                 return(ERROR) ;
            }
        } else if (directive == "index") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_server->index.push_back(values[i]);
            }
        } else {
            std::cout << "Unknown server directive: " << directive << std::endl;
             return(ERROR) ;
        }
    } else {
        std::cout << "Directive outside server block: " << tokens[0] << std::endl;
        return(ERROR) ;
    }
	return (0);
}

bool Config::parseFile(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cout << "Unable to open config file: " << path << std::endl;
         return ERROR;
    }

    std::string line;
    ServerConfig* current_server = NULL;
    LocationConfig* current_location = NULL;
    std::vector<std::string> current_location_paths; // Pour suivre les paths dans le server actuel

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.empty()) continue;

        if (tokens[0] == "server") {
            servers.push_back(ServerConfig());
            current_server = &servers.back();
            current_location = NULL;
            current_location_paths.clear(); // Réinitialiser les paths pour un nouveau server
        } else if (tokens[0] == "location" && tokens.size() >= 2) {
            if (!current_server) {
                std::cout << "Directive outside server block: location" << std::endl;
                 return ERROR;
            }
            current_server->locations.push_back(LocationConfig());
            current_location = &current_server->locations.back();
            current_location->path = tokens[1];
            // Vérification des doublons de path
            if (std::find(current_location_paths.begin(), current_location_paths.end(), current_location->path) != current_location_paths.end()) {
                std::cout << "Duplicate location path: " << current_location->path << " in server" << std::endl;
                return ERROR;
            }
            current_location_paths.push_back(current_location->path);
        } else if (current_location) {
            if (parseDirective(tokens, NULL, current_location) == ERROR){
				this->error = ERROR;
				return ERROR;
			}
        } else if (current_server) {
            if (parseDirective(tokens, current_server, NULL) == ERROR){
				this->error = ERROR;
				return ERROR;
			}
        } else {
            std::cout << "Directive outside server block: " << tokens[0] << std::endl;
            return ERROR;
        }
    }

    file.close();
    if (servers.empty()) {
        std::cout << "No server block found in config file" << std::endl;
         return ERROR;
    }
	return (0);
}