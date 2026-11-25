/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 19:38:40 by namatias          #+#    #+#             */
/*   Updated: 2026/11/24 21:57:02 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <netinet/in.h>

class Utils
{
	private:
		Utils(const Utils&);
		Utils& operator=(const Utils&);

	public:
		Utils();
		~Utils();

		/*
		** Helpers
		** Setam as configuraçoes de FDs para:
		** 1 - Não Bloqueante (O_NONBLOCK)
		** 2 - Fechar ao ser herdado por um processo filho (FD_CLOEXEC)
		** 3 - Poder ser reautilizado após uma conexão fechar (SO_REUSEADDR)
		*/
		static bool		setCloexec(int);
		static bool		setNonBlocking(int);
		static bool		setReuseAddr(int);

		static			std::string	itoa(int);
		static const	std::string	ipv4ToString(const struct sockaddr_in&);
};

#endif