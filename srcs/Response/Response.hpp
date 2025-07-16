/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: allan <allan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:58:27 by allan             #+#    #+#             */
/*   Updated: 2025/07/16 15:16:35 by allan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include "../Request/Request.hpp"

struct Response {
	std::string version;
	int statusCode;
	std::string statusMessage;
	std::map<std::string, std::string> headers;
	std::string body;	

	std::string responseToString() const;
};

template <typename T>
std::string toString(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

Response buildResponse(const Request& request);
Response handleGet();
Response handlePost(const Request& request);
Response handleDelete();
std::string getStatusMessage(int statusCode);

#endif