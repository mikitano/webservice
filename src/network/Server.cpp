/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 17:32:15 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 21:20:18 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Server.hpp"

#include "utils/Utils.hpp"
#include "utils/Logger.hpp"

#include <unistd.h>  


Server::Server(const std::vector<ServerConfig>& Servers) 
{
	Logger::log(Logger::SERVER, "Starting Server");
	for (size_t i = 0; i < Servers.size(); i++)
	{
		this->_servers.push_back(Servers[i]);
	}
	if (_servers.size())
	{
		Logger::log(Logger::DEBUG, "_server list created");
	}
}

Server::~Server() {}

void	Server::run()
{
	try
	{
		Server::init();
		// while (true)
		// {
		// 	sleep(1);
		// }
	}
	catch(const std::exception& e)
	{
		Server::stop();
		throw ;
	}
}

void	Server::init()
{
	for (size_t i = 0; i < this->_servers.size(); i++)
	{
		int port = this->_servers[i].getPort();
		this->_socketServer[i] = new SocketServer(port);
		this->_socketServer[i]->initSocket();
	}
}

/*
** Limpar todos o std::map e std::vector da nossa classe
*/
void	Server::stop()
{
	Logger::log(Logger::SERVER, "Closing Server");
	
	cleanClientList();
	cleanSocketServer();

	Logger::log(Logger::SERVER, "Server Closed");
}

void	Server::cleanSocketServer()
{
	std::map<int, SocketServer*>::iterator index;
	for (index =  this->_socketServer.begin();  index != this->_socketServer.end(); index++)
	{
		delete index->second;
	}
	this->_socketServer.clear();
	Logger::log(Logger::SERVER, "All Socket Servers closed");
}

void	Server::cleanClientList()
{
	std::map<int, Client*>::iterator index;
	for (index =  this->_clients.begin();  index != this->_clients.end(); index++)
	{
		delete index->second;
	}
	this->_clients.clear();
	Logger::log(Logger::SERVER, "All Clients Fd`s closed");
}
