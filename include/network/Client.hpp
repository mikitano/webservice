/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 18:05:56 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 14:24:49 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	CLIENT_HPP
#define	CLIENT_HPP

#include "network/ClientInfo.hpp"
#include <netinet/in.h>

class Client
{
	private:
		int				_fdClient;
		ClientInfo		_info;

		Client();
		Client(const Client&);
		Client& operator=(const Client&);

	public:
		~Client();

		Client(int, struct sockaddr_in);
		
		int					getFdClient() const;
		const ClientInfo&	getInfo() const;

};


#endif