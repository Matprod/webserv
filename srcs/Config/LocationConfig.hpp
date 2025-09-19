/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:15:57 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/19 16:24:20 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>
# include <iostream>

#include "ServerConfig.hpp"


class LocationConfig {// Need to be in canonical form ?
	public:
	std::set<std::string> allow_methods;
	std::string root;
	bool autoindex;
	std::vector<std::string> index;
	std::map<std::string, std::string> cgi_extensions;
	std::string upload_path;
	std::string path;
	int redirect_status;
	std::string redirect_url;
	std::string alias;
	bool has_root;
	bool has_alias;

	LocationConfig();
	LocationConfig(const LocationConfig& src);
	LocationConfig& operator=(const LocationConfig& rhs);
	virtual ~LocationConfig();
};

std::ostream &operator<<(std::ostream &o, const LocationConfig &i);