/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 16:17:30 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/18 19:30:00 by Matprod          ###   fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response buildResponse(const Request& request, const std::vector<ServerConfig>& servers) {
	Response res;
	res.version = "HTTP/1.1";

	// Trouver le serveur correspondant
	ServerConfig* server = findMatchingServer(request, servers);
	if (!server) {
		res.statusCode = 400;
		res.statusMessage = getStatusMessage(400);
		res.body = "Bad Request: No matching server";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	// Trouver le bloc location correspondant
	LocationConfig* loc = findMatchingLocation(request.uri, server->locations);
	if (!loc) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "Not Found";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	// Gérer les redirections
	if (loc->redirect_status >= 300 && loc->redirect_status <= 399) {
		res.statusCode = loc->redirect_status;
		res.statusMessage = getStatusMessage(loc->redirect_status);
		res.headers["Location"] = loc->redirect_url;
		res.body = "Redirecting to " + loc->redirect_url;
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	// Vérifier si c'est une requête CGI
	std::string extension = getFileExtension(request.uri);
	if (!loc->cgi_extensions.empty() && loc->cgi_extensions.find(extension) != loc->cgi_extensions.end()) {
		return executeCGI(request, *loc, *server);
	}

	// Gérer les méthodes HTTP
	if (request.method == "GET") {
		return handleGet();
	} else if (request.method == "POST") {
		return handlePost(request);
	} else if (request.method == "DELETE") {
		return handleDelete();
	} else {
		res.statusCode = 405;
		res.statusMessage = getStatusMessage(405);
		res.body = "Method Not Allowed";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}
}

Response handleGet() {
	Response res;

	res.version = "HTTP/1.1";
	res.body = "<html><body><h1>Hello, GET!<h1><body><html>";
	res.statusCode = 200;
	res.statusMessage = getStatusMessage(200);
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toString<size_t>(res.body.size());
	return res;
}

Response handlePost(const Request& request) {
	Response res;

	res.version = "HTTP/1.1";
	res.body = "You posted: " + request.body;
	res.statusCode = 200;
	res.statusMessage = getStatusMessage(200);
	res.headers["Content-Type"] = "text/plain";
	res.headers["Content-Length"] = toString<size_t>(res.body.size());
	return res;
}

Response handleDelete() {
	Response res;

	res.version = "HTTP/1.1";
	res.statusCode = 204;
	res.statusMessage = getStatusMessage(204);
	return res;
}


std::string Response::responseToString() const {
	std::ostringstream oss;
	oss << version << " " << statusCode << " " << statusMessage << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n" << body;
	return oss.str();
}

std::string getStatusMessage(int statusCode) {
	switch(statusCode) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 500: return "Internal Server Error";
	}
	return "Unknown";
}
