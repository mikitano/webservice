/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/06 16:23:31 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/07 01:40:54 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include <string>
#include <vector>
#include <cstddef>
#include "config/model/LocationConfig.hpp"

class ServerConfig {
	private:
		int							_port;
		std::vector<std::string>	_serverNames;
		size_t						_clientMaxBodySize;
		std::map<int, std::string>	_errorPages;
		std::vector<LocationConfig>	_locations;

	public:
		ServerConfig();
		// ServerConfig(const ServerConfig& copy);
		// ServerConfig& operator= (const ServerConfig& rhs);
		~ServerConfig();

		int getPort() const;
		const std::vector<std::string>& getServerNames() const;
		size_t getClientMaxBodySize() const;
		const std::map<int, std::string>& getErrorPages() const;
        std::string getErrorPage(int code) const;
        const std::vector<LocationConfig>& getLocations() const;

        void setPort(int port);
        void setClientMaxBodySize(size_t size);

		void addServerNames(const std::string& name);
        void addErrorPage(int code, const std::string& path);
        void addLocation(const LocationConfig& location);
};

#endif