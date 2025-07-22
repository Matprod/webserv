/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:58:27 by allan             #+#    #+#             */
/*   Updated: 2025/07/22 16:27:10 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include "../Request/Request.hpp"

#define MULTIPART 2
#define SINGLEPART 3

struct Response {
	std::string version;
	int statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;	

	std::string responseToString() const;
	void createResponse(unsigned int code, const std::string& reason);
};

template <typename T>
std::string toString(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

struct File {
	std::string fileName;
	std::string filePath;
	std::string fileData;
	std::string boundary;
	size_t length;

	Response response;	
	
	int getFileName(const std::string& uri);
	int getFileData(const Request& request);
	bool isValidContentLength(const std::string& contentLength);
	int createFile(const std::string& body);
	bool fileExists(const std::string fullPath) const;
	std::string generateUniqueFilename();
};

Response buildResponse(const Request& request);
Response handleGet();
Response handlePost(const Request& request);
Response handleDelete();
std::string getStatusMessage(int statusCode);

#endif