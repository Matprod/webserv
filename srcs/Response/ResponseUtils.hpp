/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseUtils.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 17:44:16 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/22 18:04:39 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Response.hpp"

struct Response;
ServerConfig* getMatchingServer(const Request& req, const std::vector<ServerConfig>& servers, Response& res);
LocationConfig* getMatchingLocation(const Request& req, ServerConfig& server, Response& res);
bool handleRedirect(const LocationConfig& loc, Response& res);
bool isCGIRequest(const LocationConfig& loc, const std::string& uri);
