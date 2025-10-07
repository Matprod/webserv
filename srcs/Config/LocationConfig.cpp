/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:16:00 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/21 17:24:58 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"
#include "Config.hpp"

void print_vector(const std::vector<std::string>& vec) {
    std::cout << "Vector contents: [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

bool Config::parseLocationDirective(const std::string& directive, const std::vector<std::string>& values, LocationConfig* loc) {
	if (directive == "allow_methods") {
		if (values.size() >= 1) {
			for (size_t i = 0; i < values.size(); ++i)
				loc->allow_methods.insert(values[i]);
		} else
			loc->allow_methods.insert("NONE");
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
		loc->has_root = true;
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
		if (values.size() != 1) {
			std::cerr << "Error: Invalid index directive - only one value allowed" << std::endl;
			return ERROR;
		}
		loc->index = values[0];
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
			print_vector(values);
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

LocationConfig::LocationConfig()
	: allow_methods()
	, root("")
	, autoindex(false)
	, index("")
	, cgi_extensions()
	, upload_path("")
	, path("")
	, redirect_status(0)
	, redirect_url("")
	, alias("")
	, has_root(false)
	, has_alias(false)
{
	// Tous les containers sont initialisés
}

LocationConfig::LocationConfig(const LocationConfig& src)
	: allow_methods(src.allow_methods)
	, root(src.root)
	, autoindex(src.autoindex)
	, index(src.index)
	, cgi_extensions(src.cgi_extensions)
	, upload_path(src.upload_path)
	, path(src.path)
	, redirect_status(src.redirect_status)
	, redirect_url(src.redirect_url)
	, alias(src.alias)
	, has_root(src.has_root)
	, has_alias(src.has_alias)
{
}

LocationConfig& LocationConfig::operator=(const LocationConfig& rhs) {
	if (this != &rhs) {
		allow_methods = rhs.allow_methods;
		root = rhs.root;
		autoindex = rhs.autoindex;
		index = rhs.index;
		cgi_extensions = rhs.cgi_extensions;
		upload_path = rhs.upload_path;
		path = rhs.path;
		redirect_status = rhs.redirect_status;
		redirect_url = rhs.redirect_url;
		alias = rhs.alias;
		has_root = rhs.has_root;
		has_alias = rhs.has_alias;
	}
	return *this;
}

LocationConfig::~LocationConfig() {}