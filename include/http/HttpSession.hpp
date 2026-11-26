#ifndef HTTPSESSION_HPP
#define HTTPSESSION_HPP

#include <cstddef>
#include <string>

#include "config/model/Config.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/RequestHandler.hpp"
#include "network/ClientInfo.hpp"

class CgiHandler;

/*
** Uma HttpSession por cliente. Não faz recv/send e não mexe no poll.
**
**   recv() > 0                -> onReceive(buf, n)
**   recv() == 0               -> onPeerClosed() (ainda pode haver resposta para enviar)
**   hasOutput()               -> POLLOUT; send(getOutput()) -> onSent(bytes enviados)
**   shouldClose()             -> fechar o cliente
**   getCgiStdinFd()  >= 0     -> vigiar com POLLOUT
**   getCgiStdoutFd() >= 0     -> vigiar com POLLIN
**   evento num fd do CGI      -> onCgiEvent(fd, POLLIN, POLLOUT, POLLERR|POLLHUP)
**   a cada volta do loop      -> checkTimeout()
**   cliente parado            -> onClientTimeout()
**
** Depois de qualquer chamada, os fds do CGI podem ter mudado (-1 = fechado).
** delete da sessão mata o CGI em andamento e fecha os pipes.
*/
class HttpSession
{
	public:
		HttpSession(const Config& config, int listenPort,
					const ClientInfo& client, RequestHandler& handler);
		~HttpSession();

		void				onReceive(const char* data, size_t len);
		void				onPeerClosed();
		void				onSent(size_t len);

		bool				hasOutput() const;
		const std::string&	getOutput() const;
		bool				shouldClose() const;

		int					getCgiStdinFd() const;
		int					getCgiStdoutFd() const;
		void				onCgiEvent(int fd, bool readable, bool writable, bool error);
		void				checkTimeout();

		bool				isWaitingCgi() const;
		bool				hasPartialRequest() const;
		void				onClientTimeout();

	private:
		HttpSession();
		HttpSession(const HttpSession&);
		HttpSession& operator=(const HttpSession&);

		void	processInput();
		void	nextRequest();
		void	finishCgi();
		void	queue(HttpResponse& response, bool closeAfter);

		HttpRequest			_request;
		const ClientInfo&	_client;
		RequestHandler&		_handler;
		std::string			_output;
		bool				_closeAfterSend;
		bool				_peerClosed;

		CgiHandler*			_cgi;
		const ServerConfig*	_cgiServer;
		bool				_cgiKeepAlive;
		bool				_cgiHeadOnly;
};

#endif
