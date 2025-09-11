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

Response executeCGI(const Request& req, const LocationConfig& loc, const ServerConfig& server) {
	Response res;
	res.version = "HTTP/1.1";

	
	std::string extension = getFileExtension(req.uri);
	if (loc.cgi_extensions.find(extension) == loc.cgi_extensions.end()) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "CGI Extension Not Supported";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}

	std::string cgiProgram = loc.cgi_extensions.at(extension);
	std::string scriptName = getScriptName(req.uri);
	std::string scriptPath = loc.root + "/" + scriptName; // Ensure proper path separator
	std::string pathInfo = getPathInfo(req.uri, loc.path, scriptName);
	std::string queryString = getQueryString(req.uri);
	std::cout << "Debug: uri=" << req.uri << ", loc_path=" << loc.path << ", scriptName=" << scriptName << ", pathInfo=" << pathInfo << std::endl;
	if (access(scriptPath.c_str(), F_OK) != 0) {
		res.statusCode = 404;
		res.statusMessage = getStatusMessage(404);
		res.body = "CGI Script Not Found";
		res.headers["Content-Type"] = "text/html";
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
		res.headers["Content-Type"] = "text/html";
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
		res.headers["Content-Type"] = "text/html";
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
		env.push_back("QUERY_STRING=" + queryString);
		env.push_back("CONTENT_LENGTH=" + toString<size_t>(req.body.size()));
		env.push_back("CONTENT_TYPE=" + (req.headers.count("content-type") ? req.headers.at("content-type") : ""));
		env.push_back("SCRIPT_NAME=" + scriptName);
		env.push_back("SCRIPT_FILENAME=" + scriptPath);
		env.push_back("PATH_INFO=" + pathInfo);
		env.push_back("SERVER_NAME=" + (server.server_names.empty() ? "localhost" : server.server_names[0]));
		env.push_back("SERVER_PORT=" + toString<int>(server.port));
		env.push_back("SERVER_PROTOCOL=HTTP/1.1");
		env.push_back("GATEWAY_INTERFACE=CGI/1.1");
		env.push_back("REDIRECT_STATUS=200");
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

	std::string cgiOutput = readCgiOutput(pipe_out[0]);
	close(pipe_out[0]);

	std::cout << "Raw CGI Output for " << scriptPath << ": " << cgiOutput << std::endl;
	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		res.statusCode = 200;
		res.statusMessage = getStatusMessage(200);
		size_t header_end = cgiOutput.find("\r\n\r\n");
		if (header_end == std::string::npos)
			header_end = cgiOutput.find("\n\n");
		if (header_end != std::string::npos) {
			std::string headers_str = cgiOutput.substr(0, header_end);
			res.body = cgiOutput.substr(header_end + (cgiOutput[header_end + 2] == '\r' ? 4 : 2)); // Ajuste selon \r\n ou \n
			std::istringstream header_stream(headers_str);
			std::string line;
			while (std::getline(header_stream, line) && !line.empty()) {
				if (line[line.size() - 1] == '\r') line.erase(line.size() - 1);
				size_t colon_pos = line.find(':');
				if (colon_pos != std::string::npos) {
					std::string key = line.substr(0, colon_pos);
					std::string value = line.substr(colon_pos + 1);
					value.erase(0, value.find_first_not_of(" \t"));
					res.headers[key] = value;
				}
			}
			if (res.headers.find("Content-Length") == res.headers.end())
				res.headers["Content-Length"] = toString<size_t>(res.body.size());
		} else {
			// Pas d’en-têtes → corps brut
			res.body = cgiOutput;
			res.headers["Content-Type"] = "text/html";
			res.headers["Content-Length"] = toString<size_t>(res.body.size());

		}
	} else {
		res.statusCode = 500;
		res.statusMessage = getStatusMessage(500);
		res.body = "CGI Execution Failed ";
		res.headers["Content-Type"] = "text/html";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
	}
	return res;
}