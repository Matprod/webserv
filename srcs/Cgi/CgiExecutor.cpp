/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 15:59:09 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/18 19:30:00 by Matprod          ###   fr       */
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
	if (last_slash == std::string::npos)
		return uri;
	return uri.substr(last_slash);
}

std::string getPathInfo(const std::string& uri, const std::string& script_name) {
	size_t script_pos = uri.find(script_name);
	if (script_pos == std::string::npos)
		return "";
	return uri.substr(script_pos + script_name.length());
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

LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations) {
	LocationConfig* best_match = NULL;
	size_t longest_match = 0;
	for (size_t i = 0; i < locations.size(); ++i) {
		if (uri.find(locations[i].path) == 0 && locations[i].path.length() > longest_match) {
			best_match = const_cast<LocationConfig*>(&locations[i]);
			longest_match = locations[i].path.length();
		}
	}
	return best_match;
}

Response executeCGI(const Request& req, const LocationConfig& loc, const ServerConfig& server) {
	Response res;
	res.version = "HTTP/1.1";
	std::string extension = getFileExtension(req.uri);
	if (loc.cgi_extensions.find(extension) == loc.cgi_extensions.end()) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "CGI Extension Not Supported";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	std::string cgiProgram = loc.cgi_extensions.at(extension);
	std::string scriptName = getScriptName(req.uri);
	std::string scriptPath = loc.root + scriptName;
	std::string pathInfo = getPathInfo(req.uri, scriptName);

	if (access(scriptPath.c_str(), F_OK) != 0) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "CGI Script Not Found";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	int pipe_in[2], pipe_out[2];
	pid_t pid;

	// Créer des pipes pour l'entrée et la sortie
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "Internal Server Error: Pipe Failed";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	pid = fork();
	if (pid < 0) {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "Internal Server Error: Fork Failed";
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	if (pid == 0) { // Processus fils
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);

		// Configurer l'environnement CGI
		std::vector<std::string> env;
		env.push_back("REQUEST_METHOD=" + req.method);
		env.push_back("QUERY_STRING=" + (req.uri.find('?') != std::string::npos ? req.uri.substr(req.uri.find('?') + 1) : ""));
		env.push_back("CONTENT_LENGTH=" + (req.headers.count("content-length") ? req.headers.at("content-length") : "0"));
		env.push_back("CONTENT_TYPE=" + (req.headers.count("content-type") ? req.headers.at("content-type") : ""));
		env.push_back("SCRIPT_NAME=" + scriptName);
		env.push_back("SCRIPT_FILENAME=" + scriptPath);
		env.push_back("PATH_INFO=" + pathInfo);
		env.push_back("SERVER_NAME=" + (server.server_names.empty() ? "localhost" : server.server_names[0]));
		env.push_back("SERVER_PORT=" + toString<int>(server.port));
		env.push_back("SERVER_PROTOCOL=HTTP/1.1");
		env.push_back("GATEWAY_INTERFACE=CGI/1.1");
		for (std::map<std::string, std::string>::const_iterator h = req.headers.begin(); h != req.headers.end(); ++h) {
			env.push_back("HTTP_" + to_lower(h->first) + "=" + h->second);
		}

		std::vector<char*> envp;
		for (std::vector<std::string>::const_iterator e = env.begin(); e != env.end(); ++e) {
			envp.push_back(const_cast<char*>(e->c_str()));
		}
		envp.push_back(NULL);

		char* argv[] = { const_cast<char*>(cgiProgram.c_str()), const_cast<char*>(scriptPath.c_str()), NULL };
		execve(cgiProgram.c_str(), argv, envp.data());
		std::cerr << "execve failed: " << cgiProgram << " (" << strerror(errno) << ")" << std::endl;
		exit(1);
	}

	close(pipe_in[0]);
	close(pipe_out[1]);

	if (!req.body.empty()) {
		write(pipe_in[1], req.body.c_str(), req.body.size());
	}
	close(pipe_in[1]);

	char buffer[4096];
	std::string cgiOutput;
	ssize_t bytes_read;
	while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0) {
		buffer[bytes_read] = '\0';
		cgiOutput += buffer;
	}
	close(pipe_out[0]);

	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		res.statusCode = 200;
		res.statusMessage = getStatusMessage(200);
		size_t header_end = cgiOutput.find("\r\n\r\n");
		if (header_end != std::string::npos) {
			std::string headers_str = cgiOutput.substr(0, header_end);
			res.body = cgiOutput.substr(header_end + 4);
			std::istringstream header_stream(headers_str);
			std::string line;
			while (std::getline(header_stream, line) && line != "\r") {
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				size_t colon_pos = line.find(':');
				if (colon_pos != std::string::npos) {
					std::string key = line.substr(0, colon_pos);
					std::string value = line.substr(colon_pos + 1);
					value.erase(0, value.find_first_not_of(" \t"));
					res.headers[key] = value;
				}
			}
		} else {
			res.body = cgiOutput;
			res.headers["Content-Type"] = "text/plain";
		}
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	} else {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "CGI Execution Failed";
		res.headers["Content-Type"] = "text/plain";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}
	return res;
}