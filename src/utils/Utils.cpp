/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 19:35:48 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 14:25:07 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Utils.hpp"

#include <string>
#include <sstream>

Utils::Utils(){}
Utils::~Utils(){}

std::string Utils::itoa(int n)
{
	std::ostringstream output;
	output << n;
	return (output.str());
};

/* 
** Converte o IP da struct preenchida pelo accept() em std::string
** equivalente ao inet_ntop (forma manual p/ respeitar subject)
*/
const std::string Utils::ipv4ToString(const struct sockaddr_in& addr) 
{
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&addr.sin_addr);

	std::ostringstream output;
	output	<< static_cast<int>(bytes[0]) << "."
			<< static_cast<int>(bytes[1]) << "."
			<< static_cast<int>(bytes[2]) << "."
			<< static_cast<int>(bytes[3]);
	return (output.str());
}