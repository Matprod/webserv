/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:58:27 by allan             #+#    #+#             */
/*   Updated: 2025/07/18 18:37:30 by Matprod          ###   ########.fr       */
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


struct Response {
	std::string version;
	int statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;	

	std::string responseToString() const;
};

template <typename T>
std::string toString(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

Response buildResponse(const Request& request, const std::vector<ServerConfig>& servers);
Response handleGet();
Response handlePost(const Request& req);
Response handleDelete();
std::string getStatusMessage(int statusCode);

#endif