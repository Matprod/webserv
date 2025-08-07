/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:45:12 by allan             #+#    #+#             */
/*   Updated: 2025/07/26 17:14:33 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response buildResponse(const Request& request, const std::vector<ServerConfig>& servers) {
	Response res;
	res.version = "HTTP/1.1";

	ServerConfig* server = getMatchingServer(request, servers, res);
	if (!server)
		return res;
	LocationConfig* loc = getMatchingLocation(request, *server, res);
	if (!loc)
		return res;
	if (handleRedirect(*loc, res))
		return res;
	if (isCGIRequest(*loc, request.uri))
		return executeCGI(request, *loc, *server);
	
	if (request.method == "GET") {
		return handleGet(request);
	} else if (request.method == "POST") {
		return handlePost(request);
	} else if (request.method == "DELETE") {
		return handleDelete(request);
	} else {
		Response res;
		res.version = "HTTP/1.1";
		res.statusCode = 405;
		res.statusMessage = getStatusMessage(405);
		res.headers["Content-Type"] = "text/plain";
		res.body = "Method Not Allowed";
		res.headers["Content-Length"] = toString<size_t>(res.body.size());
		return res;
	}
}

Response handleGet(const Request& request) {
	Response response;
	
	response.closingConnection = shouldConnectionBeClosed(request.headers);

	if (checkRequestVersion(request.version, response) == ERROR)
		return response;
	response.createResponse(200, "");	
	return response;
}


//////////////////////////////////////////////////////////
//					POST METHOD							//
//////////////////////////////////////////////////////////


Response handlePost(const Request& request) {
	std::cout << "Post Response" << std::endl;
	File file;
	

	file.response.closingConnection = shouldConnectionBeClosed(request.headers);
	std::cout << "1" << std::endl;
	if (checkRequestVersion(request.version, file.response) == ERROR)
		return file.response;
	
	std::cout << "2" << std::endl;
	if (file.getFileName(request.uri) == ERROR)
		return file.response;

	std::cout << "3" << std::endl;
	if (file.getFileData(request) == ERROR)
		return file.response;

	std::cout << "4" << std::endl;
	if (file.createFile(request.body) == ERROR)
		return file.response;
		
	std::cout << "5" << std::endl;
	file.response.createResponse(200, "File Created");
	return file.response;
}
		
int File::getFileName(const std::string& uri) {
    const std::string directory = "/upload/";

    // Must start with "/upload/"
    if (uri.compare(0, directory.size(), directory) != 0) {
		response.createResponse(404, "File must be posted to: /upload/*filename*");
        return ERROR;
	}

    // Extract filename after "/upload/"
    fileName = uri.substr(directory.size());

    // Filename must not be empty
    if (fileName.empty()) {
		response.createResponse(400, "A Filename should be mentioned in the uri: '/upload/*filename*'");
        return ERROR;
	}

    // Filename must not contain '/'
    if (fileName.find('/') != std::string::npos) {
		response.createResponse(400, "Filename should not contain '/' char");
        return ERROR;
	}

    return SUCCESS;
}


int File::getFileData(const Request& request) {
	//STEP 1: Check Header Content Type (Empty or application/octet-stream)
	const std::map<std::string, std::string> headers = request.headers;
	std::map<std::string,std::string>::const_iterator it = headers.find("content-type"); 
	if (it != headers.end()) {
		std::string contentType = it->second;
		if (contentType.size() != 24 || contentType.compare(0, 26, "application/octet-stream") != 0) {
			response.createResponse(415, "Only Content-Type allowed: 'application/octet-stream'");
			return ERROR;
		}
	}
	
	//STEP 2: Check that Header Content-Length is present and Valid
	std::map<std::string,std::string>::const_iterator itLength = headers.find("content-length"); 
	if (itLength == headers.end()) {
		response.createResponse(411, "");
		return ERROR; //Need to have a Content-Lenght 
	}
	else if (isValidContentLength(itLength->second) == ERROR) {
		response.createResponse(400 , "Invalid Content-Length");
		return ERROR; //Invalid Content-Lenght
	}
	return SUCCESS;
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

int File::createFile(const std::string& body) {
	if (body.size() != length) {
		response.createResponse(400 , "Mismatch between request: Body size/Content-Length");
		return ERROR;
	}
	filePath = generateUniqueFilename();
	std::ofstream outFile(filePath.c_str(), std::ios::binary);
	if (!outFile) {
		response.createResponse(500, "Outfile Could not be Opened");
		return ERROR;
	}
	
	outFile.write(body.data(), body.size());
	if (!outFile) {
		response.createResponse(500, "Error during: Write to File");
		return ERROR;
	}
	
	outFile.close();
	if (!outFile) {
		response.createResponse(500, "Error while attempting to close the file ");
		return ERROR;
	}
	return SUCCESS;
}

bool File::fileExists(const std::string fullPath) const {
	struct stat buffer;
	return (stat(fullPath.c_str(), &buffer) == 0);
}

std::string File::generateUniqueFilename() {
    std::string baseDir = "upload/";
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

//////////////////////////////////////////////////////////
//					DELETE METHOD						//
//////////////////////////////////////////////////////////

Response handleDelete(const Request& request) {
	std::cout << "Post Response" << std::endl;
	File file;
	
	file.response.closingConnection = shouldConnectionBeClosed(request.headers);
	
	if (checkRequestVersion(request.version, file.response) == ERROR)
		return file.response;
		
	if (file.getFileName(request.uri) == ERROR)
		return file.response;
	file.filePath = request.uri;
	
	if (unlink(file.filePath.c_str()) != 0) {
		file.createDeleteResponse();
		return file.response;
	}
	file.response.createResponse(200, "File has been Deleted");
	return file.response;
}

void File::createDeleteResponse() {
    switch (errno) {
        case ENOENT:  // No such file or directory
            response.createResponse(404, "Ressource was not Found"); // Not Found
            break;

        case EACCES:  // Permission denied
            response.createResponse(404, "Operation is not Allowed"); // Not Found
            break;
			
        case ENOTDIR: // Component in path is not a directory
            response.createResponse(404, "Path Invalid"); // Not Found
            break;
			
        case EROFS:   // Read-only filesystem
            response.createResponse(403, "Server cannot modify resource"); // Not Found
            break;

        case EPERM:   // Operation not permitted (e.g., deleting directory)
            response.createResponse(403, ""); // Not Found
            break;
		
        case EISDIR:  // Is a directory
            response.createResponse(403, "Delete Not Allowed on Directory"); // Not Found
            break;

        case ENAMETOOLONG: // Path too long
            response.createResponse(414, ""); // Not Found
            break;

        case EBUSY: // Resource busy (locked)
            response.createResponse(423, "Ressource is currently locked or in use"); // Not Found
            break;

        default:
            response.createResponse(500, "Unexpected failure"); // Not Found
            break;
	}
}


//////////////////////////////////////////////////////////
//					UTILS								//
//////////////////////////////////////////////////////////

bool shouldConnectionBeClosed(const std::map<std::string, std::string>& headers) {
	std::map<std::string,std::string>::const_iterator it = headers.find("connection"); 
	if (it != headers.end()) {
		std::string contentType = it->second;
		if (contentType.size() != 24 || contentType.compare(0, 5, "close") != 0)
			return true;
	}
	return false;
}

int checkRequestVersion(const std::string& version, Response& response) {
	const std::string allowedVersion = "HTTP/1.1";
	if (version.compare(0, version.size(), allowedVersion) != 0) {
		response.createResponse(505, "");
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


void Response::createResponse(unsigned int code, const std::string& reason) {
	version = "HTTP/1.1";	
	statusCode = code;
	statusMessage = getStatusMessage(code);
	headers["Content-Length"] = toString<size_t>(reason.size());
	if (reason.size() > 0) {
		body = reason;
		headers["Content-Type"] = "text/plain";
	}
	if (closingConnection == 1)
		headers["Connection"] = "close";
	else {
		switch(statusCode) {
			case 400:
			case 411:
			case 413:
			case 414:
			case 415:
			case 505:
				headers["Connection"] = "close";
				closingConnection == true;
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