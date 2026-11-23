/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/21 17:12:20 by namatias          #+#    #+#             */
/*   Updated: 2026/11/23 14:24:54 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	CLIENTINFO_HPP
#define	CLIENTINFO_HPP

#include <iostream>

class ClientInfo
{
	private:
		const std::string	_remoteAddr;    // "127.0.0.1"
		int					_remotePort;	// 4242

		ClientInfo();
		ClientInfo (const ClientInfo&);
		ClientInfo& operator=(const ClientInfo&);

	public:
		~ClientInfo();
		ClientInfo(const std::string&, int);

		const std::string&	getRemoteAddr() const;
		int					getRemotePort() const;
};

#endif