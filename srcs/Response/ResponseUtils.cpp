/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseUtils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 17:45:56 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/22 17:54:57 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseUtils.hpp"

ServerConfig* getMatchingServer(const Request& req, const std::vector<ServerConfig>& servers, Response& res) {
	ServerConfig* server = findMatchingServer(req, servers);
	if (!server) {
		res.statusCode = 400;
		res.statusMessage = getStatusMessage(400);
		res.body = "Bad Request: No matching server";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}
	return server;
}

LocationConfig* getMatchingLocation(const Request& req, ServerConfig& server, Response& res) {
	LocationConfig* loc = findMatchingLocation(req.uri, server.locations);
	if (!loc) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "Not Found";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}
	return loc;
}

bool handleRedirect(const LocationConfig& loc, Response& res) {
	if (loc.redirect_status >= 300 && loc.redirect_status <= 399) {
		res.statusCode = loc.redirect_status;
		res.statusMessage = getStatusMessage(loc.redirect_status);
		res.headers["Location"] = loc.redirect_url;
		res.body = "Redirecting to " + loc.redirect_url;
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return true;
	}
	return false;
}

bool isCGIRequest(const LocationConfig& loc, const std::string& uri) {
	std::string extension = getFileExtension(uri);
	return (!loc.cgi_extensions.empty() &&
			loc.cgi_extensions.find(extension) != loc.cgi_extensions.end());
}