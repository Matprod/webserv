/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  %+:+    %+:+   */
/*                                                %+:+%+:%+           */
/*   Created: 2025/07/18 10:43:19 by Matprod           %#+    %#+             */
/*   Updated: 2025/07/30 17:10:00 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "RequestBody.hpp"
#include <iostream>

void printBody(const Request& request) {
    std::cout << "Request Body: ";
    if (request.body.empty()) {
        std::cout << "(empty)" << std::endl;
    } else {
        std::cout << "'" << request.body << "' (size: " << request.body.length() << " bytes)" << std::endl;
    }
}

int parse_body_normal(const std::string& request, size_t body_start, size_t content_length, Request& req, std::map<int, std::string>& buffers, int socket) {
    if (request.length() >= body_start + content_length) {
        if (content_length > 0)
            req.body = request.substr(body_start, content_length);
        else
            req.body = "";
        buffers[socket].erase(0, body_start + content_length);
        return REQUEST_OK;
    }
    return REQUEST_INCOMPLETE;
}

int parse_body_chunked(const std::string& request, size_t body_start, Request& req, std::map<int, std::string>& buffers, int socket) {
    std::string body;
    size_t pos = body_start;
    std::cerr << "Starting parse_body_chunked, request length: " << request.length() << ", body_start: " << body_start << std::endl;

    while (pos < request.length()) {
        std::cerr << "Current pos: " << pos << ", remaining: " << request.substr(pos) << std::endl;
        size_t chunk_size_end = request.find("\r\n", pos);
        if (chunk_size_end == std::string::npos) {
            std::cerr << "No chunk size end found" << std::endl;
            return REQUEST_INCOMPLETE;
        }

        std::string chunk_size_str = request.substr(pos, chunk_size_end - pos);
        std::cerr << "Chunk size str: " << chunk_size_str << std::endl;
        char* endptr;
        long chunk_size = strtol(chunk_size_str.c_str(), &endptr, 16);
        if (*endptr != '\0' || chunk_size < 0) {
            std::cerr << "Invalid chunk size, skipping: " << chunk_size_str << std::endl;
            pos = chunk_size_end + 2;
            continue;
        }

        pos = chunk_size_end + 2; // Passe après \r\n de la taille
        if (chunk_size == 0) {
            req.body = body;
            size_t processed = pos; // Inclut le \r\n final si présent
            if (pos < request.length() && request[pos] == '\r' && request[pos + 1] == '\n')
                processed += 2;
            buffers[socket].erase(0, processed);
            std::cerr << "Chunked body complete, size: " << body.length() << ", processed: " << processed << std::endl;
			printBody(req);
            return REQUEST_OK;
        }

        if (pos + chunk_size + 2 > request.length()) { // +2 pour \r\n suivant
            std::cerr << "Incomplete chunk, needed: " << (pos + chunk_size + 2) << ", available: " << request.length() << std::endl;
            return REQUEST_INCOMPLETE;
        }

        std::string chunk_data = request.substr(pos, chunk_size);
        std::cerr << "Appending chunk: " << chunk_data << " (size: " << chunk_size << ")" << std::endl;
        body.append(chunk_data);
        pos += chunk_size + 2; // Passe après les données et \r\n
        if (body.length() > MAX_BODY_SIZE)
            return REQUEST_ERROR;
    }
    return REQUEST_INCOMPLETE;
}