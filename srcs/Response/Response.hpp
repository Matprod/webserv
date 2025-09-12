/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:58:27 by allan             #+#    #+#             */
/*   Updated: 2025/09/12 22:20:30 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include <dirent.h>
#include "../Request/Request.hpp"
#include "../Cgi/CgiExecutor.hpp"
#include "ResponseUtils.hpp"
#include "EffectiveRoute.hpp"

#define GET 1
#define POST 2
#define DELETE 3

#define MULTIPART 2
#define SINGLEPART 3

struct Response {
	std::string version;
	int statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;	
	bool closingConnection;

	std::string responseToString() const;
	void createResponse(unsigned int code, const std::string& reason);
	void setHeader(std::string header, std::string content);
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
	void createDeleteResponse(const int err);
	int checkContentType(const std::string &contentType) const;
};

bool isMethodAllowed(int method, std::set<std::string> allow_methods);
Response buildResponse(const Request& request, const std::vector<ServerConfig>& servers);
Response handleGet(const Request& request, EffectiveRoute& eff);
Response handleIndex(const EffectiveRoute& eff);
Response createIndexResponse(int fd, bool closeConnection);
Response handleAutoIndex(const EffectiveRoute& eff);
Response createAutoIndexResponse(const EffectiveRoute& eff);
Response handlePost(const Request& request, const EffectiveRoute& eff);
Response handleDelete(const Request& request, const EffectiveRoute& eff);
std::string getStatusMessage(int statusCode);
int checkRequestVersion(const std::string& version, Response& response);
bool shouldConnectionBeClosed(const std::map<std::string, std::string>& headers);

#endif