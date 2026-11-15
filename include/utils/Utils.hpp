/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 19:38:40 by namatias          #+#    #+#             */
/*   Updated: 2026/11/11 19:50:58 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>
#include <iostream>

class Utils
{
	private:
		Utils(const Utils&);
		Utils& operator=(const Utils&);

	public:
		Utils();
		~Utils();

		static std::string  itoa(int);
};

#endif