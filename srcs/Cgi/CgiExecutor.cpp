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

std::string readCgiOutput(int fd) {
	std::string output;
	char buffer[4096];
	ssize_t bytesRead;

	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
		output.append(buffer, bytesRead);
	}

	if (bytesRead < 0) {
		perror("read from CGI failed");
		return "";
	}

	return output;
}

void parseCgiResponse(const std::string& cgiOutput, Response& res) {
    std::cout << "=== CGI OUTPUT DEBUG ===" << std::endl;
    std::cout << "Raw output length: " << cgiOutput.length() << std::endl;
    std::cout << "First 200 chars: " << cgiOutput.substr(0, 200) << std::endl;
    
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

    std::cout << "Header end position: " << header_end << std::endl;
    
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

		// Chercher le séparateur ':'
		size_t colon_pos = line.find(':');
		if (colon_pos == std::string::npos) continue;

		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);
		
		// Supprimer les espaces en début de valeur
		value.erase(0, value.find_first_not_of(" \t"));

		// Traiter le header Status spécial
		if (to_lower(key) == "status") {
			// Format: "Status: 200 OK" ou "Status: 200"
			std::istringstream status_stream(value);
			std::string status_code_str;
			status_stream >> status_code_str;
			
			int status_code = atoi(status_code_str.c_str());
			if (status_code >= 100 && status_code < 600) {
				res.statusCode = status_code;
				// Extraire le message de statut si présent
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
			// Headers normaux
			res.headers[key] = value;
		}
	}

	// Si aucun status n'a été défini, utiliser 200 par défaut
	if (!status_set) {
		res.statusCode = 200;
		res.statusMessage = getStatusMessage(200);
	}

	// Ajouter Content-Length si pas défini
	if (res.headers.find("Content-Length") == res.headers.end()) {
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}
}

Response executeCGI(const Request& req, const LocationConfig& loc, const ServerConfig& server) {
	Response res;
	res.version = "HTTP/1.1";

	// Vérifier l'extension CGI
	std::string extension = getFileExtension(req.uri);
	if (loc.cgi_extensions.find(extension) == loc.cgi_extensions.end()) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "CGI Extension Not Supported";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	// Obtenir le programme CGI et construire le chemin du script
	std::string cgiProgram = loc.cgi_extensions.at(extension);
	std::string scriptName = getScriptName(req.uri);
	
	// Utiliser le root de la location s'il existe, sinon utiliser le root du serveur
	std::string rootPath;
	if (loc.has_root) {
		rootPath = loc.root;
	} else if (server.has_root) {
		rootPath = server.root;
	} else {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "No root directory configured";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}
	
	std::string scriptPath = rootPath + "/" + scriptName;
	std::string pathInfo = getPathInfo(req.uri, loc.path, scriptName);
	std::string queryString = getQueryString(req.uri);
	
	std::cout << "CGI DEBUG:" << std::endl;
	std::cout << "  Extension: " << extension << std::endl;
	std::cout << "  CGI Program: " << cgiProgram << std::endl;
	std::cout << "  Script Name: " << scriptName << std::endl;
	std::cout << "  Location Root: " << loc.root << std::endl;
	std::cout << "  Server Root: " << server.root << std::endl;
	std::cout << "  Used Root: " << rootPath << std::endl;
	std::cout << "  Script Path: " << scriptPath << std::endl;

	// Vérifier que le script existe et est exécutable
	if (access(scriptPath.c_str(), F_OK) != 0) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "CGI Script Not Found: " + scriptPath;
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	if (access(scriptPath.c_str(), X_OK) != 0) {
		res.statusCode = 403;
		res.statusMessage = getStatusMessage(403);
		res.body = "CGI Script Not Executable";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	// Créer les pipes pour la communication
	int pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "Internal Server Error: Pipe Creation Failed";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	// Fork pour créer le processus CGI
	pid_t pid = fork();
	if (pid < 0) {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "Internal Server Error: Fork Failed";
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
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
		
		// Variables CGI obligatoires
		env_vars.push_back("REQUEST_METHOD=" + req.method);
		env_vars.push_back("QUERY_STRING=" + queryString);
		env_vars.push_back("CONTENT_LENGTH=" + toString<size_t>(req.body.size()));
		env_vars.push_back("CONTENT_TYPE=" + (req.headers.count("content-type") ? req.headers.at("content-type") : ""));
		env_vars.push_back("SCRIPT_NAME=" + scriptName);
		env_vars.push_back("SCRIPT_FILENAME=" + scriptPath);
		env_vars.push_back("PATH_INFO=" + pathInfo);
		env_vars.push_back("PATH_TRANSLATED=" + loc.root + pathInfo);
		env_vars.push_back("SERVER_NAME=" + (server.server_names.empty() ? "localhost" : server.server_names[0]));
		env_vars.push_back("SERVER_PORT=" + toString<int>(server.port));
		env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
		env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
		env_vars.push_back("REDIRECT_STATUS=200");
		env_vars.push_back("SERVER_SOFTWARE=webserv/1.0");
		
		// Ajouter les headers HTTP comme variables d'environnement
		for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
			std::string env_name = "HTTP_" + to_lower(it->first);
			// Remplacer les tirets par des underscores
			for (size_t i = 0; i < env_name.length(); ++i) {
				if (env_name[i] == '-') env_name[i] = '_';
			}
			env_vars.push_back(env_name + "=" + it->second);
		}

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

	// Processus parent - gérer la communication avec le CGI
	
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
	close(pipe_in[1]);

	// Lire la sortie du CGI
	std::string cgiOutput = readCgiOutput(pipe_out[0]);
	close(pipe_out[0]);

	// Attendre que le processus CGI se termine
	int status;
	waitpid(pid, &status, 0);

	// Vérifier si le CGI s'est exécuté correctement
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		// Parser la réponse CGI
		parseCgiResponse(cgiOutput, res);
	} else {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "CGI Execution Failed";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}

	return res;
}