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

// Forward declarations
struct Response;
struct Request;
struct ServerConfig;
struct LocationConfig;

// Structure pour gérer les CGI en cours d'exécution
struct CgiProcess {
    pid_t pid;
    int clientFd;
    time_t startTime;
    int pipe_out[2];
    std::string cgiOutput;
    bool isComplete;
    Response* response;
    
    CgiProcess() : pid(-1), clientFd(-1), startTime(0), isComplete(false), response(NULL) {
        pipe_out[0] = -1;
        pipe_out[1] = -1;
    }
};

// Map globale pour stocker les processus CGI en cours
extern std::map<pid_t, CgiProcess> g_cgiProcesses;

std::string getFileExtension(const std::string& uri);
std::string getScriptName(const std::string& uri);
std::string getQueryString(const std::string& uri);
std::string getPathInfo(const std::string& uri, const std::string& loc_path, const std::string& script_name);
ServerConfig* findMatchingServer(const Request& req, const std::vector<ServerConfig>& servers);
LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations, bool &use_location);
Response executeCGI(const Request& req, const LocationConfig* loc, const ServerConfig* server, int clientFd);
void checkCgiProcesses();
void cleanupCgiProcess(pid_t pid);