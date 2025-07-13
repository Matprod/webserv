/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/12 20:15:57 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/13 21:41:15 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdlib>
# include <iostream>


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

	LocationConfig();
	virtual ~LocationConfig();
};

