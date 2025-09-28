/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 17:04:21 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/28 16:16:56 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../../includes/Webserv.hpp"
#include "RequestBody.hpp"

enum RequestStatus {
	REQUEST_OK = 0,
	REQUEST_INCOMPLETE = 1,
	REQUEST_ERROR = -1
};

struct Request {
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	const ServerConfig* config;
	bool use_location;
	bool closeConnection;
};

int parse_request(int socket, Request& req, std::map<int, std::string>& buffers, std::map<int, time_t> lastActivity);
int sanitizeRequestUri(Request& req);
std::string to_lower(const std::string& str);
void printRequest(const Request& req);
std::ostream &operator<<(std::ostream &o, const Request &i);