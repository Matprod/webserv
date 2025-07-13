/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 10:58:52 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/13 21:34:43 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once


#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

class Config {
private:
    std::vector<ServerConfig> servers;
    std::string trim(const std::string& str) const;
    std::vector<std::string> split(const std::string& str, char delimiter) const;
    unsigned long parseSize(const std::string& size_str) const;
    bool parseDirective(const std::vector<std::string>& tokens, ServerConfig* current_server, LocationConfig* current_location);
    bool parseFile(const std::string& path);
    bool parseServerDirective(const std::string& directive, const std::vector<std::string>& values, ServerConfig* srv);
    bool parseLocationDirective(const std::string& directive, const std::vector<std::string>& values, LocationConfig* srv);

public:
    Config(const std::string& path);
    virtual ~Config();
    std::vector<ServerConfig>& getServers() { return servers; } //Interdit de declarer direct dans .h
	const std::vector<ServerConfig>& getServers() const { return servers; }
    bool error;
};

#include "../../includes/Webserv.hpp"

#endif