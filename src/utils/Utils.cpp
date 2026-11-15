/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 19:35:48 by namatias          #+#    #+#             */
/*   Updated: 2026/11/14 11:38:27 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils/Utils.hpp"

Utils::Utils(){}
Utils::~Utils(){}

std::string Utils::itoa(int n)
{
	std::stringstream output;
	output << n;
	return (output.str());
};