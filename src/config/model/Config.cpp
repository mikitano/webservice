/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42sp.org.br>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 14:44:54 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/13 03:37:05 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/model/Config.hpp"

Config::Config() {}

Config::Config(const Config& copy)
{
	*this = copy;
}

Config& Config::operator=(const Config& rhs)
{
	if (this != &rhs)
	{
		_servers = rhs._servers;
	}
	return (*this);
}

Config::~Config() {}

// getters

const std::vector<ServerConfig>& Config::getServers() const
{
	return (_servers);
}

// adders

void Config::addServer(const ServerConfig& server)
{
	_servers.push_back(server);
}