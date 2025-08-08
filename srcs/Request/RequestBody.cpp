/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 10:43:19 by Matprod           #+#    #+#             */
/*   Updated: 2025/08/05 19:07:13 by Matprod          ###   ########.fr       */
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
	std::string&	buffer = buffers[socket];
	std::string&	body = req.body;
	size_t			pos = body_start;
	//			data_size = -1;
	long			all_chunk_size = 0;
	size_t			all_chunk_size_end = 0;
	size_t			string_size = 0;
	size_t			start_chunk = 0;
	long			size_chunk_digit = 0;
	long			real_size_chunk;
	size_t			start_end_chunk;
	//size_t			real_end_chunk = 0;
	size_t			start_string = 0;
	size_t			end_string = 0;
	char *endptr;
	std::cout << std::endl << "______________________________________________________________" << std::endl;
	//setup all chunk and start chunk;
	std::cout << "pos :" << pos << std::endl;
	all_chunk_size_end = request.find("\n", pos);
	if (all_chunk_size_end == std::string::npos)
		return REQUEST_INCOMPLETE;
	std::cout << "chunk_size_end :" << all_chunk_size_end << std::endl;
	all_chunk_size = strtol(request.substr(pos, all_chunk_size_end - pos).c_str(), &endptr, 16);
	std::cout << "all chunk size (at first in hexa but nox in decimal) : " << all_chunk_size << std::endl;
	if (all_chunk_size < 0)
		return REQUEST_ERROR;
	start_chunk = all_chunk_size_end + 1;
	while (true) {
		

		// setup chunk size 
		std::cout << "start_chunk pos : " << start_chunk << " | start_chunk value : "<< request.substr(start_chunk, 1) << std::endl;
		start_end_chunk = request.find("\n", start_chunk);
		if (start_end_chunk == std::string::npos)
			return REQUEST_INCOMPLETE;
		std::cout << "start end chunk pos = " << start_end_chunk << " | start end chunk value : "<< request.substr(start_end_chunk, 1) << std::endl;
		size_chunk_digit = start_end_chunk - start_chunk;
		std::cout << "size chunk = " << size_chunk_digit << std::endl;
		real_size_chunk = strtol(request.substr(start_chunk, size_chunk_digit).c_str(), &endptr, 16);
		if (real_size_chunk == 0) {
			// Verify trailing CRLF
			size_t final_crlf = request.find("\r\n", start_end_chunk);
			if (final_crlf == std::string::npos) {
				return REQUEST_INCOMPLETE;
			}
			
			// Remove processed data from buffer
			buffer.erase(0, final_crlf + 2);
			return REQUEST_OK;
		}
		
		if (real_size_chunk < 0 || *endptr != '\0')
			return REQUEST_ERROR;
			
		// setup string for body
		start_string = start_end_chunk + 1; // ptet + 1
		std::cout << "start string = " << start_string << std::endl;
		end_string =  request.find("\n", start_string);
		if (end_string == std::string::npos)
			return REQUEST_INCOMPLETE;

		string_size = end_string - start_string;
		std::cout << "body appened = " << request.substr(start_string, string_size) << std::endl;
		body.append(request.substr(start_string, string_size));
		
		std::cout << "BODY = " << req.body;
		if (body.length() > MAX_BODY_SIZE)
			return REQUEST_ERROR;
		// setup new position
		start_chunk = end_string + 1;
		std::cout << std::endl << "______________________________________________________________" << std::endl;
	}
	std::cout << "Request chunk incomplete 2" << std::endl;
	return REQUEST_INCOMPLETE;
}


/*int parse_body_chunked(const std::string& request, size_t body_start, Request& req, std::map<int, std::string>& buffers, int socket) {
	std::string& buffer = buffers[socket];
	std::string& body = req.body;
	size_t pos = body_start;
	size_t chunk_data_start = 0;
	size_t chunk_string_start = 0;
	size_t size_data = 0;
	std::string size_data_string;
	long real_size_data = 0;
	size_t chunk_size_end;
	//int i = 0;
	
	//std::cout << "buffer = " << buffer <<  std::endl << "end of buffer" << std::endl;
	std::cout << "Starting parse_body_chunked, request length: " << request.length() << ", body_start: " << body_start << std::endl;
	//std::cout << "body start value =" << request.substr(body_start, 1) << std::endl;
	while (true) {
		
		std::cout << "pos :" << pos << std::endl;
		std::cout << "pos value =" << request.substr(pos, 1) << std::endl;

			chunk_size_end = request.find("\r\n", pos);
		std::cout << "chunk_size_end :" << chunk_size_end << std::endl;
		if (chunk_size_end == std::string::npos)
			return REQUEST_INCOMPLETE;

		
		char* endptr;
		chunk_data_start = chunk_size_end + 2;
		std::cout << "chunk_data_start :" << chunk_data_start << std::endl;
		chunk_string_start = chunk_data_start + 2;
		size_data = request.find("\n", chunk_data_start) - chunk_data_start;
		std::cout << "size_data = " << size_data << std::endl;
		size_t chunk_data_end = chunk_data_start + size_data;
		std::cout << "chunk_data_end : " << chunk_data_end << " | value chunk_data_end = " << request.substr(chunk_data_end, 1) << std::endl;
		size_data_string = request.substr(chunk_data_start, size_data);
		std::cout << "print data string = " << size_data_string << std::endl;
		real_size_data = strtol(size_data_string.c_str(), &endptr, 16);
		std::cout << "real size data = " << real_size_data << std::endl;
		size_t chunk_terminator_end = chunk_data_end + real_size_data;
		std::cout << "chunk_terminator_end :" << chunk_terminator_end << std::endl;


		if (chunk_terminator_end > request.length())
		{
			std::cout << "request len = " << request.length() << std::endl << "Request chunk incomplete" << std::endl;
			return REQUEST_INCOMPLETE;
		}
		
		std::cout << "request data endddddddddddd =" << request.substr(chunk_data_end, 1) << std::endl;
		if (request.substr(chunk_data_end, 1) != "\n")
		{
			std::cout << "Request chunk not terminated with \\r\\n" << std::endl;
			return REQUEST_ERROR;
		}

		if (real_size_data == 0) {
			buffer.erase(0, chunk_terminator_end);
			return REQUEST_OK;
		}

		std::cout << "body append = " << request.substr(chunk_string_start, real_size_data) << std::endl;
		body.append(request.substr(chunk_string_start, real_size_data));
		std::cout << "BODY = " << req.body;
		if (body.length() > MAX_BODY_SIZE)
			return REQUEST_ERROR;

		pos = chunk_terminator_end +2;
		std::cout << std::endl << "______________________________________________________________" << std::endl;
	}
	std::cout << "Request chunk incomplete 2" << std::endl;
	return REQUEST_INCOMPLETE;
}*/
