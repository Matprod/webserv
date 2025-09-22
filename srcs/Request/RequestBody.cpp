/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 10:43:19 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/22 14:35:16 by adebert          ###   ########.fr       */
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

size_t count_crlf_sequences(const std::string& input) {
	size_t count = 0;
	size_t pos = 0;
	std::cout << "______________________________________________________________" << std::endl;
	std::cout << "Input string: ";
	for (size_t i = 0; i < input.length(); ++i) {
		char c = input[i];
		if (c == '\r') std::cout << "\\r";
		else if (c == '\n') std::cout << "\\n";
		else std::cout << c;
	}
	std::cout << " (length: " << input.length() << ")" << std::endl;
	std::cout << "Starting CRLF sequence count" << std::endl;

	while (pos < input.length()) {
		std::cout << "pos=" << pos << " -> ";
		if (pos < input.length()) {
			if (input[pos] == '\r') std::cout << "'\\r' (ASCII " << (int)input[pos] << ")";
			else if (input[pos] == '\n') std::cout << "'\\n' (ASCII " << (int)input[pos] << ")";
			else std::cout << "'" << input[pos] << "' (ASCII " << (int)input[pos] << ")";
		} else {
			std::cout << "end of string";
		}
		std::cout << std::endl;

		size_t found = input.find("\r\n", pos);
		
		if (found == std::string::npos) {
			std::cout << "No more CRLF found, ending search at pos: " << pos << std::endl;
			break;
		}
		count++;
		std::cout << "Found CRLF at position: " << found << std::endl;

		pos = found + 2; // Avancer après \r\n (taille de \r\n = 2)

		if (pos >= input.length()) {
			std::cout << "Reached end of string after finding CRLF" << std::endl;
			break;
		}
	}

	std::cout << "Total CRLF sequences found: " << count << std::endl;
	std::cout << "______________________________________________________________" << std::endl;

	return count;
}

void print_request(const std::string& request) {
	std::cout << "______________________________________________________________" << std::endl;
	std::cout << "Printing Request Details" << std::endl;
	std::cout << "Request length: " << request.length() << " bytes" << std::endl;

	std::cout << "Request content: ";
	for (size_t i = 0; i < request.length(); ++i) {
		char c = request[i];
		if (c == '\r') std::cout << "\\r";
		else if (c == '\n') std::cout << "\\n";
		else std::cout << c;
	}
	std::cout << std::endl;

	// Afficher les positions des séquences \r\n pour débogage
	/* size_t pos = 0;
	std::cout << "CRLF positions:" << std::endl;
	while (pos < request.length()) {
		size_t found = request.find("\r\n", pos);
		if (found == std::string::npos) break;
		std::cout << "  - CRLF found at position: " << found << std::endl;
		pos = found + 2;
	} */

	std::cout << "______________________________________________________________" << std::endl;
}

int check_final_separator(const std::string& request, size_t start_pos) {
	if (start_pos >= request.length()) {
		std::cout << "Error: Invalid start position" << std::endl;
		return 1;
	}
	size_t zero_pos = request.find("0\r\n", start_pos);
	if (zero_pos == std::string::npos) {
		std::cout << "Error: No final chunk marker '0' found" << std::endl;
		return 1;
	}
	size_t check_pos = zero_pos - 3; // Position avant le "0" pour vérifier \r\n ou \n\r\n
	if (check_pos < start_pos) {
		std::cout << "Error: Not enough data before '0' to check separator" << std::endl;
		return 1;
	}
	if (check_pos >= start_pos + 3 && request.substr(check_pos, 3) == "\n\r\n") {
		return 3;
	}
	else if (request.substr(check_pos + 1, 2) == "\r\n") {
		std::cout << "Bad chunk request" << std::endl;
		return 2;
	}
	std::cout << "Error: Invalid or missing separator before '0'" << std::endl;
	return 1;
}

int parse_body_chunked(const std::string& request, size_t body_start, Request& req, std::map<int, std::string>& buffers, int socket) {
	std::string&	buffer = buffers[socket];
	std::string&	body = req.body;
	size_t			pos = body_start;
	long			all_chunk_size = 0;
	size_t			all_chunk_size_end = 0;
	size_t			string_size = 0;
	size_t			start_chunk = 0;
	long			size_chunk_digit = 0;
	long			real_size_chunk;
	size_t			start_end_chunk;
	size_t			start_string = 0;
	size_t			end_string = 0;
	char *endptr;

	if (pos >= request.length()) {
		std::cout << "Error: Invalid body start position" << std::endl;
		return REQUEST_ERROR;
	}
	all_chunk_size_end = request.find("\n", pos);
	if (all_chunk_size_end == std::string::npos) {
		std::cout << "Error: Chunk size end marker not found" << std::endl;
		return REQUEST_INCOMPLETE;
	}
	int is_full_data_binary = check_final_separator(request, pos);// 3 = \n\r\n | 4 = \r\n | 1 = false
	if (is_full_data_binary == 1 || is_full_data_binary == 2)
		return REQUEST_ERROR;
	if (all_chunk_size < 0) {
		return REQUEST_ERROR;
	}
	if (all_chunk_size < 0)
		return REQUEST_ERROR;
	start_chunk = all_chunk_size_end + 1;
	while (start_chunk < request.length()) {
		if (start_chunk >= request.length()) {
			return REQUEST_INCOMPLETE;
		}
		start_end_chunk = request.find("\n", start_chunk);
		if (start_end_chunk == std::string::npos) {
			return REQUEST_INCOMPLETE;
		}
		size_chunk_digit = start_end_chunk - start_chunk;
		if (size_chunk_digit <= 0) {
			return REQUEST_ERROR;
		}
		if (is_full_data_binary == 3)
		{
			real_size_chunk = strtol(request.substr(start_chunk, size_chunk_digit).c_str(), &endptr, 16);
			
		}
		else if (is_full_data_binary == 2)
		{
			real_size_chunk  = all_chunk_size;
		}
		// Cas de fin (chunk size = 0)
		if (real_size_chunk == 0) {
			size_t final_crlf = request.find("\r\n", start_end_chunk + 2);
			if (final_crlf == std::string::npos) {
				std::cout << "Error: Missing final CRLF" << std::endl;
				return REQUEST_INCOMPLETE;
			}
			
			buffer.clear();
			std::cout << "Chunked transfer complete, body length: " << body.length() << std::endl;
			return REQUEST_OK;
		}
		if (endptr == request.substr(start_chunk, size_chunk_digit).c_str()) {
    		std::cout << "Error: No valid hex digits found in chunk size" << std::endl;
   			return REQUEST_ERROR;
		}
		if (*endptr != '\0') {
    		std::cout << "Error: Invalid characters after chunk size: '" << endptr << "'" << std::endl;
    		return REQUEST_ERROR;
		}
		if (real_size_chunk < 0) {
			std::cout << "Error: Invalid chunk size format" << std::endl;
			return REQUEST_ERROR;
		}
		if (is_full_data_binary == 3)
		{
			start_string = start_end_chunk + 1; // +1 or +2
			
		}
		else if (is_full_data_binary == 2)
		{
			start_string = all_chunk_size_end + 1; // +1 or +2
		}
		if (start_string + real_size_chunk > request.length()) {
			std::cout << "Error: Insufficient data for the request length" << std::endl;
			return REQUEST_ERROR;
		}
		if (is_full_data_binary == 2)
			end_string =  request.find("\r", start_string);
		else if (is_full_data_binary == 3)
			end_string =  request.find("\n", start_string);
		if (end_string == std::string::npos)
			return REQUEST_INCOMPLETE;


		string_size = end_string - start_string;
		std::cout << "string size = " << string_size<< "real size chunk = " << real_size_chunk << std::endl;
		if (real_size_chunk > (long)string_size)
		{
			std::cout << "Error: CHUNK VALUE > size of the value" << std::endl;
			return (REQUEST_ERROR);
		}
		if (real_size_chunk < (long)string_size)
		{
			std::cout << "Error: Insufficient data for chunk " << std::endl;
			return (REQUEST_ERROR);
		}
		body.append(request.substr(start_string, string_size));
		
		if (body.length() > MAX_BODY_SIZE)
			return REQUEST_ERROR;
		// setup new position
		start_chunk = end_string + 1;
		if (is_full_data_binary == 2)
		{
			size_t final_crlf = request.find("\r\n", start_end_chunk + 2);
			if (final_crlf == std::string::npos) {
				std::cout << "Error: Missing final CRLF" << std::endl;
				return REQUEST_INCOMPLETE;
			}
			buffer.clear();
			std::cout << "Chunked transfer complete, body length: " << body.length() << std::endl;
			return REQUEST_OK;
		}
	}
	std::cout << "Request chunk incomplete" << std::endl;
	return REQUEST_INCOMPLETE;
}
