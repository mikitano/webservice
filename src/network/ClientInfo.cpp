/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 17:42:18 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 13:33:48 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/ClientInfo.hpp"
#include "utils/Utils.hpp"
#include "utils/Logger.hpp"

ClientInfo::ClientInfo(const std::string& remoteAddr, int remotePort):
	_remoteAddr(remoteAddr), _remotePort(remotePort) 
{
	Logger::log(Logger::DEBUG, "Client info struct created");
}

ClientInfo::~ClientInfo()
{
	Logger::log(Logger::DEBUG, "Client info struct cleaned");
}

const std::string&	ClientInfo::getRemoteAddr() const
{
	return (this->_remoteAddr);
}

int	ClientInfo::getRemotePort() const
{
	return (this->_remotePort);
}
