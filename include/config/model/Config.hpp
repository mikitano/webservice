/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkitano <mkitano@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/06 16:16:07 by mkitano           #+#    #+#             */
/*   Updated: 2026/11/07 01:19:28 by mkitano          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include "config/model/ServerConfig.hpp"

class Config {
	private:
		std::vector<ServerConfig> _servers;

	public:
		Config();
		// Config(const Config& copy);
		// Config& operator=(const Config& rhs);
		~Config();

		const std::vector<ServerConfig>& getServers() const;
		void addServer(const ServerConfig& server);
};

#endif