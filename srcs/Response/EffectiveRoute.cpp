/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EffectiveRoute.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 14:20:55 by adebert           #+#    #+#             */
/*   Updated: 2025/09/20 15:39:50 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EffectiveRoute.hpp"

bool EffectiveRoute::createEffectiveRoute(const ServerConfig* srv, const LocationConfig* loc) {
	std::cout << "USE LOCATION" << std::endl;	
	std::cout << *loc << std::endl;	
	
	server = srv;
	location = loc;
	useLocation = true;
	
	if (loc->has_alias) {
		use_alias = true;
		alias = loc->alias;
		root.clear();
	} else {
		use_alias = false;
		if (loc->has_root)
			root = loc->root;
		else if (srv->has_root)
			root = srv->root;
		else
			return false; 
	}
	
	if (!loc->allow_methods.empty())
		allow_methods = loc->allow_methods;
	else if (!srv->allow_methods.empty())
		allow_methods = srv->allow_methods;
	else {
		allow_methods.insert("GET");
		allow_methods.insert("POST");
		allow_methods.insert("DELETE");
	}
	
	if (loc->autoindex)
		autoindex = loc->autoindex;
	else
		autoindex = srv->autoindex;
	
	location_prefix = loc->path;
	redirect_status = loc->redirect_status;
	redirect_url = loc->redirect_url;
	
	return true;
}

bool EffectiveRoute::createEffectiveRoute(const ServerConfig* srv) {
	std::cout << "NO LOCATION" << std::endl;	
	server = srv;
	location = NULL;
	useLocation = false;
	
	use_alias = false;
	if (srv->has_root)
		root = srv->root;
	else
		return false;
	
	if (!srv->allow_methods.empty())
		allow_methods = srv->allow_methods;
	else {
		allow_methods.insert("GET");
		allow_methods.insert("POST");
		allow_methods.insert("DELETE");
	}
	
	autoindex = srv->autoindex;
	
	location_prefix = "/";
	redirect_status = 0;
	redirect_url = std::string();

	return true;
}

int EffectiveRoute::createEffectivePath(std::string req_uri) {
	std::string base;
	std::string suffix;
	std::string prefix = location_prefix;
	
	if (use_alias) {
		base = alias;
		if (req_uri.size() < prefix.size()) return 404;
		suffix = req_uri.substr(prefix.size());
		uri = joinPaths(base, suffix);
	} else {
		base = root;
		if (req_uri.size() < prefix.size()) return 404;
		suffix = req_uri;
		uri = joinPaths(base, suffix);	
	}
	
	if (uri.empty()) return 500;	
	
	
	int result;
	result = isValidPath();
	if (result != PATH_OK)
		return result;
	
	return PATH_OK;	
}

std::string joinPaths(const std::string& base, const std::string& suffix) {
    if (base.empty()) return std::string();
    if (suffix.empty())  return base;
    bool baseSlash = base[base.size()-1] == '/';
    bool suffixSlash  = suffix[0] == '/';
    if (baseSlash && suffixSlash) return base + suffix.substr(1);
    if (!baseSlash && !suffixSlash) return base + "/" + suffix;
    return base + suffix;
}


int EffectiveRoute::isValidPath(void) {
	std::string toCheck;

	if (getMethod)
		toCheck = uri;
	else {
    	size_t SlashPos = uri.find_last_of('/') + 1;
		toCheck = uri.substr(0, SlashPos);
	}
	
	std::cout << "URI CHECKED:\t" << toCheck << std::endl;
	
	
	struct stat st;
    if (::stat(toCheck.c_str(), &st) != 0) { //Checking Errno is allowed: stat is neither a read or write action
        switch (errno) {
            case ENOENT:      	return 404;
            case ENOTDIR:     	return 404;
            case EACCES:      	return 403;
            case EPERM:       	return 403;
            case ELOOP:       	return 403;
            case ENAMETOOLONG:	return 403;
            default:          	return 500;
        }
    }

	if (getMethod)
    	isDir = S_ISDIR(st.st_mode) != 0;
	else
		isDir = false;

    if (isDir && getMethod) {
        if (!uri.empty() && uri[uri.size() - 1] != '/')
            uri += '/';
    }	
	
	return PATH_OK;
}

std::ostream &operator<<(std::ostream &o, const EffectiveRoute&i) {
	o << "EFFECTIVE ROUTE:\n"
      << "Final Request URI:\t" << i.uri << "\n"
      << "Is it a Directory?:\t" << (i.isDir ? "yes" : "no") << "\n"
      << "Autoindex Allowed?:\t" << (i.autoindex ? "yes" : "no") << "\n"
      << "Methods Allowed:\t";

    for (std::set<std::string>::const_iterator it = i.allow_methods.begin();
         it != i.allow_methods.end(); ++it) {
        if (it != i.allow_methods.begin())
            o << ", ";
        o << *it;
    }
	
	o << "\nUse Location:\t\t" << (i.useLocation ? "yes" : "no") << "\n";
	o << "Close Connection:\t" << (i.closeConnection ? "yes" : "no") << "\n";
    o << std::endl;	

	return o;
}