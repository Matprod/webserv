/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 15:54:17 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/21 18:39:52 by mvoisin          ###   ########.fr       */
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
LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations, bool &use_location);
Response executeCGI(const Request& req, const LocationConfig* loc, const ServerConfig* server);