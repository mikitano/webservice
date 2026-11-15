/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketServer.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 11:31:01 by namatias          #+#    #+#             */
/*   Updated: 2026/11/08 11:35:41 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/SocketServer.hpp"
#include "utils/Logger.hpp"
#include "utils/Utils.hpp"

#include <cstring>        // memset, strerror
#include <stdexcept>      // std::runtime_error
#include <unistd.h>       // close
#include <fcntl.h>        // fcntl
#include <cerrno>		  // errno
#include <sys/socket.h>   // socket, bind, listen, setsockopt
#include <netinet/in.h>   // sockaddr_in, htons, htonl

SocketServer::~SocketServer()
{
	if (this->_fdSocket >= 0)
	{
		Logger::log(Logger::DEBUG, "Closing socket. fd = " + Utils::itoa(this->_fdSocket));
		close(this->_fdSocket);
		this->_fdSocket = -1;
	}
}

SocketServer::SocketServer(int port): _fdSocket(-1), _port(port){}

/*
** Resolve a maior parte do ciclo de vida do socket
** Cria, faz bind e deixa no estado de listening
** Ordem com flags: socket, SO_REUSEADDR, O_NONBLOCK, FD_CLOEXEC, bind, listen.
** SE falhar lança exceção correspondente
*/
void	SocketServer::initSocket()
{
	this->_fdSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_fdSocket < 0)
		throw std::runtime_error("Socket: failed to create socket");
	Logger::log(Logger::DEBUG, "Socket created. fd = " + Utils::itoa(this->getFdSocket()));
	
	if (!setReuseAddr(this->_fdSocket) | !setNonBlocking(this->_fdSocket)| !setCloexec(this->_fdSocket))
		throw std::runtime_error("Socket failed: " + std::string(std::strerror(errno)));
	Logger::log(Logger::DEBUG, "Socket options and descriptor flags configured successfully");
	
	struct sockaddr_in myAddr;

	memset(&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(this->_port);
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(this->_fdSocket, (struct sockaddr *) &myAddr, sizeof (myAddr)) < 0)
		throw std::runtime_error("bind() failed: " + std::string(std::strerror(errno)));
	Logger::log(Logger::DEBUG, "bind() completed on port " + Utils::itoa(this->getPortSocket()));
	
	if (listen(this->_fdSocket, SOMAXCONN) < 0)
        throw std::runtime_error("listen() failed: " + std::string(std::strerror(errno)));
	Logger::log(Logger::DEBUG, "listen() completed");

	Logger::log(Logger::SERVER, "Listening on *:" + Utils::itoa(this->getPortSocket()));
	Logger::log(Logger::SERVER, "Local test: http://127.0.0.1:" + Utils::itoa(this->getPortSocket()));
}

/*
** Helpers de status
*/
bool	SocketServer::setCloexec(int fdSocket)
{
	if (fcntl(fdSocket, F_SETFD, FD_CLOEXEC) < 0)
		return (false);
	return (true);
}

bool	SocketServer::setNonBlocking(int fdSocket)
{
	int flags = 1;

	flags = fcntl(fdSocket, F_GETFL, 0);
	if (flags < 0)
		return (false);

	if (fcntl(fdSocket, F_SETFL, flags | O_NONBLOCK) < 0)
		return (false);
	return (true);
}

bool	SocketServer::setReuseAddr(int fdSocket)
{
	int success = 1;

	if (setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, &success, sizeof success) < 0)
		return (false);
	return (true);
}

/*
** Getters
*/
int	SocketServer::getFdSocket() const
{
	return (this->_fdSocket);
}

int	SocketServer::getPortSocket() const
{
	return (this->_port);
}
