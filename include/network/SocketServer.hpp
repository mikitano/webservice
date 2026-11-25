/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 11:31:01 by namatias          #+#    #+#             */
/*   Updated: 2026/11/24 21:40:12 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETSERVER_HPP
#define SOCKETSERVER_HPP

/*
** Um SocketServer = um listener (host:porta) em estado de listen.
** Vários server blocks do config podem apontar para o mesmo SocketServer.
*/ 
class SocketServer
{
	private:
		int									_fdSocket;
		int									_port;

		SocketServer();
		SocketServer(const SocketServer&);
		SocketServer&	operator=(const SocketServer&);
 
	public:
		~SocketServer();
		SocketServer(int port);

		/*
		** Resolve todo a maior parte do ciclo de vida do socket
		** Cria, bind e deixa no estado de listening
		** Ordem: socket, SO_REUSEADDR, O_NONBLOCK, FD_CLOEXEC, bind, listen.
		** SE falhar lança exceção correspondente
		*/
		void	initSocket();

		int		getFdSocket() const;
		int		getPortSocket() const;
};

#endif
