/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 16:21:09 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 21:25:15 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	SERVER_HPP
#define	SERVER_HPP

#include "config/model/Config.hpp"
#include "network/Client.hpp"
#include "network/ClientInfo.hpp"
#include "network/LoopingEvent.hpp"
#include "network/SocketServer.hpp"

#include <iostream>

class Server
{
	/*
	** Containers do tipo std::map organizam os dados por pares: 1 - Struct
	** No caso std::map<int, SocketServer*> usamos : 
	**
	** index->first : Representa a porçao KEY (int)
	** index->second : Representa a porçao VALUE (Socket*)
	*/
	private:
		LoopingEvent					_loop;
		std::vector<ServerConfig>		_servers;
		std::map<int, SocketServer*>	_socketServer;
		std::map<int, Client*>			_clients;
		
		
		//add mapa para os fds do CGI ---- Decidir como vai funcionar com o Breno!!


		Server();
		Server(const Server&);
		Server& operator=(const Server&);
	public:
		~Server();

		Server(const std::vector<ServerConfig>&);
		
		/*
		** Ciclo de Vida do Server
		*/
		void	init();
		void	run();
		void	stop();

		/*
		** Anti-leaks
		*/
		void	cleanSocketServer();
		void	cleanClientList();
};

#endif