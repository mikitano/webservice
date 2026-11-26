#include "http/HttpSession.hpp"

#include "cgi/CgiHandler.hpp"

static const size_t	MAX_PENDING_OUTPUT = 1048576;

HttpSession::HttpSession(const Config& config, int listenPort,
						 const ClientInfo& client, RequestHandler& handler)
	: _request(&config, listenPort), _client(client), _handler(handler),
	  _output(), _closeAfterSend(false), _peerClosed(false), _cgi(NULL), _cgiServer(NULL),
	  _cgiKeepAlive(false), _cgiHeadOnly(false)
{
}

HttpSession::~HttpSession()
{
	delete _cgi;
}

void	HttpSession::onReceive(const char* data, size_t len)
{
	if (_closeAfterSend || len == 0)
		return;
	_request.feed(data, len);
	processInput();
}

void	HttpSession::onPeerClosed()
{
	_peerClosed = true;
	processInput();
	if (_cgi == NULL)
		_closeAfterSend = true;
}

void	HttpSession::onSent(size_t len)
{
	if (len > _output.size())
		len = _output.size();
	_output.erase(0, len);
	processInput();
}

bool	HttpSession::hasOutput() const { return !_output.empty(); }
const std::string&	HttpSession::getOutput() const { return _output; }

bool	HttpSession::shouldClose() const
{
	return _closeAfterSend && _output.empty() && _cgi == NULL;
}

void	HttpSession::processInput()
{
	while (_cgi == NULL && !_closeAfterSend && _output.size() <= MAX_PENDING_OUTPUT)
	{
		HttpRequest::ParseStatus	status = _request.feed(NULL, 0);

		if (_request.needsContinue())
		{
			_output += "HTTP/1.1 100 Continue\r\n\r\n";
			_request.continueSent();
		}
		if (status == HttpRequest::PARSE_INCOMPLETE)
		{
			if (_peerClosed)
				_closeAfterSend = true;
			return;
		}
		bool	headOnly = (_request.getMethod() == "HEAD");

		if (status == HttpRequest::PARSE_ERROR)
		{
			HttpResponse	error = _handler.errorResponse(_request.getErrorStatus(),
															_request.getServerConfig());
			error.setHeadOnly(headOnly);
			queue(error, true);
			return;
		}
		bool			keepAlive = _request.isKeepAlive();
		HttpResponse	response;
		CgiHandler*		cgi = NULL;

		if (_handler.handle(_request, _client, response, cgi))
		{
			response.setHeadOnly(headOnly);
			queue(response, !keepAlive);
		}
		else if (cgi != NULL)
		{
			_cgi = cgi;
			_cgiServer = _request.getServerConfig();
			_cgiKeepAlive = keepAlive;
			_cgiHeadOnly = headOnly;
		}
		else
		{
			HttpResponse	error = _handler.errorResponse(500, _request.getServerConfig());
			queue(error, !keepAlive);
		}
		nextRequest();
	}
}

void	HttpSession::nextRequest()
{
	std::string	rest = _request.takeLeftover();

	_request.reset();
	if (!rest.empty())
		_request.feed(rest.data(), rest.size());
}

void	HttpSession::queue(HttpResponse& response, bool closeAfter)
{
	if (closeAfter)
	{
		response.setHeader("Connection", "close");
		_closeAfterSend = true;
	}
	else
		response.setHeader("Connection", "keep-alive");
	_output += response.toString();
}

int	HttpSession::getCgiStdinFd() const
{
	return (_cgi == NULL) ? -1 : _cgi->getStdinFd();
}

int	HttpSession::getCgiStdoutFd() const
{
	return (_cgi == NULL) ? -1 : _cgi->getStdoutFd();
}

bool	HttpSession::isWaitingCgi() const { return _cgi != NULL; }

void	HttpSession::onCgiEvent(int fd, bool readable, bool writable, bool error)
{
	if (_cgi == NULL || fd < 0)
		return;
	if (fd == _cgi->getStdinFd())
	{
		if (writable || error)
			_cgi->onWritable();
	}
	else if (fd == _cgi->getStdoutFd())
	{
		if (readable)
			_cgi->onReadable();
		else if (error)
			_cgi->onHangup();
	}
	if (_cgi->isFinished())
		finishCgi();
}

void	HttpSession::checkTimeout()
{
	if (_cgi != NULL && _cgi->checkTimeout())
		finishCgi();
}

void	HttpSession::finishCgi()
{
	HttpResponse	response = _handler.cgiResponse(_cgi->getResult(), _cgiServer);

	if (_cgiHeadOnly)
		response.setHeadOnly(true);
	delete _cgi;
	_cgi = NULL;
	queue(response, !_cgiKeepAlive);
	processInput();
}

bool	HttpSession::hasPartialRequest() const
{
	return _cgi == NULL && _request.hasPartialData();
}

void	HttpSession::onClientTimeout()
{
	if (_cgi != NULL || _closeAfterSend)
		return;
	if (hasPartialRequest())
	{
		HttpResponse	timeout = _handler.errorResponse(408, _request.getServerConfig());
		queue(timeout, true);
	}
	else
		_closeAfterSend = true;
}
