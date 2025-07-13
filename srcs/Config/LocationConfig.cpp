/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:16:00 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/13 21:41:17 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "Config.hpp"


bool Config::parseLocationDirective(const std::string& directive, const std::vector<std::string>& values, LocationConfig* loc) {
	if (directive == "allow_methods") {
		for (size_t i = 0; i < values.size(); ++i)
			loc->allow_methods.insert(values[i]);
	} else if (directive == "root") {
		if (values.size() != 1) {
			std::cerr << "Error: Invalid root directive" << std::endl;
			return ERROR;
		}
		if (!loc->root.empty()) {
			std::cerr << "Error: Duplicate root directive in location " << loc->path << std::endl;
			return ERROR;
		}
		if (!loc->alias.empty()) {
			std::cerr << "Error: Cannot use root and alias together in location " << loc->path << std::endl;
			return ERROR;
		}
		loc->root = values[0];
	} else if (directive == "autoindex" || directive == "directory_listing") {
		if (values.size() != 1) {
			std::cerr << "Invalid " << directive << " directive" << std::endl;
			return ERROR;
		}
		if (values[0] == "on" || values[0] == "on;")
			loc->autoindex = true;
		else if (values[0] == "off" || values[0] == "off;")
			loc->autoindex = false;
		else {
			std::cerr << "Invalid " << directive << " directive value: " << values[0] << std::endl;
			return ERROR;
		}
	} else if (directive == "index") {
		for (size_t i = 0; i < values.size(); ++i)
			loc->index.push_back(values[i]);
	} else if (directive == "cgi_extension") {
		if (values.size() != 2) {
			std::cerr << "Error: Invalid cgi_extension directive" << std::endl;
			return ERROR;
		}
		loc->cgi_extensions[values[0]] = values[1];
	} else if (directive == "upload_path") {
		if (values.size() != 1) {
			std::cerr << "Error: Invalid upload_path directive" << std::endl;
			return ERROR;
		}
		loc->upload_path = values[0];
	} else if (directive == "return") {
		if (values.size() != 2) {
			std::cerr << "Error: Invalid return directive" << std::endl;
			return ERROR;
		}
		char* endptr;
		int status = strtol(values[0].c_str(), &endptr, 10);
		if (*endptr != '\0' || status < 300 || status > 399) {
			std::cerr << "Error: Invalid return status code: " << values[0] << std::endl;
			return ERROR;
		}
		loc->redirect_status = status;
		loc->redirect_url = values[1];
	} else if (directive == "alias") {
		if (values.size() != 1) {
			std::cerr << "Invalid alias directive" << std::endl;
			return ERROR;
		}
		if (!loc->root.empty()) {
			std::cerr << "Error: Cannot use root and alias together in location " << loc->path << std::endl;
			return ERROR;
		}
		loc->alias = values[0];
	} else {
		std::cerr << "Error: Unknown location directive: " << directive << std::endl;
		return ERROR;
	}
	return 0;
}