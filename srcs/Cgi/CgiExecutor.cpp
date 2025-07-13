/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExecutor.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 19:54:49 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/13 20:45:24 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiExecutor.hpp"

bool Server::isCgiRequest(const std::string& uri, LocationConfig* loc) {
    if (!loc->cgi_path.empty()) {
        size_t pos = uri.rfind('.');
        if (pos != std::string::npos) {
            std::string ext = uri.substr(pos);
            return loc->cgi_extension == ext;
        }
    }
    return false;
}

void Server::handleCgiRequest(int client_fd, const Request& req, LocationConfig& loc, ServerConfig& conf) {
    int pipe_in[2], pipe_out[2];
    if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) {
        sendErrorResponse(client_fd, 500);
        close(client_fd);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        sendErrorResponse(client_fd, 500);
        close(pipe_in[0]); close(pipe_in[1]);
        close(pipe_out[0]); close(pipe_out[1]);
        close(client_fd);
        return;
    }

    if (pid == 0) {
        close(pipe_in[1]);
        close(pipe_out[0]);
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);
        close(pipe_out[1]);

        std::string script_path = resolvePath(req.uri, loc.root);
        char* argv[] = {const_cast<char*>(loc.cgi_path.c_str()), const_cast<char*>(script_path.c_str()), NULL};
        char* env[] = {
            const_cast<char*>("REQUEST_METHOD=" + req.method.c_str()),
            const_cast<char*>("PATH_INFO=" + script_path.c_str()),
            const_cast<char*>("SERVER_PROTOCOL=" + req.version.c_str()),
            NULL
        };
        execve(loc.cgi_path.c_str(), argv, env);
        exit(1); // Exec failed
    }

    // Parent process
    close(pipe_in[0]);
    close(pipe_out[1]);

    // Write request body to CGI
    if (!req.body.empty()) {
        write(pipe_in[1], req.body.c_str(), req.body.length());
    }
    close(pipe_in[1]);

    // Read CGI output
    std::string cgi_output;
    char buffer[1024];
    int bytes_read;
    while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        cgi_output.append(buffer, bytes_read);
    }
    close(pipe_out[0]);

    waitpid(pid, NULL, 0);

    // Send CGI response
    std::string stream;
    stream << "HTTP/1.1 200 OK\r\n";
    stream << "Content-Type: text/html\r\n"; // Adjust based on CGI output
    stream << "Content-Length: " << cgi_output.length() << "\r\n\r\n";
    stream << cgi_output;
    send(client_fd, stream.c_str(), stream.length(), 0);
    close(client_fd);
}