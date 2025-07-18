/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Matprod <matprod42@gmail.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 14:07:08 by allan             #+#    #+#             */
/*   Updated: 2025/07/18 18:10:37 by Matprod          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <iostream>
#include <vector>
#include <sys/types.h>      // For data types like socklen_t
#include <sys/socket.h>     // For socket(), bind(), listen(), accept(), recv(), send()
#include <netinet/in.h>     // For sockaddr_in, htons(), htonl(), etc.
#include <arpa/inet.h>      // For inet_pton(), inet_ntop()
#include <unistd.h>         // For close()
#include <cstring>          // For memset(), memcpy()
#include <errno.h>          // For errno
#include <cstdio>           // For perror()

#include "../Config/Config.hpp"


void setupSockets(std::vector<ServerConfig>& config);

#endif