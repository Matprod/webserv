/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:44:20 by allan             #+#    #+#             */
/*   Updated: 2025/07/13 19:18:14 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::string to_lower(const std::string& str) {
	std::string res = str;
	std::transform(res.begin(), res.end(), res.begin(), ::tolower);
	return res;
}

int get_request(int socket, Request& req) {
	std::string request;
	char buffer[4096]; // Increased buffer size
	int bytes_read;
	bool headers_complete = false;
	size_t content_length = 0;
	size_t body_start = 0;
	bool is_chunked = false;

	bytes_read = recv(socket, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read <= 0) {
		if (bytes_read < 0)
			std::cerr << "Erreur lors de la lecture de la socket\n";
		return (bytes_read < 0) ? REQUEST_ERROR : REQUEST_INCOMPLETE;
	}
	buffer[bytes_read] = '\0';
	request.append(buffer, bytes_read);

	if (!headers_complete) {
		size_t header_end = request.find("\r\n\r\n");
		if (header_end != std::string::npos) {
			headers_complete = true;
			body_start = header_end + 4;

			std::istringstream iss(request.substr(0, header_end));
			std::string request_line;
			if (!std::getline(iss, request_line)) {
				std::cerr << "Erreur: Ligne de requête manquante\n";
				return REQUEST_ERROR;
			}

			size_t method_end = request_line.find(' ');
			if (method_end == std::string::npos)
				return REQUEST_ERROR;
			req.method = request_line.substr(0, method_end);

			size_t uri_end = request_line.find(' ', method_end + 1);
			if (uri_end == std::string::npos)
				return REQUEST_ERROR;
			req.uri = request_line.substr(method_end + 1, uri_end - method_end - 1);

			req.version = request_line.substr(uri_end + 1);
			req.version.erase(req.version.find_last_not_of(" \t\r\n") + 1);

			std::string line;
			while (std::getline(iss, line)) {
				if (line == "\r" || line.empty())
					break;
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

			if (req.headers.count("transfer-encoding") && req.headers["transfer-encoding"] == "chunked") {
				is_chunked = true;
			} else if (req.headers.count("content-length")) {
				char* endptr;
				long len = strtol(req.headers["content-length"].c_str(), &endptr, 10);
				if (*endptr != '\0' || len < 0 || len > MAX_BODY_SIZE) {
					std::cerr << "Erreur: Content-Length invalide ou trop grand\n";
					return REQUEST_ERROR;
				}
				content_length = static_cast<size_t>(len);
			}
		}
	}

	if (headers_complete) {
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
				if (chunk_size == 0) {
					req.body = body;
					return REQUEST_OK;
				}
				pos = chunk_size_end + 2;
				if (pos + chunk_size + 2 > request.length())
					return REQUEST_INCOMPLETE;
				body.append(request.substr(pos, chunk_size));
				pos += chunk_size + 2; // Skip chunk data and trailing \r\n
				if (body.length() > MAX_BODY_SIZE) {
					std::cerr << "Erreur: Body size exceeds MAX_BODY_SIZE\n";
					return REQUEST_ERROR;
				}
			}
			return REQUEST_INCOMPLETE;
		} else if (request.length() >= body_start + content_length) {
			if (content_length > 0)
				req.body = request.substr(body_start, content_length);
			return REQUEST_OK;
		}
	}

	return REQUEST_INCOMPLETE;
}
