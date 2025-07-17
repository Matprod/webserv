/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:45:12 by allan             #+#    #+#             */
/*   Updated: 2025/07/16 20:09:06 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"


Response buildResponse(const Request& request) {
	if (request.method == "GET") {
		return handleGet();
	} else if (request.method == "POST") {
		return handlePost(request);
	} else if (request.method == "DELETE") {
		return handleDelete();
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

Response handleGet() {
	Response res;

	res.version = "HTTP/1.1";
	res.body = "<html><body><h1>Hello, GET!<h1><body><html>";
	res.statusCode = 200;
	res.statusMessage = getStatusMessage(200);
	res.headers["Content-Type"] = "text/html";
	res.headers["Content-Length"] = toString<size_t>(res.body.size());
	return res;
}

Response handlePost(const Request& request) {
	Response res;

	res.version = "HTTP/1.1";
	res.body = "You posted: " + request.body;
	res.statusCode = 200;
	res.statusMessage = getStatusMessage(200);
	res.headers["Content-Type"] = "text/plain";
	res.headers["Content-Length"] = toString<size_t>(res.body.size());
	return res;
}

Response handleDelete() {
	Response res;

	res.version = "HTTP/1.1";
	res.statusCode = 204;
	res.statusMessage = getStatusMessage(204);
	return res;
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

std::string getStatusMessage(int statusCode) {
	switch(statusCode) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 400: return "Bad Request";
		case 404: return "Not Found";
		case 405: return "Method not Allowed";
		case 500: return "Internal Server Error";
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