/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 17:04:21 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/11 15:15:11 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../../includes/Webserv.hpp"
#include "RequestBody.hpp"

#define MAX_BODY_SIZE 1048576 // 1 Mo max

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
};

int parse_request(int socket, Request& req, std::map<int, std::string>& buffers, std::map<int, time_t> lastActivity);
int sanitizeRequestUri(Request& req);
std::string to_lower(const std::string& str);
void printRequest(const Request& req);
std::ostream &operator<<(std::ostream &o, const Request &i);