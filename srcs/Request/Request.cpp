/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:44:20 by allan             #+#    #+#             */
/*   Updated: 2025/07/15 20:21:48 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::string to_lower(const std::string& str) {
	std::string res = str;
	std::transform(res.begin(), res.end(), res.begin(), ::tolower);
	return res;
}

int parse_request(int socket, Request& req, std::map<int, std::string>& buffers, std::map<int, time_t> lastActivity) {
	char buffer[4096];
	int bytes_read;

	bytes_read = recv(socket, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read <= 0) {
		if (bytes_read < 0)
			std::cerr << "Erreur lors de la lecture de la socket\n";
		return (bytes_read < 0) ? REQUEST_ERROR : REQUEST_INCOMPLETE;
	}
	lastActivity[socket] = time(NULL); 
	buffer[bytes_read] = '\0';
	buffers[socket].append(buffer, bytes_read);
	std::string& request = buffers[socket];

	// Trouver fin des headers
	size_t header_end = request.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return REQUEST_INCOMPLETE;

	size_t body_start = header_end + 4;

	// 🔽 Parse ligne de requête
	std::istringstream iss(request.substr(0, header_end));
	std::string request_line;
	if (!std::getline(iss, request_line)) return REQUEST_ERROR;

	size_t method_end = request_line.find(' ');
	if (method_end == std::string::npos) return REQUEST_ERROR;
	req.method = request_line.substr(0, method_end);

	size_t uri_end = request_line.find(' ', method_end + 1);
	if (uri_end == std::string::npos) return REQUEST_ERROR;
	req.uri = request_line.substr(method_end + 1, uri_end - method_end - 1);

	req.version = request_line.substr(uri_end + 1);
	req.version.erase(req.version.find_last_not_of(" \t\r\n") + 1);

	// 🔽 Parse headers
	std::string line;
	while (std::getline(iss, line)) {
		if (line == "\r" || line.empty()) break;
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos) {
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);
			req.headers[to_lower(key)] = value;
		}
	}

	// 🔽 Gérer Body
	bool is_chunked = req.headers.count("transfer-encoding") && to_lower(req.headers["transfer-encoding"]) == "chunked";
	size_t content_length = 0;
	if (!is_chunked && req.headers.count("content-length")) {
		char* endptr;
		long len = strtol(req.headers["content-length"].c_str(), &endptr, 10);
		if (*endptr != '\0' || len < 0 || len > MAX_BODY_SIZE) {
			std::cerr << "Content-Length invalide ou trop grand\n";
			return REQUEST_ERROR;
		}
		content_length = static_cast<size_t>(len);
	}

	if (is_chunked) {
		std::string body;
		size_t pos = body_start;

		while (pos < request.length()) {
			size_t chunk_size_end = request.find("\r\n", pos);
			if (chunk_size_end == std::string::npos)
				return REQUEST_INCOMPLETE;

			std::string chunk_size_str = request.substr(pos, chunk_size_end - pos);
			char* endptr;
			long chunk_size = strtol(chunk_size_str.c_str(), &endptr, 16);
			if (*endptr != '\0' || chunk_size < 0)
				return REQUEST_ERROR;

			pos = chunk_size_end + 2;
			if (chunk_size == 0) {
				req.body = body;
				buffers.erase(socket); // nettoyage du buffer
				return REQUEST_OK;
			}

			if (pos + chunk_size + 2 > request.length())
				return REQUEST_INCOMPLETE;

			body.append(request.substr(pos, chunk_size));
			pos += chunk_size + 2;

			if (body.length() > MAX_BODY_SIZE)
				return REQUEST_ERROR;
		}
		return REQUEST_INCOMPLETE;
	}
	else if (request.length() >= body_start + content_length) {
		if (content_length > 0)
			req.body = request.substr(body_start, content_length);
		buffers.erase(socket); // nettoyage du buffer
		return REQUEST_OK;
	}
	else if (!is_chunked && content_length == 0) {
		req.body = "";
		buffers.erase(socket);
	return REQUEST_OK;
}


	return REQUEST_INCOMPLETE;
}
