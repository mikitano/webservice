/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/25 13:38:43 by namatias          #+#    #+#             */
/*   Updated: 2026/11/25 16:42:42 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/Signal.hpp"

#include <csignal>

/*
** Enquanto g_stop = 0, ou seja, ninguem pressionou Ctrl + C, a func
** requestToStop retorna false (g_stop != 0)
** Após pressionar Ctrl + C, handler intercepta e muda g_stop para 1
** e por sua vez requestToStop passa a retornar true e encerra o poll()
** OBS: looping do poll na classe Server !!  
*/

static volatile std::sig_atomic_t g_stop = 0;

void 	Signal::handler(int signum)
{
	(void)signum;
	g_stop = 1;
}

bool	Signal::requestToStop()
{
	return (g_stop != 0);
}

/*
** SIGINT -> macro para sinal Ctrl + C
** SIGPIPE -> macro para o sinal de quando um send() tentar enviar algo para 
** um cliente que já desconectou. Assim evita crashar o server inteiro
*/
void	Signal::setSigHandler()
{
	std::signal(SIGINT, Signal::handler);
	std::signal(SIGPIPE, SIG_IGN);
}