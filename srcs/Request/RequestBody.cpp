/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 10:43:19 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/18 10:49:59 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "RequestBody.hpp"

int parse_body_normal(const std::string& request, size_t body_start, size_t content_length, Request& req, std::map<int, std::string>& buffers, int socket) {
	if (request.length() >= body_start + content_length) {
		if (content_length > 0)
			req.body = request.substr(body_start, content_length);
		else
			req.body = "";
		buffers.erase(socket);
		return REQUEST_OK;
	}
	return REQUEST_INCOMPLETE;
}

// Parsing body chunked
int parse_body_chunked(const std::string& request, size_t body_start, Request& req, std::map<int, std::string>& buffers, int socket) {
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
			buffers.erase(socket);
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