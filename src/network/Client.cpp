/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 20:14:49 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 14:01:11 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Client.hpp"
#include "utils/Utils.hpp"
#include "utils/Logger.hpp"

#include <unistd.h>

Client::Client(int fd, struct sockaddr_in clientAdress):
	_fdClient(fd), 
	_info(Utils::ipv4ToString(clientAdress), static_cast<int>(htons(clientAdress.sin_port)))
{
	Logger::log(Logger::SERVER, "Client connect on Fd = " + Utils::itoa(this->getFdClient()));
}

Client::~Client()
{
	if (this->_fdClient >= 0)
	{
		Logger::log(Logger::SERVER, "Closing Client connection. fd = " + Utils::itoa(this->_fdClient));
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