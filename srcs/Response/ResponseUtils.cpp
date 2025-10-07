/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseUtils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 17:45:56 by Matprod           #+#    #+#             */
/*   Updated: 2025/10/07 17:29:12 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseUtils.hpp"

/* std::string readLine(int fd) {
	std::string line;
	char c;
	while (read(fd, &c, 1) == 1) {
		line += c;
		if (c == '\n') break;
	}
	return line;
} */

std::string readChunkedBody(const std::string& rawData) {
	std::string body;
	size_t pos = 0;

	while (true) {
		// 1. Trouver la fin de la taille de chunk (jusqu'à \r\n)
		size_t chunkSizeEnd = rawData.find("\r\n", pos);
		if (chunkSizeEnd == std::string::npos)
			throw std::runtime_error("Invalid chunked encoding (missing \\r\\n after size)");

		// 2. Lire la taille en hexadécimal
		std::string chunkSizeHex = rawData.substr(pos, chunkSizeEnd - pos);
		size_t chunkSize = std::strtoul(chunkSizeHex.c_str(), NULL, 16);

		// 3. 0 = fin de chunks
		if (chunkSize == 0)
			break;

		// 4. Lire les données du chunk
		pos = chunkSizeEnd + 2;
		if (rawData.size() < pos + chunkSize)
			throw std::runtime_error("Chunk size exceeds data size");

		body.append(rawData, pos, chunkSize);
		pos += chunkSize;

		// 5. Sauter \r\n après les données
		if (rawData.substr(pos, 2) != "\r\n")
			throw std::runtime_error("Expected \\r\\n after chunk data");
		pos += 2;
	}

	// Skip final \r\n if present
	if (rawData.substr(pos, 2) == "\r\n")
		pos += 2;

	return body;
}

ServerConfig* getMatchingServer(const Request& req, const std::vector<ServerConfig>& servers, Response& res) {
	ServerConfig* server = findMatchingServer(req, servers);
	if (!server) {
		std::map<int, std::string> empty_map;
		res.createResponse(400, "", empty_map);
	}
	return server;
}

LocationConfig* getMatchingLocation(const Request& req, const ServerConfig* server, Response& res, bool &use_location) {
	LocationConfig* loc = findMatchingLocation(req.uri, server->locations, use_location);
	if (!loc)
		res.createResponse(404, "", server->error_pages);
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

bool isCGIRequest(const LocationConfig* loc, const std::string& uri) {
	if (!loc)
		return false;
		
    std::string extension = getFileExtension(uri);
/*     std::cout << "[isCGIRequest] uri: " << uri << ", extension: " << extension << std::endl;
    std::cout << "[isCGIRequest] cgi_extensions size: " << loc->cgi_extensions.size() << std::endl; */
    for (std::map<std::string, std::string>::const_iterator it = loc->cgi_extensions.begin(); it != loc->cgi_extensions.end(); ++it) {
        //std::cout << "[isCGIRequest] cgi_extension: " << it->first << " -> " << it->second << std::endl;
    }
    bool result = (!loc->cgi_extensions.empty() && loc->cgi_extensions.find(extension) != loc->cgi_extensions.end());
    //std::cout << "[isCGIRequest] result: " << result << std::endl;
    return result;
}

std::ostream &operator<<(std::ostream &o, const LocationConfig&i) {
	o << "LOCATION:\n";
	 o << i.path << std::endl;
	return o;
}