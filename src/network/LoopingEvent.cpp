/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LoopingEvent.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: namatias <namatias@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/14 17:51:53 by namatias          #+#    #+#             */
/*   Updated: 2026/11/21 15:48:11 by namatias         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "network/LoopingEvent.hpp"
#include "utils/Logger.hpp"

/*
** Struct Constructor
*/
// LoopingEvent::FdEvent::FdEvent(): _fd(-1), _readable(false), _writable(false), _error(false) {}

LoopingEvent::LoopingEvent(){}

LoopingEvent::~LoopingEvent(){}

void	LoopingEvent::addFdPoll(int fd)
{
	/*
	** Verificamos se esse fd é de algum cliente/processo que ja foi add ao poll
	*/
	if (findIndex(fd) >= 0)
		return;

	/*
	** Struc pollfd construida pela biblioteca poll.h
	** Os 3 campos sao de preenchimento manual e obrigatorio
	*/
	struct pollfd newFd;
	newFd.fd = fd;
	newFd.events = POLLIN;
	newFd.revents = 0;

	this->_pollFds.push_back(newFd);
}


void	LoopingEvent::removeFd(int fd)
{
	/*
	** Verificamos se esse fd ainda existe ou já foi fechado
	** Se existir apagamos a posiçao em q ele esta (index = findIndex)
	*/
	if (findIndex(fd) >= 0)
		_pollFds.erase(_pollFds.begin() + findIndex(fd));

	/*
	** Tambem retiramos da lista de Fds que estavam aguardando uma açao
	*/
	for (size_t i = 0; i < _ready.size(); i++)
	{
		if (_ready[i]._fd == fd)
		{
			_ready.erase(_ready.begin() + i);
			break;
		}
	}
}

void	LoopingEvent::setPollIn(int fd, bool enable)
{
	/*
	** Verificamos se o _pollFds está monitorando esse fd
	** Alteramos o valor dele no _pollFds
	*/
	if (findIndex(fd) < 0)
	{
		Logger::log(Logger::DEBUG, "setPollIn: failed to find fd");
		return ;
	}

	/*
	** Operador bit a bit (bitwise)
	** os operadores usados nesse passo ligam ou desligam um bit do campo events
	** sem mexer nos outros.
	** _pollFds[index].events -> local onde nossa flag de evento fica armazenada
	** |= representa OR -> liga o POLLIN
	** &= ~(AND + NOT) desliga o POLLIN
	*/
	if (enable)
		_pollFds[findIndex(fd)].events |= POLLIN;
	else
		_pollFds[findIndex(fd)].events &= ~POLLIN;
}

void	LoopingEvent::setPollOut(int fd, bool enable)
{
	/*
	** Verificamos se o _pollFds está monitorando esse fd
	** Alteramos o valor dele no _pollFds
	*/
	if (findIndex(fd) < 0)
	{
		Logger::log(Logger::DEBUG, "setPollOut: failed to find fd");
		return ;
	}

	if (enable)
		_pollFds[findIndex(fd)].events |= POLLOUT;
	else
		_pollFds[findIndex(fd)].events &= ~POLLOUT;
}


bool	LoopingEvent::hasFd(int fd) const
{
	if (findIndex(fd) < 0)
		return (false);
	return (true);
}

size_t	LoopingEvent::size(void) const
{
	return (_pollFds.size());
}

/*
** Toda vez que a varredura ocorre é formada uma nova lista _ready
** _ready é atualizado constantemente com os FDs prontos para ler/escrever
**
** O poll calcula sozinha se o timeoutMs foi atingido, "acorda" e roda novamente
**
** Caso ocorra algum evento o poll tbm acorda ainda q o tempo n tenha sido atingido
** Retornos
** 		-1 interrompido por sinal
**		0 tudo OK, retorno esperado
**		Mesmo se >0 pode dar 3 erros possiveis, por isso temos q testar TODOS os result dos fds
**
** SE td estiver ok, vamos preencher o FdEvent com os resultados (indice por indice)
** ao preencher o _error descobrimos se voltou um dos erros possíveis
** estando tudo certo o FdEvent é acrescentao ao vetor _ready para ser executado
**
** REVENTS, é preenchido automaticamente pelo poll e reponde se nossa pergunta (event) pode ser respondida
** Ex: event = POLLIN ->  Pergunta: tem dado a ser Lido ?
**     revents = POLLIN ->  resposta: sim, tem
**     revents = 0      ->  resposta: ainda não
**	   revents = POLLERR/POLLHUP/POLLNVAL se o fd teve problema, mesmo que não tenhamos perguntado
*/
int	LoopingEvent::wait(int timeoutMs)
{
	_ready.clear();

	int result = poll(&_pollFds[0], _pollFds.size(), timeoutMs);
	if (result < 0)
		return (-1);

	for (size_t i = 0; i < _pollFds.size(); i++)
	{
		short reventPoll = _pollFds[i].revents;
		if (reventPoll == 0)
			continue ;

		/*
		** (reventPoll & MACRO) faz um betwise entre os 2  comparando os valores
		** equivalente a um == , porem como pode vir mais de um macro no revent precisamos usar bitwise
		**
		** a comparaçap com != 0 , faz o resultado final ser 0 ou 1 (true ou false)
		*/
		FdEvent eventPoll;
		eventPoll._fd = _pollFds[i].fd;
		eventPoll._readable = (reventPoll & POLLIN) != 0;
		eventPoll._writable = (reventPoll & POLLOUT) != 0;
		eventPoll._error = (reventPoll & (POLLERR | POLLHUP | POLLNVAL)) != 0;
		_ready.push_back(eventPoll);
	}
	return (result);
}

int	LoopingEvent::findIndex(int fd) const
{
	for (size_t i = 0; i < _pollFds.size(); i++)
	{
		if (_pollFds[i].fd == fd)
			return (static_cast<int>(i));
	}
	return (-1);
}

const std::vector<LoopingEvent::FdEvent>&	LoopingEvent::getReady() const
{
	return (_ready);
}
