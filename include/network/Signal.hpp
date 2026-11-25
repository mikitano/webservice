/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 11:34:50 by namatias          #+#    #+#             */
/*   Updated: 2026/11/25 14:02:49 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNAL_HPP
#define SIGNAL_HPP

class Signal
{
	private:
		Signal();
		Signal(const Signal&);
		Signal& operator=(const Signal&);
		~Signal();

		static void 	handler(int);

	public:
		static bool	requestToStop();
		static void	setSigHandler();

};

#endif