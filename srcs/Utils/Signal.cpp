/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvoisin <mvoisin@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/21 14:45:21 by mvoisin           #+#    #+#             */
/*   Updated: 2025/09/21 14:55:17 by mvoisin          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Signal.hpp"

volatile sig_atomic_t g_stop = 0;

void signalHandler(int sig) {
    if (sig == SIGINT) {
        std::cout << "\n[Signal] Caught CTRL+C (SIGINT)" << std::endl;
        g_stop = 1;
    }
}

