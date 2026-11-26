#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <ctime>
#include <map>
#include <string>
#include <sys/types.h>
#include "cgi/CgiOutputParser.hpp"

struct CgiParams
{
	std::string	interpreter;
	std::string	scriptPath;
	std::string	scriptFile;
	std::string	scriptDir;
	std::string	scriptName;
	std::string	pathInfo;
	std::string	pathTranslated;
	std::string	method;
	std::string	queryString;
	std::string	requestUri;
	std::string	contentType;
	std::string	body;
	std::map<std::string, std::string>	headers;
	std::string	serverName;
	int			serverPort;
	std::string	remoteAddr;

	CgiParams() : serverPort(0) {}
};

class CgiHandler
{
	public:
		enum State
		{
			IDLE,
			RUNNING,
			DONE,
			FAILED,
			TIMED_OUT
		};

		explicit CgiHandler(const CgiParams& params, time_t timeoutSec = 5);
		~CgiHandler();

		bool	start();

		int		getStdinFd() const;
		int		getStdoutFd() const;

		void	onWritable();
		void	onReadable();
		void	onHangup();

		bool	checkTimeout();
		void	abort();

		State		getState() const;
		bool		isFinished() const;
		CgiResult	getResult() const;

	private:
		CgiHandler(const CgiHandler&);
		CgiHandler& operator=(const CgiHandler&);

		void	closeStdin();
		void	closeStdout();
		void	reapChild();
		void	finishOutput();
		void	fail(State newState);

		CgiParams		_params;
		State			_state;
		pid_t			_pid;
		int				_stdinFd;
		int				_stdoutFd;
		size_t			_written;
		std::string		_output;
		time_t			_startTime;
		time_t			_timeout;
		int				_exitStatus;
		bool			_reaped;
};

#endif
