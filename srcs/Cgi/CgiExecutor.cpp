/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.cpp                                    %+:%+    %+:%+    %+:%+ */
/*                                                    %+:%+ %+:%+ %+:%+       */
/*   By: Matprod <matprod42@gmail.com>              %+:%+ %+:%+ %+:%+        */
/*                                                %+:+%+:%+           */
/*   Created: 2025/07/18 15:59:09 by Matprod           %#+    %#+             */
/*   Updated: 2025/07/30 16:45:00 by Matprod          ###   fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiExecutor.hpp"
#include "../Response/Response.hpp"
#include <fcntl.h>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <sstream>

// Définition de la map globale pour les processus CGI
// Clé = pipe_out_read fd (le fd surveillé par poll)
std::map<int, CgiProcess> g_cgiProcesses;

std::string getFileExtension(const std::string& uri) {
	size_t dot_pos = uri.find_last_of('.');
	if (dot_pos == std::string::npos || dot_pos == uri.size() - 1)
		return "";
	return uri.substr(dot_pos);
}

std::string getScriptName(const std::string& uri) {
	size_t last_slash = uri.find_last_of('/');
	size_t query_pos = uri.find('?');
	if (last_slash == std::string::npos)
		return (query_pos != std::string::npos) ? uri.substr(0, query_pos) : uri;
	size_t end_pos = (query_pos != std::string::npos) ? query_pos : uri.length();
	return uri.substr(last_slash + 1, end_pos - last_slash - 1);
}

std::string getPathInfo(const std::string& uri, const std::string& loc_path, const std::string& script_name) {
	std::string clean_loc_path = loc_path;
	if (!clean_loc_path.empty() && clean_loc_path[clean_loc_path.length() - 1] == '/') {
		clean_loc_path.erase(clean_loc_path.length() - 1);
	}
	if (uri.find(clean_loc_path) != 0)
		return "";
	std::string relative_path = uri.substr(clean_loc_path.length());
	if (!relative_path.empty() && relative_path[0] == '/')
		relative_path.erase(0, 1);
	size_t script_pos = relative_path.find(script_name);
	if (script_pos != 0)
		return "";
	std::string after_script = relative_path.substr(script_pos + script_name.length());
	return after_script; // Return empty string if no extra path, no forced "/"
}

std::string getQueryString(const std::string& uri) {
	size_t query_pos = uri.find('?');
	if (query_pos != std::string::npos)
		return uri.substr(query_pos + 1);
	return "";
}

ServerConfig* findMatchingServer(const Request& req, const std::vector<ServerConfig>& servers) {
	for (size_t i = 0; i < servers.size(); ++i) {
		for (std::vector<std::string>::const_iterator name = servers[i].server_names.begin(); name != servers[i].server_names.end(); ++name) {
			if (*name == req.headers.at("host")) {
				return const_cast<ServerConfig*>(&servers[i]);
			}
		}
	}
	return servers.empty() ? NULL : const_cast<ServerConfig*>(&servers[0]);
}

LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations, bool &use_location) {
	LocationConfig* best_match = NULL;
	use_location = false;
	size_t longest_match = 0;
	for (size_t i = 0; i < locations.size(); ++i) {
		if (uri.find(locations[i].path) == 0 && locations[i].path.length() > longest_match) {
			best_match = const_cast<LocationConfig*>(&locations[i]);
			use_location = true;
			longest_match = locations[i].path.length();
		}
	}
	return best_match;
}


// =============================================================================
// HELPERS INTERNES
// =============================================================================

void prepareCGIEnvironment(std::vector<std::string>& env_vars, const Request& req, 
                          const std::string& scriptName, const std::string& scriptPath,
                          const std::string& queryString, const std::string& pathInfo,
                          const LocationConfig* loc, const ServerConfig* server) {
	// Variables CGI obligatoires (RFC 3875)
	env_vars.push_back("REQUEST_METHOD=" + req.method);
	env_vars.push_back("QUERY_STRING=" + queryString);
	env_vars.push_back("CONTENT_LENGTH=" + toString<size_t>(req.body.size()));
	env_vars.push_back("CONTENT_TYPE=" + (req.headers.count("content-type") ? req.headers.at("content-type") : ""));
	env_vars.push_back("SCRIPT_NAME=" + scriptName);
	env_vars.push_back("SCRIPT_FILENAME=" + scriptPath);
	env_vars.push_back("PATH_INFO=" + pathInfo);
	env_vars.push_back("PATH_TRANSLATED=" + loc->root + pathInfo);
	env_vars.push_back("SERVER_NAME=" + (server->server_names.empty() ? "localhost" : server->server_names[0]));
	env_vars.push_back("SERVER_PORT=" + toString<int>(server->port));
	env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env_vars.push_back("REDIRECT_STATUS=200");
	env_vars.push_back("SERVER_SOFTWARE=webserv/1.0");
	
	// Ajouter les headers HTTP comme variables d'environnement
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); 
	     it != req.headers.end(); ++it) {
		std::string env_name = "HTTP_" + to_lower(it->first);
		// Remplacer les tirets par des underscores
		for (size_t i = 0; i < env_name.length(); ++i) {
			if (env_name[i] == '-') 
				env_name[i] = '_';
		}
		env_vars.push_back(env_name + "=" + it->second);
	}
}

// =============================================================================
// PARSING DE LA REPONSE CGI
// =============================================================================

void parseCgiResponse(const std::string& cgiOutput, Response& res) {
/*     std::cout << "=== CGI OUTPUT DEBUG ===" << std::endl;
    std::cout << "Raw output length: " << cgiOutput.length() << std::endl;
    std::cout << "First 200 chars: " << cgiOutput.substr(0, 200) << std::endl; */
    
    // Chercher la fin des headers (double CRLF ou double LF)
    size_t header_end = cgiOutput.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        header_end = cgiOutput.find("\n\n");
        if (header_end != std::string::npos) {
            header_end += 2; // Compenser pour \n\n
        }
    } else {
        header_end += 4; // Compenser pour \r\n\r\n
    }

    //std::cout << "Header end position: " << header_end << std::endl;
    
    if (header_end == std::string::npos) {
        // Pas de headers CGI, traiter comme du contenu brut
        std::cout << "No CGI headers found, treating as raw content" << std::endl;
        res.statusCode = 200;
        res.statusMessage = getStatusMessage(200);
        res.body = cgiOutput;
        res.headers["Content-Type"] = "text/html";
        res.headers["Content-Length"] = toString<size_t>(res.body.size());
        return;
    }

	// Parser les headers CGI
	std::string headers_str = cgiOutput.substr(0, header_end);
	res.body = cgiOutput.substr(header_end);

	std::istringstream header_stream(headers_str);
	std::string line;
	bool status_set = false;

	while (std::getline(header_stream, line)) {
		// Supprimer le \r final si présent
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		if (line.empty()) break;

		size_t colon_pos = line.find(':');
		if (colon_pos == std::string::npos) continue;

		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);
		
		value.erase(0, value.find_first_not_of(" \t"));

		if (to_lower(key) == "status") {
			std::istringstream status_stream(value);
			std::string status_code_str;
			status_stream >> status_code_str;
			
			int status_code = atoi(status_code_str.c_str());
			if (status_code >= 100 && status_code < 600) {
				res.statusCode = status_code;
				std::string status_message;
				if (std::getline(status_stream, status_message)) {
					status_message.erase(0, status_message.find_first_not_of(" \t"));
					res.statusMessage = status_message;
				} else {
					res.statusMessage = getStatusMessage(status_code);
				}
				status_set = true;
			}
		} else {
			res.headers[key] = value;
		}
	}

	if (!status_set) {
		res.statusCode = 200;
		res.statusMessage = getStatusMessage(200);
	}

	if (res.headers.find("Content-Length") == res.headers.end()) {
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}
}


// =============================================================================
// NOUVELLE API ASYNCHRONE
// =============================================================================

// Lance un CGI de manière asynchrone et retourne le fd du pipe à surveiller
// Retourne -1 en cas d'erreur, le fd du pipe sinon
int startCGIAsync(const Request& req, const LocationConfig* loc, const ServerConfig* server, int clientFd) {
	std::string extension = getFileExtension(req.uri);
	if (loc->cgi_extensions.find(extension) == loc->cgi_extensions.end()) {
		std::cerr << "CGI Extension Not Supported: " << extension << std::endl;
		return -1;
	}

	std::string cgiProgram = loc->cgi_extensions.at(extension);
	std::string scriptName = getScriptName(req.uri);
	std::string rootPath;
	
	if (loc->has_root) {
		rootPath = loc->root;
	} else if (server->has_root) {
		rootPath = server->root;
	} else {
		std::cerr << "No root directory configured" << std::endl;
		return -1;
	}
	
	std::string scriptPath = rootPath + "/" + scriptName;
	std::string pathInfo = getPathInfo(req.uri, loc->path, scriptName);
	std::string queryString = getQueryString(req.uri);
	
	// Vérifier que le script existe et est exécutable
	if (access(scriptPath.c_str(), F_OK) != 0) {
		std::cerr << "CGI Script Not Found: " << scriptPath << std::endl;
		return -1;
	}

	if (access(scriptPath.c_str(), X_OK) != 0) {
		std::cerr << "CGI Script Not Executable: " << scriptPath << std::endl;
		return -1;
	}

	// Créer les pipes pour la communication
	int pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
		perror("pipe");
		if (pipe_in[0] >= 0) { close(pipe_in[0]); close(pipe_in[1]); }
		return -1;
	}

	// Mettre le pipe de lecture en mode non-bloquant
	int flags = fcntl(pipe_out[0], F_GETFL, 0);
	if (flags == -1 || fcntl(pipe_out[0], F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("fcntl O_NONBLOCK (pipe)");
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		return -1;
	}

	// Fork pour créer le processus CGI
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		return -1;
	}

	if (pid == 0) {
		// Processus enfant - exécuter le CGI
		
		// Rediriger stdin et stdout vers les pipes
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		
		// Fermer les extrémités inutiles des pipes
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);

		// Préparer les variables d'environnement CGI
		std::vector<std::string> env_vars;
		prepareCGIEnvironment(env_vars, req, scriptName, scriptPath, queryString, pathInfo, loc, server);

		// Convertir en char* pour execve
		std::vector<char*> envp;
		for (std::vector<std::string>::iterator it = env_vars.begin(); it != env_vars.end(); ++it) {
			envp.push_back(const_cast<char*>(it->c_str()));
		}
		envp.push_back(NULL);

		// Préparer les arguments pour execve
		char* argv[] = { const_cast<char*>(cgiProgram.c_str()), const_cast<char*>(scriptPath.c_str()), NULL };
		
		// Exécuter le programme CGI
		execve(cgiProgram.c_str(), argv, envp.data());
		
		// Si on arrive ici, execve a échoué
		std::cerr << "execve failed: " << cgiProgram << " (" << strerror(errno) << ")" << std::endl;
		exit(1);
	}

	// Processus parent - configuration de la structure CGI
	
	// Fermer les extrémités inutiles des pipes
	close(pipe_in[0]);  // On n'a pas besoin de lire depuis stdin du CGI
	close(pipe_out[1]); // On n'a pas besoin d'écrire dans stdout du CGI

	// Envoyer les données POST au CGI si nécessaire
	if (!req.body.empty()) {
		ssize_t bytes_written = write(pipe_in[1], req.body.c_str(), req.body.size());
		if (bytes_written < 0) {
			std::cerr << "Failed to write to CGI stdin" << std::endl;
		}
	}
	close(pipe_in[1]); // Fermer stdin du CGI après écriture

	// Créer et stocker la structure CGI
	CgiProcess cgiProc;
	cgiProc.pid = pid;
	cgiProc.clientFd = clientFd;
	cgiProc.startTime = time(NULL);
	cgiProc.pipe_out_read = pipe_out[0];
	cgiProc.request = req;
	cgiProc.location = loc;
	cgiProc.server = server;
	cgiProc.isComplete = false;
	cgiProc.headersSent = false;
	
	g_cgiProcesses[pipe_out[0]] = cgiProc;
	
	std::cout << "CGI started: pid=" << pid << ", pipe_fd=" << pipe_out[0] << ", client_fd=" << clientFd << std::endl;
	
	return pipe_out[0]; // Retourner le fd à surveiller
}

// Lit les données disponibles sur le pipe CGI (appelée par poll)
void handleCGIReadEvent(int pipe_fd) {
	std::map<int, CgiProcess>::iterator it = g_cgiProcesses.find(pipe_fd);
	if (it == g_cgiProcesses.end()) {
		std::cerr << "handleCGIReadEvent: CGI process not found for pipe_fd=" << pipe_fd << std::endl;
		return;
	}
	
	CgiProcess& cgiProc = it->second;
	
	// Lire TOUTES les données disponibles en boucle
	while (true) {
		char buffer[CGI_BUFFER_SIZE];
		ssize_t bytesRead = read(pipe_fd, buffer, sizeof(buffer));
		
		if (bytesRead > 0) {
			cgiProc.cgiOutput.append(buffer, bytesRead);
		} else if (bytesRead == 0) {
			// EOF - le CGI a fermé son stdout
			cgiProc.isComplete = true;
			std::cout << "CGI completed: pid=" << cgiProc.pid << ", output_size=" << cgiProc.cgiOutput.size() << std::endl;
			break;
		} else {
			// Erreur de lecture ou EAGAIN
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				// Plus de données disponibles pour le moment
				break;
			} else {
				std::cerr << "Error reading from CGI pipe: " << strerror(errno) << std::endl;
				cgiProc.isComplete = true;
				break;
			}
		}
	}
}

// Vérifie les timeouts des CGI (appelée périodiquement)
void checkCGITimeouts() {
	time_t now = time(NULL);
	std::vector<int> toCleanup;
	
	for (std::map<int, CgiProcess>::iterator it = g_cgiProcesses.begin(); it != g_cgiProcesses.end(); ++it) {
		CgiProcess& cgiProc = it->second;
		
		// Vérifier le timeout configuré
		if (!cgiProc.isComplete && (now - cgiProc.startTime) > CGI_TIMEOUT) {
			std::cout << "CGI timeout: pid=" << cgiProc.pid << std::endl;
			
			// Tuer le processus
			kill(cgiProc.pid, SIGKILL);
			
			// Marquer comme complete avec erreur
			cgiProc.isComplete = true;
			cgiProc.cgiOutput = ""; // Vider la sortie
			
			toCleanup.push_back(it->first);
		}
	}
	
	// Nettoyer les processus CGI zombies
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0) {
		// Processus nettoyé
	}
}

// Nettoie un processus CGI
void cleanupCGIProcess(int pipe_fd) {
	std::map<int, CgiProcess>::iterator it = g_cgiProcesses.find(pipe_fd);
	if (it == g_cgiProcesses.end()) {
		return;
	}
	
	CgiProcess& cgiProc = it->second;
	
	// Fermer le pipe
	if (cgiProc.pipe_out_read >= 0) {
		close(cgiProc.pipe_out_read);
	}
	
	// Attendre le processus s'il n'a pas déjà été attendu
	int status;
	waitpid(cgiProc.pid, &status, WNOHANG);
	
	std::cout << "CGI cleaned up: pid=" << cgiProc.pid << ", pipe_fd=" << pipe_fd << std::endl;
	
	// Supprimer de la map
	g_cgiProcesses.erase(it);
}

// Finalise la réponse CGI
Response finalizeCGIResponse(CgiProcess& cgiProc) {
	Response res;
	res.version = "HTTP/1.1";
	
	// Vérifier si le CGI a timeout
	time_t now = time(NULL);
	if ((now - cgiProc.startTime) > CGI_TIMEOUT && cgiProc.cgiOutput.empty()) {
		res.statusCode = 504;
		res.statusMessage = "Gateway Timeout";
		
		std::ostringstream body_stream;
		body_stream << "<html><body><h1>504 Gateway Timeout</h1>"
		            << "<p>The CGI script took too long to execute (>" << CGI_TIMEOUT << " seconds).</p>"
		            << "</body></html>";
		res.body = body_stream.str();
		
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		res.closingConnection = cgiProc.request.closeConnection;
		return res;
	}
	
	// Si on a reçu des données et que le pipe est fermé (isComplete), c'est probablement un succès
	if (cgiProc.isComplete && !cgiProc.cgiOutput.empty()) {
		// Attendre le processus pour éviter les zombies
		int status;
		waitpid(cgiProc.pid, &status, 0);  // Bloquant mais le processus est déjà terminé
		
		// Parser la réponse même si le processus a échoué, car on a des données
		parseCgiResponse(cgiProc.cgiOutput, res);
	} else if (cgiProc.isComplete && cgiProc.cgiOutput.empty()) {
		// Le CGI est terminé mais n'a rien produit
		int status;
		waitpid(cgiProc.pid, &status, 0);
		
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "CGI produced no output";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	} else {
		// Ne devrait pas arriver ici, mais au cas où
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "CGI Execution Failed";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}
	
	res.closingConnection = cgiProc.request.closeConnection;
	return res;
}
