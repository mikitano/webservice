/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 19:35:48 by namatias          #+#    #+#             */
/*   Updated: 2026/11/24 21:56:08 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Utils.hpp"

#include <string>
#include <sstream>
#include <fcntl.h>
#include <sys/socket.h>

Utils::Utils(){}
Utils::~Utils(){}

std::string Utils::itoa(int n)
{
	std::ostringstream output;
	output << n;
	return (output.str());
}

/* 
** Converte o IP da struct preenchida pelo accept() em std::string
** equivalente ao inet_ntop (forma manual p/ respeitar subject)
*/
const std::string Utils::ipv4ToString(const struct sockaddr_in& addr) 
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&addr.sin_addr);

	std::ostringstream output;
	output	<< static_cast<int>(bytes[0]) << "."
			<< static_cast<int>(bytes[1]) << "."
			<< static_cast<int>(bytes[2]) << "."
			<< static_cast<int>(bytes[3]);
	return (output.str());
}

/*
** Helpers de status de FD
*/
bool	Utils::setCloexec(int fdSocket)
{
	if (fcntl(fdSocket, F_SETFD, FD_CLOEXEC) < 0)
		return (false);
	return (true);
}

bool	Utils::setNonBlocking(int fdSocket)
{
	if (fcntl(fdSocket, F_SETFL, O_NONBLOCK) < 0)
		return (false);
	return (true);
}

bool	Utils::setReuseAddr(int fdSocket)
{
	int success = 1;

	if (setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, &success, sizeof success) < 0)
		return (false);
	return (true);
}