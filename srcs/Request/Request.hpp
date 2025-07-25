/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 17:04:21 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/18 10:49:22 by Matprod          ###   ########.fr       */
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
};

int parse_request(int socket, Request& req, std::map<int, std::string>& buffers, std::map<int, time_t> lastActivity);
std::string to_lower(const std::string& str);