/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 15:54:17 by Matprod           #+#    #+#             */
/*   Updated: 2025/09/21 18:39:52 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "wait.h"
#include <map>
#include <ctime>
#include <string>
#include <vector>
#include "../Request/Request.hpp"

// Configuration CGI
#define CGI_TIMEOUT 20          // Timeout en secondes pour l'exécution des CGI
#define CGI_BUFFER_SIZE 4096    // Taille du buffer pour lire la sortie CGI

// Forward declarations
struct Response;
struct ServerConfig;
struct LocationConfig;

// Structure pour gérer les CGI en cours d'exécution
struct CgiProcess {
    pid_t pid;
    int clientFd;
    time_t startTime;
    int pipe_out_read;      // fd du pipe de lecture (que le serveur lit)
    std::string cgiOutput;
    bool isComplete;
    bool headersSent;       // Pour savoir si on a déjà envoyé les headers
    Request request;        // Copie de la requête originale
    const LocationConfig* location;
    const ServerConfig* server;
    
    CgiProcess() : pid(-1), clientFd(-1), startTime(0), pipe_out_read(-1), 
                   isComplete(false), headersSent(false), location(NULL), server(NULL) {}
};

// Map globale pour stocker les processus CGI en cours
// Clé = pipe_out_read fd (le fd surveillé par poll)
extern std::map<int, CgiProcess> g_cgiProcesses;

// Fonctions utilitaires
std::string getFileExtension(const std::string& uri);
std::string getScriptName(const std::string& uri);
std::string getQueryString(const std::string& uri);
std::string getPathInfo(const std::string& uri, const std::string& loc_path, const std::string& script_name);
ServerConfig* findMatchingServer(const Request& req, const std::vector<ServerConfig>& servers);
LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations, bool &use_location);

// Helpers internes
void prepareCGIEnvironment(std::vector<std::string>& env_vars, const Request& req, 
                          const std::string& scriptName, const std::string& scriptPath,
                          const std::string& queryString, const std::string& pathInfo,
                          const LocationConfig* loc, const ServerConfig* server);

// Nouvelle API asynchrone
int startCGIAsync(const Request& req, const LocationConfig* loc, const ServerConfig* server, int clientFd);
void handleCGIReadEvent(int pipe_fd);
void checkCGITimeouts();
void cleanupCGIProcess(int pipe_fd);
Response finalizeCGIResponse(CgiProcess& cgiProc);
