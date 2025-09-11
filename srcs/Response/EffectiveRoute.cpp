/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EffectiveRoute.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 14:20:55 by adebert           #+#    #+#             */
/*   Updated: 2025/09/11 17:09:33 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EffectiveRoute.hpp"

bool EffectiveRoute::createEffectiveRoute(const ServerConfig* srv, const LocationConfig* loc) {
	server = srv;
	location = loc;
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
	server = srv;
	location = NULL;
	
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
	
	std::cout << *this << std::endl;
	
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
	struct stat st;
	if (stat(uri.c_str(), &st) != 0) {
    	switch (errno) {
    	    case ENOENT:   		return 404; // No such file or directory
    	    case ENOTDIR:  		return 404; // Component of the path is not a dir
    	    case EACCES:   		return 403; // Permission denied
    	    case EPERM:   		return 403; // Operation not permitted
			case ELOOP:			return 403;
			case ENAMETOOLONG:	return 403;
    	    default:       		return 500; // Unexpected server error
 		   }
	} 
	
	isDir = S_ISDIR(st.st_mode);
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
	
    o << std::endl;	

	return o;
}