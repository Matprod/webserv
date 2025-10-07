/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 11:00:25 by Matprod           #+#    #+#             */
/*   Updated: 2025/10/07 15:18:25 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

// Suppression de la définition de LocationConfig::operator= dans ce fichier (déjà définie dans LocationConfig.cpp)

Config::Config(const std::string& path) {
	this->error = 0;
	if (parseFile(path) == ERROR)
	{
		std::cerr << "Error: Failed to parse the config file" << std::endl;
		this->error = 1;	
	}
	else if (serversHaveRoot() == ERROR)
	{
		std::cerr << "Error: At least one server must have a root directive" << std::endl;
		this->error = 1;
	}
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
