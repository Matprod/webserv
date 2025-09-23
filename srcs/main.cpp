/* ************************************************************************** */
/*                                                                            */
/*                                                    :::      ::::::::       */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/22 10:58:05 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/15 13:41:18by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include "../includes/Webserv.hpp"
#include "./Server/Socket.hpp"
#include "./Server/Server.hpp"

bool printConfig(const Config& config) {
    const std::vector<ServerConfig>& servers = config.getServers();
    if (servers.empty()) {
        std::cout << "Aucun serveur configure." << std::endl;
        return ERROR;
    }

    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerConfig& server = servers[i];
        std::cout << "Serveur " << i + 1 << ":" << std::endl;
        std::cout << "  Port: " << server.port << std::endl;
        std::cout << "  Host: " << (server.host.empty() ? "INADDR_ANY" : server.host) << std::endl;
        std::cout << "  Server Names: ";
        for (size_t j = 0; j < server.server_names.size(); ++j) {
            std::cout << server.server_names[j];
            if (j < server.server_names.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
        std::cout << "  Root: " << server.root << std::endl;
        std::cout << "  Index: " << (server.index.empty() ? "none" : server.index) << std::endl;
        std::cout << "  Max Body Size: " << server.max_body_size << " bytes" << std::endl;
        std::cout << "  Error Pages: ";
        for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); it != server.error_pages.end(); ++it) {
            std::cout << it->first << " -> " << it->second << ", ";
        }
        std::cout << std::endl;

        std::cout << "  Locations:" << std::endl;
        for (size_t j = 0; j < server.locations.size(); ++j) {
			if (j >= 1)
				std::cout << std::endl;
            const LocationConfig& loc = server.locations[j];
            std::cout << "  Path: " << loc.path << std::endl;
            std::cout << "  Allow Methods: ";
            for (std::set<std::string>::const_iterator it = loc.allow_methods.begin(); it != loc.allow_methods.end(); ++it) {
                std::cout << *it << ", ";
            }
            std::cout << std::endl;
            std::cout << "  Root: " << loc.root << std::endl;
            std::cout << "  Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
            std::cout << "  Index: " << (loc.index.empty() ? "none" : loc.index) << std::endl;
            std::cout << "  Redirect Status: " << (loc.redirect_status) << std::endl; // À corriger avec stringstream (voir ci-dessous)
            std::cout << "  Redirect URL: " << (loc.redirect_url.empty() ? "none" : loc.redirect_url) << std::endl;
            std::cout << "  CGI Extensions: ";
            for (std::map<std::string, std::string>::const_iterator it = loc.cgi_extensions.begin(); it != loc.cgi_extensions.end(); ++it) {
                std::cout << it->first << " -> " << it->second << ", ";
            }
            std::cout << std::endl;
            std::cout << "  Upload Path: " << (loc.upload_path.empty() ? "none" : loc.upload_path) << std::endl;
            std::cout << "  Alias: " << (loc.alias.empty() ? "none" : loc.alias) << std::endl; // Nouvel affichage pour alias
        }
        std::cout << std::endl;
    }
    return SUCCESS;
}

int main(int argc, char *argv[]) {
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, signalHandler);
	std::string config_path = (argc > 1) ? argv[1] : "default.conf";
	//STEP 1: Get/Parse Server Config Files
	Config config(config_path);

	if (config.error != ERROR)
    {
		/* std::cout << "Configuration parsee a partir de : " << config_path << std::endl;
		if (printConfig(config) == ERROR)
			return (ERROR); */
			
		//STEP 2: Create Sockets for each server
		setupSockets(config.getServers());
		serverLoop(config.getServers());
	}
	else
		return (ERROR);

	return SUCCESS;
}
