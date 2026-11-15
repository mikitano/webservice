/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 15:49:55 by namatias          #+#    #+#             */
/*   Updated: 2026/11/11 18:26:45 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

class Logger
{
	private:
		Logger(const Logger&);
		Logger& operator=(const Logger&);

	public:
		Logger();
		~Logger();

		enum LogLevel
		{
			SERVER,
			ERROR,
			DEBUG
		};

		static void		log(LogLevel level, const std::string&);
		static std::string	getLevel(LogLevel level);
};

#endif
