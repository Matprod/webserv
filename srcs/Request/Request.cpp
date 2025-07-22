/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:44:20 by allan             #+#    #+#             */
/*   Updated: 2025/07/22 17:30:55 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::string to_lower(const std::string& str) {
	std::string res = str;
	std::transform(res.begin(), res.end(), res.begin(), ::tolower);
	return res;
}

int read_socket(int socket, std::map<int, std::string>& buffers, std::map<int, time_t>& lastActivity) {
	char buffer[4096];
	int bytes_read = recv(socket, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read <= 0) {
		if (bytes_read < 0)
			std::cerr << "Erreur lors de la lecture de la socket\n";
		return (bytes_read < 0) ? REQUEST_ERROR : REQUEST_INCOMPLETE;
	}
	lastActivity[socket] = time(NULL);
	buffer[bytes_read] = '\0';
	buffers[socket].append(buffer, bytes_read);
	return bytes_read;
}

// Parsing request
int parse_request_line(const std::string& request_line, Request& req) {
	size_t method_end = request_line.find(' ');
	if (method_end == std::string::npos) return REQUEST_ERROR;
	req.method = request_line.substr(0, method_end);

	size_t uri_end = request_line.find(' ', method_end + 1);
	if (uri_end == std::string::npos) return REQUEST_ERROR;
	req.uri = request_line.substr(method_end + 1, uri_end - method_end - 1);

	req.version = request_line.substr(uri_end + 1);
	req.version.erase(req.version.find_last_not_of(" \t\r\n") + 1);
	return REQUEST_OK;
}

// Parsing headers
int parse_headers(std::istringstream& iss, Request& req) {
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
	return REQUEST_OK;
}

int parse_request(int socket, Request& req, std::map<int, std::string>& buffers, std::map<int, time_t> lastActivity) {
	// Step 1 : Reading in socket for request
	int read_result = read_socket(socket, buffers, lastActivity);
	if (read_result <= 0)
		return read_result;

	std::string& request = buffers[socket];

	// Step 2 : Verify end of the header
	size_t header_end = request.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return REQUEST_INCOMPLETE;
	size_t body_start = header_end + 4;

	// Step 3 : Parse request
	std::istringstream iss(request.substr(0, header_end));
	std::string request_line;
	if (!std::getline(iss, request_line))
		return REQUEST_ERROR;	
	if (parse_request_line(request_line, req) != REQUEST_OK)
		return REQUEST_ERROR;
	if (parse_headers(iss, req) != REQUEST_OK)
		return REQUEST_ERROR;
	
	// Check if it's chunked and size of content length
	bool is_chunked = req.headers.count("transfer-encoding") && to_lower(req.headers["transfer-encoding"]) == "chunked";
	size_t content_length = 0;
	if (!is_chunked && req.headers.count("content-length")) {
		char* endptr;
		long len = strtol(req.headers["content-length"].c_str(), &endptr, 10);
		if (*endptr != '\0' || len < 0 || len > MAX_BODY_SIZE) {
			std::cerr << "Content-Length invalide or too big\n";
			return REQUEST_ERROR;
		}
		content_length = static_cast<size_t>(len);
	}

	// parse body
	if (is_chunked) {
		return parse_body_chunked(request, body_start, req, buffers, socket);
	} else {
		return parse_body_normal(request, body_start, content_length, req, buffers, socket);
	}
}

std::ostream &operator<<(std::ostream &o, const Request &i) {
	o << "Method: " << i.method << std::endl;
	o << "Uri: " << i.uri << std::endl;
	o << "Version: " << i.version << std::endl;
	o << "Headers:\n";
	for (std::map<std::string, std::string>::const_iterator it = i.headers.begin(); it != i.headers.end(); ++it) {
		o << it->first << ":" << it->second << std::endl;
	}
	o << std::endl;
	o << "Body:\n" << i.body << std::endl;
	return o;	
}
