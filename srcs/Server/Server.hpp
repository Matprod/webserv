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

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <poll.h>

#include "../Config/Config.hpp"
#include "../Request/Request.hpp"
#include "../Response/Response.hpp"
#include "Client.hpp"

#define CLIENT_TIMEOUT 5

void serverLoop(const std::vector<ServerConfig>& servers);


#endif