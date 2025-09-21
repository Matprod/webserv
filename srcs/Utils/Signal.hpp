/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/21 14:45:17 by mvoisin           #+#    #+#             */
/*   Updated: 2025/09/21 14:59:16 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <csignal>
#include "../../includes/Webserv.hpp"


extern volatile sig_atomic_t g_stop;
void signalHandler(int sig);
