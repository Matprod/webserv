/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 10:58:52 by Matprod           #+#    #+#             */
/*   Updated: 2025/06/26 21:13:47 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../../includes/Webserv.hpp"

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>


class LocationConfig {
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

class ServerConfig {
public:
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::string root;
    std::map<int, std::string> error_pages;
    unsigned long max_body_size;
    std::vector<LocationConfig> locations;
    std::vector<std::string> index;

    ServerConfig();
    virtual ~ServerConfig();
};

class Config {
private:
    std::vector<ServerConfig> servers;
    std::string trim(const std::string& str) const;
    std::vector<std::string> split(const std::string& str, char delimiter) const;
    unsigned long parseSize(const std::string& size_str) const;
    bool parseDirective(const std::vector<std::string>& tokens, ServerConfig* current_server, LocationConfig* current_location);
    bool parseFile(const std::string& path);

public:
    Config(const std::string& path);
    virtual ~Config();
    const std::vector<ServerConfig>& getServers() const { return servers; }
    bool error;
};

#endif