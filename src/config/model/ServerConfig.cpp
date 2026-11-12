/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42sp.org.br>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 14:44:59 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/11 22:32:25 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/model/ServerConfig.hpp"

ServerConfig::ServerConfig()
	: _port(0),
	  _clientMaxBodySize(0) {}

ServerConfig::ServerConfig(const ServerConfig& copy)
{
	*this = copy;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& rhs)
{
	if (this != &rhs)
	{
		_port = rhs._port;
		_serverNames = rhs._serverNames;
		_clientMaxBodySize = rhs._clientMaxBodySize;
		_errorPages = rhs._errorPages;
		_locations = rhs._locations;
	}
	return (*this);
}

ServerConfig::~ServerConfig() {}

// getters

int ServerConfig::getPort() const
{
	return (_port);
}

const std::vector<std::string>& ServerConfig::getServerNames() const
{
	return (_serverNames);
}

std::size_t ServerConfig::getClientMaxBodySize() const
{
	return (_clientMaxBodySize);
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
	return (_errorPages);
}

std::string ServerConfig::getErrorPage(int code) const
{
	std::map<int, std::string>::const_iterator it;
	it = _errorPages.find(code);

	if (it != _errorPages.end())
		return (it->second);
	return ("");
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const
{
	return (_locations);
}

// setters

void ServerConfig::setPort(int port)
{
	_port = port;
}

void ServerConfig::setClientMaxBodySize(std::size_t size)
{
	_clientMaxBodySize = size;
}

// adders

void ServerConfig::addServerName(const std::string& name)
{
	_serverNames.push_back(name);
}

void ServerConfig::addErrorPage(int code, const std::string& path)
{
	_errorPages[code] = path;
}

void ServerConfig::addLocation(const LocationConfig& location)
{
	_locations.push_back(location);
}