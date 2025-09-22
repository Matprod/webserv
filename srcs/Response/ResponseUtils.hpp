/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseUtils.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 17:44:16 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/21 18:49:36 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Response.hpp"

#define BUFFER_SIZE 4096
struct Response;
ServerConfig* getMatchingServer(const Request& req, const std::vector<ServerConfig>& servers, Response& res);
LocationConfig* getMatchingLocation(const Request& req, const ServerConfig* server, Response& res, bool &use_location);
bool handleRedirect(const LocationConfig& loc, Response& res);
bool isCGIRequest(const LocationConfig *loc, const std::string& uri);
std::string readLine(int fd);
std::string readChunkedBody(int fd);