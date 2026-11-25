/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 20:14:49 by namatias          #+#    #+#             */
/*   Updated: 2026/11/25 17:06:02 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Client.hpp"
#include "utils/Utils.hpp"
#include "utils/Logger.hpp"

#include <unistd.h>

Client::Client(int fd, struct sockaddr_in clientAdress):
	_fdClient(fd), 
	_info(Utils::ipv4ToString(clientAdress), static_cast<int>(ntohs(clientAdress.sin_port)))
{
	Logger::log(Logger::SERVER,
		"Client connected: fd=" + Utils::itoa(this->getFdClient()) +
		" from " + this->_info.getRemoteAddr() + ":" +
		Utils::itoa(this->_info.getRemotePort()));
}

Client::~Client()
{
	if (this->_fdClient >= 0)
	{
		Logger::log(Logger::DEBUG, 
			"Closing Client connection. fd = " + Utils::itoa(this->_fdClient));
		close(this->_fdClient);
		this->_fdClient = -1;
	}
}


int	Client::getFdClient() const
{
	return (this->_fdClient);
}

const ClientInfo&	Client::getInfo() const
{
	return (this->_info);
}