/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 11:00:25 by Matprod           #+#    #+#             */
/*   Updated: 2025/06/24 18:17:04 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

LocationConfig::LocationConfig() : autoindex(false), redirect_status(0) {}
LocationConfig::~LocationConfig() {}

ServerConfig::ServerConfig() : port(8080), max_body_size(1048576) {}
ServerConfig::~ServerConfig() {}

Config::Config(const std::string& path) {
    parseFile(path);
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
		std::cerr << "Invalid size format: " << size_str << std::endl;
		exit(1);
	}
	if (unit == 'K') num *= 1024;
	else if (unit == 'M') num *= 1024 * 1024;
	else if (unit == 'G') num *= 1024 * 1024 * 1024;
	else if (unit != '\0') {
		std::cerr << "Invalid size unit: " << size_str << std::endl;
		exit(1);
	}
	return num;
}

void Config::parseDirective(const std::vector<std::string>& tokens, ServerConfig* current_server, LocationConfig* current_location) {
    if (tokens.empty()) return;

    std::string directive = tokens[0];
    std::vector<std::string> values;
    for (size_t i = 1; i < tokens.size(); ++i) {
        values.push_back(tokens[i]);
    }

    if (current_location) {
        if (directive == "allow_methods") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_location->allow_methods.insert(values[i]);
            }
        } else if (directive == "root") {
            if (values.size() == 1) current_location->root = values[0];
            else {
                std::cerr << "Invalid root directive" << std::endl;
                exit(1);
            }
        } else if (directive == "autoindex") { // Nouvelle prise en charge pour autoindex
            if (values.size() == 1 && values[0] == "on") current_location->autoindex = true;
            else if (values.size() == 1 && values[0] == "off") current_location->autoindex = false;
            else {
                std::cerr << "Invalid autoindex directive" << std::endl;
                exit(1);
            }
        } else if (directive == "index") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_location->index.push_back(values[i]);
            }
        } else if (directive == "cgi_extension") {
            if (values.size() == 2) {
                current_location->cgi_extensions[values[0]] = values[1];
            } else {
                std::cerr << "Invalid cgi_extension directive" << std::endl;
                exit(1);
            }
        } else if (directive == "upload_path") {
            if (values.size() == 1) current_location->upload_path = values[0];
            else {
                std::cerr << "Invalid upload_path directive" << std::endl;
                exit(1);
            }
        } else if (directive == "return") {
            if (values.size() == 2) {
                char* endptr;
                int status = strtol(values[0].c_str(), &endptr, 10);
                if (*endptr != '\0' || status < 300 || status > 399) {
                    std::cerr << "Invalid return status code: " << values[0] << std::endl;
                    exit(1);
                }
                current_location->redirect_status = status;
                current_location->redirect_url = values[1];
            } else {
                std::cerr << "Invalid return directive" << std::endl;
                exit(1);
            }
        } else if (directive == "alias") {
            if (values.size() == 1) {
                current_location->alias = values[0];
            } else {
                std::cerr << "Invalid alias directive" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "Unknown location directive: " << directive << std::endl;
            exit(1);
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
                        std::cerr << "Invalid port in listen directive" << std::endl;
                        exit(1);
                    }
                } else {
                    char* endptr;
                    current_server->port = strtol(value.c_str(), &endptr, 10);
                    if (*endptr != '\0') {
                        std::cerr << "Invalid port in listen directive" << std::endl;
                        exit(1);
                    }
                }
            } else {
                std::cerr << "Invalid listen directive" << std::endl;
                exit(1);
            }
        } else if (directive == "server_name") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_server->server_names.push_back(values[i]);
            }
        } else if (directive == "root") {
            if (values.size() == 1) current_server->root = values[0];
            else {
                std::cerr << "Invalid root directive" << std::endl;
                exit(1);
            }
        } else if (directive == "error_page") {
            if (values.size() >= 2) {
                char* endptr;
                int code = strtol(values[0].c_str(), &endptr, 10);
                if (*endptr != '\0') {
                    std::cerr << "Invalid error_page directive" << std::endl;
                    exit(1);
                }
                current_server->error_pages[code] = values[1];
            } else {
                std::cerr << "Invalid error_page directive" << std::endl;
                exit(1);
            }
        } else if (directive == "client_max_body_size") {
            if (values.size() == 1) {
                current_server->max_body_size = parseSize(values[0]);
            } else {
                std::cerr << "Invalid client_max_body_size directive" << std::endl;
                exit(1);
            }
        } else if (directive == "index") {
            for (size_t i = 0; i < values.size(); ++i) {
                current_server->index.push_back(values[i]);
            }
        } else {
            std::cerr << "Unknown server directive: " << directive << std::endl;
            exit(1);
        }
    } else {
        std::cerr << "Directive outside server block: " << directive << std::endl;
        exit(1);
    }
}

void Config::parseFile(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir " << path << std::endl;
        return;
    }

    ServerConfig* current_server = 0;
    LocationConfig* current_location = 0;
    std::string line;
    int brace_level = 0;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        // Vérifier si la ligne contient un '{'
        size_t brace_pos = line.find('{');
        if (brace_pos != std::string::npos) {
            std::string directive_part = trim(line.substr(0, brace_pos));
            std::vector<std::string> tokens = split(directive_part, ' ');
            if (!tokens.empty()) {
                if (tokens[0] == "server" && brace_level == 0) {
                    if (current_server) {
                        std::cerr << "Nested server block" << std::endl;
                        exit(1);
                    }
                    current_server = new ServerConfig();
                    ++brace_level;
                } else if (tokens[0] == "location" && tokens.size() >= 2 && current_server && brace_level == 1) {
                    if (current_location) {
                        std::cerr << "Nested location block" << std::endl;
                        exit(1);
                    }
                    current_location = new LocationConfig();
                    current_location->path = tokens[1];
                    ++brace_level;
                }
            }
            // Traiter le '{' séparément
            if (brace_pos < line.length() - 1) {
                std::string rest = trim(line.substr(brace_pos + 1));
                if (!rest.empty()) {
                    std::vector<std::string> rest_tokens = split(rest, ' ');
                    if (!rest_tokens.empty()) {
                        parseDirective(rest_tokens, current_server, current_location);
                    }
                }
            }
            continue;
        }

        // Remove trailing semicolon for regular directives
        if (!line.empty() && line[line.length() - 1] == ';') {
            line.erase(line.length() - 1);
        }

        if (line == "}") {
            --brace_level;
            if (brace_level < 0) {
                std::cerr << "Unmatched closing brace" << std::endl;
                exit(1);
            }
            if (current_location && brace_level == 1) {
                current_server->locations.push_back(*current_location);
                delete current_location;
                current_location = 0;
            } else if (!current_location && brace_level == 0) {
                servers.push_back(*current_server);
                delete current_server;
                current_server = 0;
            }
            continue;
        }

        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.empty()) continue;

        parseDirective(tokens, current_server, current_location);
    }

    file.close();
    if (brace_level != 0) {
        std::cerr << "Unclosed block in configuration file" << std::endl;
        exit(1);
    }
    if (current_server || current_location) {
        std::cerr << "Incomplete block at end of file" << std::endl;
        exit(1);
    }
}