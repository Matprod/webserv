/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:44:20 by allan             #+#    #+#             */
/*   Updated: 2025/07/12 17:20:35 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

//A Request is the possible Action a client can make on the server GET...
//This file will be parsing kind of like the config file

int get_request(int socket, Request& req) {
    std::string request;
    char buffer[1024];
    int bytes_read;
    bool headers_complete = false;
    size_t content_length = 0;
    size_t body_start = 0;

    // Lire les données depuis la socket
    while ((bytes_read = recv(socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0'; // Assurer que le buffer est terminé par un null
        request.append(buffer, bytes_read);

        // Vérifier si les en-têtes sont complets
        if (!headers_complete) {
            size_t header_end = request.find("\r\n\r\n");
            if (header_end != std::string::npos) {
                headers_complete = true;
                body_start = header_end + 4;

                // Parser la ligne de requête
                std::istringstream iss(request.substr(0, header_end));
                std::string request_line;
                if (!std::getline(iss, request_line)) {
                    std::cerr << "Erreur: Ligne de requête manquante" << std::endl;
                    return ERROR;
                }

                // Extraire méthode, URI, version
                size_t method_end = request_line.find(' ');
                if (method_end == std::string::npos) {
                    std::cerr << "Erreur: Ligne de requête mal formée" << std::endl;
                    return ERROR;
                }
                req.method = request_line.substr(0, method_end);
                size_t uri_end = request_line.find(' ', method_end + 1);
                if (uri_end == std::string::npos) {
                    std::cerr << "Erreur: URL ou version manquante" << std::endl;
                    return ERROR;
                }
                req.uri = request_line.substr(method_end + 1, uri_end - method_end - 1);
                req.version = request_line.substr(uri_end + 1);
                // Nettoyer la version
                size_t version_end = req.version.find_last_not_of(" \t\r\n");
                if (version_end != std::string::npos) {
                    req.version = req.version.substr(0, version_end + 1);
                }

                // Parser les en-têtes
                std::string line;
                while (std::getline(iss, line) && line != "\r") {
                    size_t colon_pos = line.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string key = line.substr(0, colon_pos);
                        std::string value = line.substr(colon_pos + 1);
                        // Nettoyer les espaces autour de la valeur
                        size_t start = value.find_first_not_of(" \t");
                        size_t end = value.find_last_not_of(" \t");
                        if (start != std::string::npos && end != std::string::npos) {
                            value = value.substr(start, end - start + 1);
                        } else {
                            value = "";
                        }
                        req.headers[key] = value;
                    }
                }

                // Vérifier Content-Length pour le corps
                if (req.headers.find("Content-Length") != req.headers.end()) {
                    char* endptr;
                    content_length = strtol(req.headers["Content-Length"].c_str(), &endptr, 10);
                    if (*endptr != '\0' || content_length < 0) {
                        std::cerr << "Erreur: Content-Length invalide" << std::endl;
                        return ERROR;
                    }
                } else {
                    content_length = 0;
                }
            }
        }

        // Vérifier si la requête complète est reçue
        if (headers_complete && request.length() >= body_start + content_length) {
            // Extraire le corps si présent
            if (content_length > 0) {
                req.body = request.substr(body_start, content_length);
            }
            return 0; // Requête complète
        }
    }

    if (bytes_read < 0) {
        std::cerr << "Erreur lors de la lecture de la socket" << std::endl;
        return ERROR;
    }

    return ERROR; // Requête incomplète ou connexion fermée
}