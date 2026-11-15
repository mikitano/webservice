/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 18:27:01 by namatias          #+#    #+#             */
/*   Updated: 2026/11/11 18:29:37 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Logger.hpp"

Logger::Logger(){}

Logger::~Logger(){}

void	Logger::log(LogLevel level, const std::string& msg)
{
	std::cout << getLevel(level) << msg << std::endl;
}

std::string	Logger::getLevel(LogLevel level)
{
	switch (level) 
	{
		case SERVER:
			return "\033[1;32m[SERVER]\033[0m ";
		case ERROR:
			return "\033[1;31m[ERROR]\033[0m ";
    	case DEBUG:
			return "\033[1;34m[DEBUG]\033[0m ";
		default:
            return "[DEFAULT] ";
    }
}