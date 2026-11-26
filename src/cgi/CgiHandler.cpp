#include "cgi/CgiHandler.hpp"

#include <csignal>
#include <fcntl.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

#include "cgi/CgiEnv.hpp"

static const size_t	READ_CHUNK = 65536;
static const size_t	MAX_OUTPUT = 256 * 1024 * 1024;

CgiHandler::CgiHandler(const CgiParams& params, time_t timeoutSec)
	: _params(params), _state(IDLE), _pid(-1), _stdinFd(-1), _stdoutFd(-1),
	  _written(0), _output(), _startTime(0), _timeout(timeoutSec),
	  _exitStatus(0), _reaped(false)
{
}

CgiHandler::~CgiHandler()
{
	if (_pid > 0 && !_reaped)
	{
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
	}
	closeStdin();
	closeStdout();
}

static void	setCloseOnExec(int fd)
{
	fcntl(fd, F_SETFD, FD_CLOEXEC);
}

static std::string	numToString(size_t n)
{
	std::ostringstream	oss;

	oss << n;
	return oss.str();
}

bool	CgiHandler::start()
{
	if (_state != IDLE)
		return false;

	int	toChild[2];
	int	fromChild[2];

	if (pipe(toChild) < 0)
		return false;
	if (pipe(fromChild) < 0)
	{
		close(toChild[0]);
		close(toChild[1]);
		return false;
	}

	setCloseOnExec(toChild[0]);
	setCloseOnExec(toChild[1]);
	setCloseOnExec(fromChild[0]);
	setCloseOnExec(fromChild[1]);

	CgiEnv	env;
	env.set("GATEWAY_INTERFACE", "CGI/1.1");
	env.set("SERVER_PROTOCOL", "HTTP/1.1");
	env.set("SERVER_SOFTWARE", "webserv/1.0");
	env.set("REDIRECT_STATUS", "200");
	env.set("REQUEST_METHOD", _params.method);
	env.set("QUERY_STRING", _params.queryString);
	env.set("REQUEST_URI", _params.requestUri);
	env.set("SCRIPT_NAME", _params.scriptName);

	const std::string&	scriptArg = _params.scriptFile.empty()
		? _params.scriptPath : _params.scriptFile;
	env.set("SCRIPT_FILENAME", scriptArg);
	env.set("PATH_INFO", _params.pathInfo);
	if (!_params.pathInfo.empty())
		env.set("PATH_TRANSLATED", _params.pathTranslated);
	env.set("SERVER_NAME", _params.serverName);
	env.set("SERVER_PORT", numToString(static_cast<size_t>(_params.serverPort)));
	env.set("REMOTE_ADDR", _params.remoteAddr);
	if (!_params.contentType.empty())
		env.set("CONTENT_TYPE", _params.contentType);

	if (!_params.body.empty())
		env.set("CONTENT_LENGTH", numToString(_params.body.size()));
	env.addHttpHeaders(_params.headers);

	char**	envp = env.getEnvp();
	char*	argv[3];
	argv[0] = const_cast<char*>(_params.interpreter.c_str());
	argv[1] = const_cast<char*>(scriptArg.c_str());
	argv[2] = NULL;

	_pid = fork();
	if (_pid < 0)
	{
		close(toChild[0]);
		close(toChild[1]);
		close(fromChild[0]);
		close(fromChild[1]);
		_pid = -1;
		return false;
	}
	if (_pid == 0)
	{
		dup2(toChild[0], STDIN_FILENO);
		dup2(fromChild[1], STDOUT_FILENO);
		signal(SIGPIPE, SIG_DFL);

		for (int fd = 3; fd < 1024; ++fd)
			close(fd);
		if (!_params.scriptDir.empty() && chdir(_params.scriptDir.c_str()) != 0)
			_exit(1);
		execve(argv[0], argv, envp);
		_exit(1);
	}

	close(toChild[0]);
	close(fromChild[1]);
	_stdinFd = toChild[1];
	_stdoutFd = fromChild[0];
	fcntl(_stdinFd, F_SETFL, O_NONBLOCK);
	fcntl(_stdoutFd, F_SETFL, O_NONBLOCK);
	_startTime = time(NULL);
	_state = RUNNING;
	if (_params.body.empty())
		closeStdin();
	return true;
}

int	CgiHandler::getStdinFd() const { return _stdinFd; }
int	CgiHandler::getStdoutFd() const { return _stdoutFd; }

void	CgiHandler::onWritable()
{
	if (_stdinFd < 0)
		return;
	ssize_t	n = write(_stdinFd, _params.body.data() + _written,
		_params.body.size() - _written);

	if (n < 0)
	{
		closeStdin();
		return;
	}
	_written += static_cast<size_t>(n);
	if (_written >= _params.body.size())
		closeStdin();
}

void	CgiHandler::onReadable()
{
	if (_stdoutFd < 0)
		return;
	char	buf[READ_CHUNK];
	ssize_t	n = read(_stdoutFd, buf, sizeof(buf));

	if (n > 0)
	{
		_output.append(buf, static_cast<size_t>(n));
		if (_output.size() > MAX_OUTPUT)
			fail(FAILED);
	}
	else if (n == 0)
		finishOutput();
	else
		fail(FAILED);
}

void	CgiHandler::onHangup()
{
	finishOutput();
}

void	CgiHandler::reapChild()
{
	if (_reaped || _pid <= 0)
		return;
	int	status = 0;

	if (waitpid(_pid, &status, WNOHANG) == _pid)
	{
		_exitStatus = status;
		_reaped = true;
	}
}

void	CgiHandler::finishOutput()
{
	closeStdout();
	closeStdin();
	reapChild();
	_state = DONE;
}

void	CgiHandler::fail(State newState)
{
	if (_pid > 0 && !_reaped)
	{
		kill(_pid, SIGKILL);
		waitpid(_pid, &_exitStatus, 0);
		_reaped = true;
	}
	closeStdin();
	closeStdout();
	_state = newState;
}

bool	CgiHandler::checkTimeout()
{
	if (_state != RUNNING)
		return false;
	if (time(NULL) - _startTime > _timeout)
	{
		fail(TIMED_OUT);
		return true;
	}
	return false;
}

void	CgiHandler::abort()
{
	if (_state == RUNNING)
		fail(FAILED);
}

void	CgiHandler::closeStdin()
{
	if (_stdinFd >= 0)
	{
		close(_stdinFd);
		_stdinFd = -1;
	}
}

void	CgiHandler::closeStdout()
{
	if (_stdoutFd >= 0)
	{
		close(_stdoutFd);
		_stdoutFd = -1;
	}
}

CgiHandler::State	CgiHandler::getState() const { return _state; }

bool	CgiHandler::isFinished() const
{
	return _state == DONE || _state == FAILED || _state == TIMED_OUT;
}

CgiResult	CgiHandler::getResult() const
{
	CgiResult	r;

	if (_state == TIMED_OUT)
	{
		r.statusCode = 504;
		r.statusText = "Gateway Timeout";
	}
	else if (_state == DONE)
	{
		if (!CgiOutputParser::parse(_output, r))
		{
			r = CgiResult();
			r.statusCode = 502;
			r.statusText = "Bad Gateway";
		}
	}
	else
	{
		r.statusCode = 500;
		r.statusText = "Internal Server Error";
	}
	return r;
}
