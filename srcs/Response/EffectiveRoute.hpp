/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EffectiveRoute.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 14:16:15 by adebert           #+#    #+#             */
/*   Updated: 2025/09/12 23:03:22 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EFFECTIVEROUTE_HPP
#define EFFECTIVEROUTE_HPP

#include <string>
#include <iostream>
#include "../Config/ServerConfig.hpp"
#include "../Config/LocationConfig.hpp"
#include "../Request/Request.hpp"

#define PATH_OK 0

struct EffectiveRoute {
    const ServerConfig* server;
    const LocationConfig* location;     // may be NULL if no location matched (but you should always have "/")
	bool useLocation;
    std::string root;              // resolved base dir (if using root)
    std::string alias;             // resolved alias (if using alias)
    bool use_alias;                // true iff alias is active
    bool autoindex;
    std::set<std::string> allow_methods;
    std::vector<std::string> index;
    std::string upload_path;       // may be empty if not set
    std::string location_prefix;   // loc->path
    int redirect_status;
    std::string redirect_url;
	std::string uri;
	bool isDir;
	bool closeConnection;

	bool createEffectiveRoute(const ServerConfig* srv, const LocationConfig* loc);
	bool createEffectiveRoute(const ServerConfig* srv);
	int createEffectivePath(std::string req_uri);
	int isValidPath(void);
};

std::ostream &operator<<(std::ostream &o, const EffectiveRoute&i);
std::string joinPaths(const std::string& base, const std::string& suffix);

#endif