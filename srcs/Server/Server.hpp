/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adebert <adebert@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 15:03:56 by allan             #+#    #+#             */
/*   Updated: 2025/09/21 18:27:11 by adebert          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>

// Forward declarations
struct ServerConfig;

#define CLIENT_TIMEOUT 20

void serverLoop(const std::vector<ServerConfig>& servers);


#endif