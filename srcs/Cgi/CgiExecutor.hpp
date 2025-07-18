/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 15:54:17 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/18 17:56:37 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Response/Response.hpp"
#include "wait.h"

struct Response;

std::string getFileExtension(const std::string& uri);
std::string getScriptName(const std::string& uri);
ServerConfig* findMatchingServer(const Request& req, const std::vector<ServerConfig>& servers);
LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations);
Response executeCGI(const Request& req, const LocationConfig& loc, const ServerConfig& server);
