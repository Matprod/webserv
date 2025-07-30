/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 15:54:17 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/23 19:08:40 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Response/Response.hpp"
#include "wait.h"

struct Response;

std::string getFileExtension(const std::string& uri);
std::string getScriptName(const std::string& uri);
std::string getQueryString(const std::string& uri);
std::string getPathInfo(const std::string& uri, const std::string& loc_path, const std::string& script_name);
ServerConfig* findMatchingServer(const Request& req, const std::vector<ServerConfig>& servers);
LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations);
Response executeCGI(const Request& req, const LocationConfig& loc, const ServerConfig& server);