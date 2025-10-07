/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:45:12 by allan             #+#    #+#             */
/*   Updated: 2025/09/21 17:56:48by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response buildResponse(const Request& request) {
	std::cout << "\n\n\n--------------------------------------------\n" << std::endl;
	std::cout << "NEW REQUEST:\n" << std::endl;
	
	Response res;
	bool use_location = false;
	
	LocationConfig* loc = getMatchingLocation(request, request.config, res, use_location);
	
	if (isCGIRequest(loc, request.uri))
		return executeCGI(request, loc, request.config);	
	
	EffectiveRoute eff;
	eff.getMethod = request.method == "GET" ? true : false;
	eff.closeConnection = request.closeConnection;
	if (use_location && eff.createEffectiveRoute(request.config, loc) == false) {
		res.createResponse(500, "", eff.server->error_pages);
		return res;
	} else if (!use_location && eff.createEffectiveRoute(request.config) == false) {
		res.createResponse(500, "", eff.server->error_pages);
		return res;
	}
		
	int result;
	result = eff.createEffectivePath(request.uri);
	if (use_location && eff.redirect_status >= 300 && eff.redirect_status <= 399 && !eff.redirect_url.empty()) {
        res.statusCode = eff.redirect_status;
        res.statusMessage = getStatusMessage(eff.redirect_status);
        res.headers["Location"] = eff.redirect_url;
        res.body = "<html><head><title>Redirect</title></head><body>Redirecting to <a href='" + eff.redirect_url + "'>" + eff.redirect_url + "</a></body></html>";
        res.headers["Content-Type"] = "text/html";
        res.headers["Content-Length"] = toString<size_t>(res.body.size());
        return res;
    }
	if (result != PATH_OK) {
		res.createResponse(result, "", eff.server->error_pages);
		return res;
	}

	std::cout << eff << std::endl;

	if (request.method == "GET" && isMethodAllowed(GET, eff.allow_methods)) {
		return handleGet(request, eff);
	} else if (request.method == "POST" && isMethodAllowed(POST, eff.allow_methods)) {
		return handlePost(request, eff);
	} else if (request.method == "DELETE" && isMethodAllowed(DELETE, eff.allow_methods)) {
		return handleDelete(request, eff);
	} else
		res.createResponse(405, "", eff.server->error_pages);
		
	return res;
}

//////////////////////////////////////////////////////////
//					GET METHOD							//
//////////////////////////////////////////////////////////

Response handleGet(const Request& request, EffectiveRoute& eff) {
	Response response;
    response.closingConnection = eff.closeConnection;

    if (checkRequestVersion(request.version, response, eff.server->error_pages) == ERROR)
		return response;
	
	if (eff.isDir)
		return handleIndex(eff);	
		
    std::ifstream inFile(eff.uri.c_str(), std::ios::binary);
    if (!inFile) {
        response.createResponse(500, "Failed to open file", eff.server->error_pages);
        return response;
    }
	
    std::ostringstream buffer;
    buffer << inFile.rdbuf();
    std::string content = buffer.str();
	
    inFile.close();
    if (!inFile) {
        response.createResponse(500, "Error closing file", eff.server->error_pages);
        return response;
    }
	
    // Success
    response.version = "HTTP/1.1";
    response.statusCode = 200;
    response.statusMessage = "OK";
    response.body = content;
    response.headers["Content-Length"] = toString<size_t>(content.size());
    response.headers["Content-Type"] = "application/octet-stream";
    response.headers["Connection"] = (eff.closeConnection ? "close" : "keep-alive");

    return response;
}

Response handleIndex(const EffectiveRoute& eff) {
	Response response;
	response.closingConnection = eff.closeConnection;
	
	std::string index_path = eff.uri + eff.index;
	
	int fd = open(index_path.c_str(), O_RDONLY);
	if (fd < 0) { // Checking Errno is allowed here: open is neither a write or read action
    	switch (errno) {
    	    case ENOENT:
				return handleAutoIndex(eff);
			
    	    case EACCES:   	
    	    case EPERM:   		
			case ELOOP:		
			case ENAMETOOLONG:
				response.createResponse(403, "", eff.server->error_pages);
				return response;
				
    	    default:
				response.createResponse(500, "", eff.server->error_pages);
				return response;
		}
	}
	
	struct stat st;
	if (stat(index_path.c_str(), &st) != 0) {
		close(fd);
		response.createResponse(500, "", eff.server->error_pages);
		return response;
	} else if (!S_ISREG(st.st_mode)) {
		close(fd);
		return handleAutoIndex(eff);
	}
	
	return createIndexResponse(fd, eff.closeConnection, eff.server->error_pages);	
}

Response createIndexResponse(int fd, bool closeConnection, std::map<int, std::string> error_pages) {
	Response response;
	response.closingConnection = closeConnection;
    response.version = "HTTP/1.1";
	response.statusCode = 200;
	response.statusMessage = getStatusMessage(200);
	response.setHeader("Content-Type", "text/html");
    response.headers["Connection"] = (closeConnection ? "close" : "keep-alive");
	

	const size_t BUFSZ = 64 * 1024;
    std::vector<char> buf(BUFSZ);
    ssize_t n;
    while ((n = read(fd, &buf[0], BUFSZ)) > 0) //Read Check OK
		response.body.append(&buf[0], static_cast<std::string::size_type>(n));
    close(fd);

    if (n < 0) {
		Response resp;
		resp.closingConnection = closeConnection;
		resp.createResponse(500, "", error_pages);
		return resp;
    }
	
	response.setHeader("Content-Length", toString<size_t>(response.body.size()));
	return response;
}

Response handleAutoIndex(const EffectiveRoute& eff) {
	
	if (!eff.autoindex) {
		std::cout << "Hello:" << eff.autoindex << std::endl;
		Response response;
		response.closingConnection = eff.closeConnection;
		response.createResponse(403, "AutoIndex Not allowed by Default (add rule in config file)", eff.server->error_pages);
		return response;
	}
	
	return createAutoIndexResponse(eff);
}

Response createAutoIndexResponse(const EffectiveRoute& eff) {
	Response response;
    response.version = "HTTP/1.1";
	response.closingConnection = eff.closeConnection;
	
	response.body = "<!DOCTYPE html>\n";
	response.body += "<html>\n";
	response.body += "<head><title>Index of ";
	response.body += eff.uri;
	response.body += "</title></head>\n";
	response.body += "<body>\n";
	response.body += "<h1>Index of ";
	response.body += eff.uri;
	response.body += "</h1>\n";
	response.body += "<ul>\n";
	
	DIR* dir = opendir(eff.uri.c_str());
	if (!dir) {
		Response resp;
		resp.closingConnection = eff.closeConnection;
		resp.createResponse(500, "", eff.server->error_pages);
		return response;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
    	std::string name = entry->d_name;
    	if (name == ".") continue;

		struct stat st;
		std::string currPath = eff.uri + name;
		if (stat(currPath.c_str(), &st) == 0) {
			std::string dirPath = name + "/";
    		if (S_ISDIR(st.st_mode)) { 
				response.body += "<li><a href=";
				response.body += dirPath;
				response.body += ">";
				response.body += dirPath;
				response.body += "</a></li>\n";
			}
    		else if (S_ISREG(st.st_mode)) { 
				response.body += "<li><a href=";
				response.body += name;
				response.body += ">";
				response.body += name;
				response.body += "</a></li>\n";
			}
		}
	}
	
	response.body += "</ul>\n";
	response.body += "</body>\n";
	response.body += "</html>\n";
	
	response.statusCode = 200;
	response.statusMessage = getStatusMessage(200);
	response.setHeader("Content-Type", "text/html");
    response.headers["Content-Length"] = toString<size_t>(response.body.size());
    response.headers["Connection"] = (eff.closeConnection ? "close" : "keep-alive");
	
	closedir(dir);
	return response;
}

//////////////////////////////////////////////////////////
//					POST METHOD							//
//////////////////////////////////////////////////////////


Response handlePost(const Request& request, const EffectiveRoute& eff) {
	//std::cout << "Post Response" << std::endl;
	File file;
	
	file.response.closingConnection = shouldConnectionBeClosed(request.headers);
	if (checkRequestVersion(request.version, file.response, eff.server->error_pages) == ERROR)
		return file.response;
	
	if (file.getFileName(eff) == ERROR)
		return file.response;
	std::cout << "FileName:\t" << file.fileName << std::endl;

	if (file.getFileData(request, eff) == ERROR)
		return file.response;

	if (file.createFile(request.body, eff) == ERROR)
		return file.response;
	
	file.response.createResponse(200, "File Created", eff.server->error_pages);
	return file.response;
}
		

//////////////////////////////////////////////////////////
//					DELETE METHOD						//
//////////////////////////////////////////////////////////

Response handleDelete(const Request& request, const EffectiveRoute& eff) {
	//std::cout << "Delete Response" << std::endl;
	File file;
	
	file.response.closingConnection = shouldConnectionBeClosed(request.headers);
	
	if (checkRequestVersion(request.version, file.response, eff.server->error_pages) == ERROR)
		return file.response;
		
	if (file.getFileName(eff) == ERROR)
		return file.response;
	
	if (eff.use_alias)
		file.filePath = eff.alias + "/upload/" + file.fileName;
	else
		file.filePath = eff.root + "/upload/" + file.fileName;
	
	if (unlink(file.filePath.c_str()) != 0) { //Unlink is not read or write, errno is allowed
		file.createDeleteResponse(errno, eff.server->error_pages);
		return file.response;
	}
	file.response.createResponse(204, "", eff.server->error_pages);
	return file.response;
}

void File::createDeleteResponse(const int err, std::map<int, std::string> error_pages) {
    switch (err) {
        case ENOENT:  // No such file or directory
            response.createResponse(404, "Ressource was not Found", error_pages); // Not Found
            break;

        case EACCES:  // Permission denied
            response.createResponse(403, "Operation is not Allowed", error_pages); // Not Found
            break;
			
        case ENOTDIR: // Component in path is not a directory
            response.createResponse(404, "Path Invalid", error_pages); // Not Found
            break;
			
        case EROFS:   // Read-only filesystem
            response.createResponse(403, "Server cannot modify resource", error_pages); // Not Found
            break;

        case EPERM:   // Operation not permitted (e.g., deleting directory)
            response.createResponse(403, "Operation Not Permitted", error_pages); // Not Found
            break;
		
        case EISDIR:  // Is a directory
            response.createResponse(403, "Delete Not Allowed on Directory", error_pages); // Not Found
            break;

        case ENAMETOOLONG: // Path too long
            response.createResponse(414, "", error_pages); // Not Found
            break;

        case EBUSY: // Resource busy (locked)
            response.createResponse(423, "Ressource is currently locked or in use", error_pages); // Not Found
            break;

        default:
            response.createResponse(500, "Unexpected failure", error_pages); // Not Found
            break;
	}
}

//////////////////////////////////////////////////////////
//					FILE MANAGMENT						//
//////////////////////////////////////////////////////////

int File::getFileName(const EffectiveRoute& eff) {
	
    std::string directory;
	size_t pos = eff.uri.find_last_of('/');

    std::string name;
    if (pos == std::string::npos) {
        name = eff.uri;
    } else {
        name = eff.uri.substr(pos + 1);
    }

    if (name.empty()) {
        response.createResponse(400, "A filename must be provided in the URI", eff.server->error_pages);
        return ERROR;
    }

    fileName = name;
    return SUCCESS;
}

int File::createFile(const std::string& body, const EffectiveRoute& eff) {
	if (body.size() != length) {
		response.createResponse(400 , "Mismatch between request: Body size/Content-Length", eff.server->error_pages);
		return ERROR;
	}
	filePath = generateUniqueFilename(eff);
	std::cout << "FilePath:\t" << filePath << std::endl;
	
	std::ofstream outFile(filePath.c_str(), std::ios::binary);
	if (!outFile) {
		response.createResponse(500, "Outfile Could not be Opened", eff.server->error_pages);
		return ERROR;
	}
	
	outFile.write(body.data(), body.size()); //std::ofstream::write() != write();
	if (!outFile) {
		response.createResponse(500, "Error during: Write to File", eff.server->error_pages);
		return ERROR;
	}
	
	outFile.close();
	if (!outFile) {
		response.createResponse(500, "Error while attempting to close the file ", eff.server->error_pages);
		return ERROR;
	}
	return SUCCESS;
}

bool File::fileExists(const std::string fullPath) const {
	struct stat buffer;
	return (stat(fullPath.c_str(), &buffer) == 0);
}

std::string File::generateUniqueFilename(const EffectiveRoute& eff) {
    std::string baseDir;
	if (eff.use_alias)
		baseDir = eff.alias + eff.upload_path + "/";
	else
		baseDir = eff.root + eff.upload_path + "/";
		
    std::string name = fileName;
    std::string extension = "";

    // Extract extension if present
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        name = fileName.substr(0, dotPos);
        extension = fileName.substr(dotPos);
    }

    std::string fullPath = baseDir + fileName;
    int counter = 1;

    // Check if file exists and append counter if needed
    while (fileExists(fullPath)) {
        std::ostringstream newName;
        newName << baseDir << name << "_" << counter << extension;
        fullPath = newName.str();
        counter++;
    }

    return fullPath;
}

int File::getFileData(const Request& request, const EffectiveRoute& eff) {
	//STEP 1: Check Header Content Type (Empty or application/octet-stream)
	const std::map<std::string, std::string> headers = request.headers;
	std::map<std::string,std::string>::const_iterator it = headers.find("content-type"); 
	if (it != headers.end()) {
		std::string contentType = it->second;
		if (checkContentType(contentType) == ERROR) {
			response.createResponse(415, "Only Content-Type allowed:\n'application/octet-stream'", eff.server->error_pages);
			return ERROR;
		}
	}
	
	//STEP 2: Check that Header Content-Length is present and Valid
	std::map<std::string,std::string>::const_iterator itLength = headers.find("content-length"); 
	if (itLength == headers.end()) {
		response.createResponse(411, "", eff.server->error_pages);
		return ERROR; //Need to have a Content-Lenght 
	}
	else if (isValidContentLength(itLength->second) == ERROR) {
		response.createResponse(400 , "Invalid Content-Length", eff.server->error_pages);
		return ERROR; //Invalid Content-Lenght
	}
	return SUCCESS;
}

int File::checkContentType(const std::string &contentType) const {
	if (contentType.size() == 24 && contentType.compare(0, 26, "application/octet-stream") == 0)
		return SUCCESS;
	else if (contentType.size() == 33 && contentType.compare(0, 35, "application/x-www-form-urlencoded") == 0)
		return SUCCESS;
	else if (contentType.size() == 16 && contentType.compare(0, 18, "application/json") == 0)
		return SUCCESS;
	else if (contentType.size() == 10 && contentType.compare(0, 12, "text/plain") == 0)
		return SUCCESS;
	return ERROR;
}

bool File::isValidContentLength(const std::string& contentLength) {
	if (contentLength[0] && !isdigit(contentLength[0]))
		return ERROR; //Check for '-'
		
	char *end;

	length = std::strtol(contentLength.c_str(), &end, 10);
	if (*end != '\0')
		return ERROR;
	return SUCCESS;	
}

//////////////////////////////////////////////////////////
//					UTILS								//
//////////////////////////////////////////////////////////

bool isMethodAllowed(int method, std::set<std::string> allow_methods) {
	switch (method) {
		case 1: if (allow_methods.find("GET") == allow_methods.end()) return false;
				else return true;
		case 2: if (allow_methods.find("POST") == allow_methods.end()) return false;
				else return true;
		case 3: if (allow_methods.find("DELETE") == allow_methods.end()) return false;
				else return true;
	}
	return false;
}


bool shouldConnectionBeClosed(const std::map<std::string, std::string>& headers) {
	std::map<std::string,std::string>::const_iterator it = headers.find("connection"); 
	if (it != headers.end()) {
		std::string contentType = it->second;
		if (contentType.size() == 5 && contentType.compare(0, 7, "close") == 0)
			return true;
	}
	return false;
}

int checkRequestVersion(const std::string& version, Response& response, std::map<int, std::string> error_pages) {
	const std::string allowedVersion = "HTTP/1.1";
	if (version.compare(0, version.size(), allowedVersion) != 0) {
		response.createResponse(505, "", error_pages);
		return ERROR;
	}
	return SUCCESS;
}
		
std::string Response::responseToString() const {
	std::ostringstream oss;

	oss << version << " " << statusCode << " " << statusMessage << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n" << body;
	return oss.str();
}

void Response::createResponse(unsigned int code, const std::string& bodyText, std::map<int, std::string> error_pages) {
    version = "HTTP/1.1";
    statusCode = code;
    statusMessage = getStatusMessage(code);

    // 1 - Decide whether this status is allowed to have a body
    const bool mayHaveBody = !( (code >= 100 && code < 200) || code == 204 || code == 304 );

    // 2 - Set/clear body + content-type
    if (mayHaveBody && !bodyText.empty()) {
        body = bodyText;
        if (headers.find("Content-Type") == headers.end())
            headers["Content-Type"] = "text/plain";
    } else {
        body.clear();
        headers.erase("Content-Type");
    }

    // 3 - Connection handling
	setClosingConnection();
	if (isErrorStatusCode(statusCode)) {
		if (setCustomErrorPage(error_pages) == false)
			setDefaultErrorPage();
	}
    headers["Content-Length"] = toString<size_t>(body.size());
}

void Response::setClosingConnection(void) {
    if (closingConnection) {
        headers["Connection"] = "close";
    } else {
        switch (statusCode) {
            case 400: // Bad Request (parsing issues)
            case 408: // Request Timeout (if you use it)
            case 411: // Length Required
            case 413: // Payload Too Large
            case 414: // URI Too Long
            case 431: // Request Header Fields Too Large (if you use it)
            case 505: // HTTP Version Not Supported
                headers["Connection"] = "close";
                closingConnection = true;
                break;
            default:
                // keep-alive by default in HTTPS/1.1
                headers["Connection"] = "keep alive";
                break;
        }
    }
}
	

std::string getStatusMessage(int statusCode) {
	switch(statusCode) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method not Allowed";
		case 411: return "Length Required";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsuported Media Type";
		case 423: return "Locked";
		case 500: return "Internal Server Error";
		case 505: return "Version not supported";
	}
	return "Unknown";
}

void Response::setHeader(std::string header, std::string content) {
	if (headers.find(header) == headers.end()) {
		if (header == "Content-Type")
        	headers["Content-Type"] = content;
		else if (header == "Content-Length")
        	headers["Content-Length"] = content;
	}
	return ;
}

bool isErrorStatusCode(int statusCode) {
	switch (statusCode)
	{
		case 400:
		case 403:
		case 404:
		case 405:
		case 411:
		case 413:
		case 414:
		case 415:
		case 423:
		case 500:
		case 505:
			return true;
	
		default:
			return false;
	}
	return true;
}

bool Response::setCustomErrorPage(std::map<int, std::string> error_pages) {
	std::map<int, std::string>::iterator it = error_pages.find(statusCode);
	if (it == error_pages.end())
		return false;
		
	//std::cout << "ERROR PAGE URI: \t" << it->second << std::endl;
	
	int fd = open(it->second.c_str(), O_RDONLY);
	if (fd < 0) { // Check Errno allowed here: open is neither a write or read action
		return false;
	}

	struct stat st;
	if (stat(it->second.c_str(), &st) != 0) {
		close(fd);
		return false;
	} else if (!S_ISREG(st.st_mode)) {
		close(fd);
		return false;
	}

	body.clear();
	if (!writeCustomErrorToBody(fd)) {
		body.clear();
		return false;
	}
	
	setHeader("Content-Type", "text/html");	
	return true;
}

bool Response::writeCustomErrorToBody(int fd) {
	const size_t BUFSZ = 64 * 1024;
    std::vector<char> buf(BUFSZ);
    ssize_t n;
	
    while ((n = read(fd, &buf[0], BUFSZ)) > 0) //Check Read OK
		body.append(&buf[0], static_cast<std::string::size_type>(n));
    close(fd);

    if (n < 0) {
		body.clear();
		return false;
	}
	
	setHeader("Content-Length", toString<size_t>(body.size()));
	std::cout << body.size() << std::endl;
	return true;
}

void Response::setDefaultErrorPage() {
	headers["Content-Type"] = "text/html";
	body = 	"<!DOCTYPE html>";
	body +=	"<html>";
	body +=	"<head><title>";
	body += toString<int>(statusCode);
	body += statusMessage;	
	body += "</title></head>";
	body +=	"<body>";
	body +=	"<h1>";
	body += toString<int>(statusCode);
	body += statusMessage;	
	body +=	"</h1>";
	body +=	"</body>";
	body +=	"</html>";
	
    headers["Content-Length"] = toString<size_t>(body.size());
}

Response::Response()
	: version("HTTP/1.1"), statusCode(0), statusMessage(""),  body(""), closingConnection(false)
{
	headers.clear();
}

/* 
	HTTP status codes are grouped into 5 classes:
		1xx → Informational
		2xx → Success (200 OK, 201 Created)
		3xx → Redirection (301, 302)
		4xx → Client errors (400 Bad Request, 404 Not Found)
		5xx → Server errors (500 Internal Server Error) 
*/

/* 
	GET: Server sends back the requested resource (HTML page, image, etc.).
	POST: Usually processes data and returns a result or confirmation.
	DELETE: Returns confirmation (204 No Content or 200 OK).
	
	Each method expects certain status codes:
		GET → 200 OK or 404 Not Found
		POST → 201 Created, 200 OK, or 400 Bad Request
		DELETE → 200 OK or 204 No Content
 */