/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 19:38:40 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 14:25:01 by namatias         ###   ########.fr       */
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

		static			std::string	itoa(int);
		static const	std::string	ipv4ToString(const struct sockaddr_in&);
};

#endif