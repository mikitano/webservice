/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42sp.org.br>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/06 16:16:07 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/12 02:51:30 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include "ServerConfig.hpp"

class Config {
	private:
		std::vector<ServerConfig> _servers;

	public:
		Config();
		Config(const Config& copy);
		Config& operator=(const Config& rhs);
		~Config();

		const std::vector<ServerConfig>& getServers() const;
		void addServer(const ServerConfig& server);
};

#endif