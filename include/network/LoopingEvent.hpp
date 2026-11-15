#ifndef LOOPINGEVENT_HPP
#define LOOPINGEVENT_HPP

#include <vector>
#include <map>
#include <poll.h>
#include <cstddef>

/*
** Encapsula o único poll() do programa e o vetor de fds
*/
class LoopingEvent
{
	

	public:
		/*
		** Fd a ser processado pelo recv/send
		*/
		struct FdEvent
		{
			int		_fd;
			bool	_readable;		// ler -> pollin
			bool	_writable;		// escrever -> pollout
			bool	_error;			// POLLERR, POLLHUP ou POLLNVAL
		};

		void	addFdPoll(int fd);
		void	removeFd(int fd);
		void	setPollIn(int fd, bool enable);
		void	setPollOut(int fd, bool enable);

		bool	hasFd(int fd) const;
		size_t	size(void) const;
		
		int							wait(int timeoutMs);
		const std::vector<FdEvent>&	getReady(void) const;

		LoopingEvent();
		~LoopingEvent();

	private:
		std::vector<struct pollfd>				_pollFds;
		std::vector<LoopingEvent::FdEvent>		_ready;
		
		int	findIndex(int fd) const;

		LoopingEvent(const LoopingEvent&);
		LoopingEvent&	operator=(const LoopingEvent&);
};

#endif