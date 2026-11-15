/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 11:31:01 by namatias          #+#    #+#             */
/*   Updated: 2026/11/15 00:46:40 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <iostream>
#include <unistd.h>  
#include "network/SocketServer.hpp"

int main(int argc, char** argv) {
	int port = (argc > 1) ? std::atoi(argv[1]) : 8080;
    try
	{
        SocketServer sock(port);
        sock.initSocket();          // é aqui que o socket é criado de verdade
        // while (true)
            sleep(5);               // só para manter o programa vivo
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << std::endl;
        return 1;
    }
	return 0;
}