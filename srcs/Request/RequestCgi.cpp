/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCgi.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 12:12:16 by Matprod           #+#    #+#             */
/*   Updated: 2025/07/30 16:18:00 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestCgi.hpp"

std::string getFileExtension(const std::string& uri) {
    size_t dot_pos = uri.find_last_of('.');
    if (dot_pos == std::string::npos || dot_pos == uri.length() - 1)
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

std::string getQueryString(const std::string& uri) {
    size_t query_pos = uri.find('?');
    if (query_pos != std::string::npos)
        return uri.substr(query_pos + 1);
    return "";
}

LocationConfig* findMatchingLocation(const std::string& uri, const std::vector<LocationConfig>& locations) {
    for (size_t i = 0; i < locations.size(); ++i) {
        if (uri.find(locations[i].path) == 0) // Vérifie si l'URI commence par le chemin de la location
            return const_cast<LocationConfig*>(&locations[i]);
    }
    return NULL;
}