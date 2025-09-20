#include "srcs/Config/Config.hpp"
#include <iostream>

int main() {
    try {
        std::vector<ServerConfig> servers = parseConfigFile("config/good/webserv.conf");
        std::cout << "Configuration chargée avec succès!" << std::endl;
        std::cout << "Nombre de serveurs: " << servers.size() << std::endl;
        
        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "Serveur " << i << ":" << std::endl;
            std::cout << "  Port: " << servers[i].port << std::endl;
            std::cout << "  Nombre de locations: " << servers[i].locations.size() << std::endl;
            
            for (size_t j = 0; j < servers[i].locations.size(); ++j) {
                std::cout << "    Location " << j << ": " << servers[i].locations[j].path << std::endl;
                std::cout << "      Root: " << servers[i].locations[j].root << std::endl;
                std::cout << "      CGI extensions: " << servers[i].locations[j].cgi_extensions.size() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
