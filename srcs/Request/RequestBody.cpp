/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBody.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 10:43:19 by Matprod           #+#    #+#             */
/*   Updated: 2025/08/19 01:52:48 by Matprod          ###   ########.fr       */
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

//parse_body chunk qui prend en compte les ch\nks envoyer en entier
/*int parse_body_chunked(const std::string& request, size_t body_start, Request& req, std::map<int, std::string>& buffers, int socket) {
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
	//size_t count = 0;
	//setup all chunk and start chunk;
	//std::cout << "Request = " << req << std::endl;
	std::cout << "request.length() = " << request.length() << std::endl;
	print_request(request.substr(pos, request.length() - pos).c_str());
	//std::cout << "pos start pos = "<< pos << " | pos start value ===== " << request.substr(pos, request.length()) << std::endl;
	if (pos >= request.length()) {
		std::cout << "Error: Invalid body start position" << std::endl;
		return REQUEST_ERROR;
	}
	all_chunk_size_end = request.find("\n", pos);
	std::cout << "all_chunk_size_end = " << all_chunk_size_end << std::endl;
	if (all_chunk_size_end == std::string::npos) {
		std::cout << "Error: Chunk size end marker not found" << std::endl;
		return REQUEST_INCOMPLETE;
	}
	//count = count_crlf_sequences(request.substr(pos, request.length() - pos).c_str());
	//std::cout << "Nombre de séquences CRLF : " << count << std::endl;
	int is_full_data_binary = check_final_separator(request, pos);// 3 = \n\r\n | 4 = \r\n | 1 = false
	//std::cout << "FULL DATA BINARY = " << is_full_data_binary << std::endl;
	if (is_full_data_binary == 1 || is_full_data_binary == 2)
		return REQUEST_ERROR;
	all_chunk_size = strtol(request.substr(pos, all_chunk_size_end - pos).c_str(), &endptr, 16);
	std::cout << "all_chunk_size = " << all_chunk_size << std::endl;
	if (all_chunk_size < 0) {
		std::cout << "Error: Invalid chunk size format" << std::endl;
		return REQUEST_ERROR;
	}
	if (all_chunk_size < 0)
		return REQUEST_ERROR;
	start_chunk = all_chunk_size_end + 1;
	std::cout << "start_chunk pos : " << start_chunk << " | start_chunk value : "<< request.substr(start_chunk, 1) << std::endl;
	while (start_chunk < request.length()) {
		// setup chunk size 
		if (start_chunk >= request.length()) {
			std::cout << "Error: Invalid chunk start position" << std::endl;
			return REQUEST_INCOMPLETE;
		}
		start_end_chunk = request.find("\n", start_chunk);
		std::cout << "start_end_chunk = " << start_end_chunk << std::endl;
		if (start_end_chunk == std::string::npos) {
			std::cout << "Error: Chunk size end marker not found" << std::endl;
			return REQUEST_INCOMPLETE;
		}
		size_chunk_digit = start_end_chunk - start_chunk;
		std::cout << "size_chunk_digit = " << size_chunk_digit << std::endl;
		if (size_chunk_digit <= 0) {
			std::cout << "Error: Invalid chunk size length" << std::endl;
			return REQUEST_ERROR;
		}
		//endptr = NULL;
		if (is_full_data_binary == 3)
		{
			real_size_chunk = strtol(request.substr(start_chunk, size_chunk_digit).c_str(), &endptr, 16);
			std::cout << "test de request.substr(start_chunk, size_chunk_digit) = " << request.substr(start_chunk, size_chunk_digit) << std::endl;
			std::cout << "real_size_chunk = " << real_size_chunk << std::endl;
			
		}
		else if (is_full_data_binary == 2)
		{
			real_size_chunk  = all_chunk_size;
			std::cout << "real_size_chunk = " << real_size_chunk << std::endl;
		}
		if (real_size_chunk < 0) {
			std::cout << "Error: Invalid chunk size format" << std::endl;
			return REQUEST_ERROR;
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
		if (real_size_chunk < 0) {
    		std::cout << "Error: Invalid chunk size format" << std::endl;
    		return REQUEST_ERROR;
		}

		// Vérifier que endptr pointe bien sur un CRLF
		 if (*endptr != '\0') {
			std::cout << "endptr = " << *endptr << std::endl;
			std::cout << "Error: invalid chunk size terminator" << std::endl;
			return REQUEST_ERROR;
		} 
		// setup string for body
		if (is_full_data_binary == 3)
		{
			start_string = start_end_chunk + 1; // +1 or +2
			std::cout << "start_string pos : " << start_string;
			std::cout << " | start_string value : "<< request.substr(start_string, 1) << std::endl;
			
		}
		else if (is_full_data_binary == 2)
		{
			start_string = all_chunk_size_end + 1; // +1 or +2
			std::cout << "start_string pos : " << start_string;
			std::cout << " | start_string value : "<< request.substr(start_string, 1) << std::endl;
		}
		if (start_string + real_size_chunk > request.length()) {
			std::cout << "Error: Insufficient data for chunk | start_string = " << start_string << " real_size_chunk = " << real_size_chunk << " request_lenght = " << request.length() << std::endl;
			return REQUEST_INCOMPLETE;
		}
		if (is_full_data_binary == 2)
			end_string =  request.find("\r", start_string);
		else if (is_full_data_binary == 3)
			end_string =  request.find("\n", start_string);
		if (end_string == std::string::npos)
			return REQUEST_INCOMPLETE;

		string_size = end_string - start_string;
		std::cout << 
		body.append(request.substr(start_string, string_size));
		std::cout << "BODY APPEND (start_string, string size) = " << request.substr(start_chunk, size_chunk_digit) << std::endl;
		
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
	std::cout << "Request chunk incomplete 2" << std::endl;
	return REQUEST_INCOMPLETE;
}*/

/*int parse_body_chunked(const std::string& request, size_t body_start, Request& req, std::map<int, std::string>& buffers, int socket) {
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
	size_t count = 0;
	//setup all chunk and start chunk;
	//std::cout << "Request = " << req << std::endl;
	std::cout << "request.length() = " << request.length() << std::endl;
	//std::cout << "pos start pos = "<< pos << " | pos start value ===== " << request.substr(pos, request.length()) << std::endl;
	if (pos >= request.length()) {
		std::cout << "Error: Invalid body start position" << std::endl;
		return REQUEST_ERROR;
	}
	all_chunk_size_end = request.find("\n", pos);
	if (all_chunk_size_end == std::string::npos) {
		std::cout << "Error: Chunk size end marker not found" << std::endl;
		return REQUEST_INCOMPLETE;
	}
	//count = count_crlf_sequences(request.substr(pos, request.length() - pos).c_str());
	std::cout << "Nombre de séquences CRLF : " << count << std::endl;
	int is_full_data_binary = check_final_separator(request, pos);// 3 = \n\r\n | 4 = \r\n | 1 = false
	if (is_full_data_binary == 1)
		return REQUEST_ERROR;

	
	all_chunk_size = strtol(request.substr(pos, all_chunk_size_end - pos).c_str(), &endptr, 16);
	std::cout << "all_chunk_size = " << all_chunk_size << std::endl;
	if (all_chunk_size < 0) {
		std::cout << "Error: Invalid chunk size format" << std::endl;
		return REQUEST_ERROR;
	}
	if (all_chunk_size < 0)
		return REQUEST_ERROR;
	start_chunk = all_chunk_size_end + 1;
	while (start_chunk < request.length()) {
		// setup chunk size 
		if (start_chunk >= request.length()) {
			std::cout << "Error: Invalid chunk start position" << std::endl;
			return REQUEST_INCOMPLETE;
		}
		std::cout << "start_chunk pos : " << start_chunk << " | start_chunk value : "<< request.substr(start_chunk, 1) << std::endl;
		start_end_chunk = request.find("\n", start_chunk);
		std::cout << "start_end_chunk = " << start_end_chunk << std::endl;
		if (start_end_chunk == std::string::npos) {
			std::cout << "Error: Chunk size end marker not found" << std::endl;
			return REQUEST_INCOMPLETE;
		}
		size_chunk_digit = start_end_chunk - start_chunk;
		std::cout << "size_chunk_digit = " << size_chunk_digit << std::endl;
		if (size_chunk_digit <= 0) {
			std::cout << "Error: Invalid chunk size length" << std::endl;
			return REQUEST_ERROR;
		}
		endptr = NULL;
		real_size_chunk = strtol(request.substr(start_chunk, size_chunk_digit).c_str(), &endptr, 16);
		std::cout << "test de request.substr(start_chunk, size_chunk_digit) = " << request.substr(start_chunk, size_chunk_digit) << std::endl;
		std::cout << "real_size_chunk = " << real_size_chunk << std::endl;
		if (real_size_chunk < 0) {
			std::cout << "Error: Invalid chunk size format" << std::endl;
			return REQUEST_ERROR;
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
		if (real_size_chunk < 0) {
    		std::cout << "Error: Invalid chunk size format" << std::endl;
    		return REQUEST_ERROR;
		}

		// Vérifier que endptr pointe bien sur un CRLF
		if (*endptr != '\0') {
			std::cout << "endptr = " << *endptr << std::endl;
			std::cout << "Error: invalid chunk size terminator" << std::endl;
			return REQUEST_ERROR;
		}
		// setup string for body
		start_string = start_end_chunk + 1; // +1 or +2
		std::cout << "start_string pos : " << start_string << std::endl;
		std::cout << " | start_string value : "<< request.substr(start_string, 1) << std::endl;
		if (start_string + real_size_chunk > request.length()) {
			std::cout << "Error: Insufficient data for chunk | start_string = " << start_string << " real_size_chunk = " << real_size_chunk << " request_lenght = " << request.length() << std::endl;
			return REQUEST_INCOMPLETE;
		}
		end_string =  request.find("\n", start_string);
		if (end_string == std::string::npos)
			return REQUEST_INCOMPLETE;

		string_size = end_string - start_string;
		body.append(request.substr(start_string, string_size));
		
		if (body.length() > MAX_BODY_SIZE)
			return REQUEST_ERROR;
		// setup new position
		start_chunk = end_string + 1;
	}
	std::cout << "Request chunk incomplete 2" << std::endl;
	return REQUEST_INCOMPLETE;
}*/

/*int parse_body_chunked(const std::string& request, size_t body_start, Request& req, std::map<int, std::string>& buffers, int socket) {
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
	if (pos >= request.length()) {
		std::cout << "Error: Invalid body start position" << std::endl;
		return REQUEST_ERROR;
	}

	all_chunk_size_end = request.find("\n", pos);
	if (all_chunk_size_end == std::string::npos) {
		std::cout << "Error: Chunk size end marker not found" << std::endl;
		return REQUEST_INCOMPLETE;
    }
	std::cout << "chunk_size_end :" << all_chunk_size_end << std::endl;
	all_chunk_size = strtol(request.substr(pos, all_chunk_size_end - pos).c_str(), &endptr, 16);
	if (all_chunk_size < 0) {
        std::cout << "Error: Invalid chunk size format" << std::endl;
        return REQUEST_ERROR;
    }

	std::cout << "all chunk size (at first in hexa but nox in decimal) : " << all_chunk_size << std::endl;
	if (all_chunk_size < 0)
		return REQUEST_ERROR;
	start_chunk = all_chunk_size_end + 1;
	while (start_chunk < request.length()) {
		// setup chunk size 
		if (start_chunk >= request.length()) {
            std::cout << "Error: Invalid chunk start position" << std::endl;
            return REQUEST_INCOMPLETE;
        }
		std::cout << "start_chunk pos : " << start_chunk << " | start_chunk value : "<< request.substr(start_chunk, 1) << std::endl;
		start_end_chunk = request.find("\n", start_chunk);
		if (start_end_chunk == std::string::npos) {
            std::cout << "Error: Chunk size end marker not found" << std::endl;
            return REQUEST_INCOMPLETE;
        }
		std::cout << "start end chunk pos = " << start_end_chunk << " | start end chunk value : "<< request.substr(start_end_chunk, 1) << std::endl;
		size_chunk_digit = start_end_chunk - start_chunk;
		if (size_chunk_digit <= 0) {
            std::cout << "Error: Invalid chunk size length" << std::endl;
            return REQUEST_ERROR;
        }
		std::cout << "size chunk = " << size_chunk_digit << std::endl;
		real_size_chunk = strtol(request.substr(start_chunk, size_chunk_digit).c_str(), &endptr, 16);
		if (real_size_chunk < 0) {
            std::cout << "Error: Invalid chunk size format" << std::endl;
            return REQUEST_ERROR;
        }

        // Cas de fin (chunk size = 0)
        if (real_size_chunk == 0) {
            size_t final_crlf = request.find("\r\n", start_end_chunk + 2);
            if (final_crlf == std::string::npos) {
                std::cout << "Error: Missing final CRLF" << std::endl;
                return REQUEST_INCOMPLETE;
            }
            
            // Nettoyer le buffer et retourner OK
            buffer.erase(0, final_crlf + 2);
            std::cout << "Chunked transfer complete, body length: " << body.length() << std::endl;
            return REQUEST_OK;
        }
		
		if (real_size_chunk < 0 || *endptr != '\0')
		{
			std::cout << "REAL SIZE CHUNK NEGATIV OR ERROR ENDPTR " << body.length() << std::endl;
			return REQUEST_ERROR;
		}
			
		// setup string for body
		start_string = start_end_chunk + 1; // ptet + 1
		// Vérifier si assez de données sont disponibles
        if (start_string + real_size_chunk > request.length()) {
            std::cout << "Error: Insufficient data for chunk" << std::endl;
            return REQUEST_INCOMPLETE;
        }
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
}*/
