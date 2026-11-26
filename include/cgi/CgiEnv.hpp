#ifndef CGIENV_HPP
#define CGIENV_HPP

#include <map>
#include <string>
#include <vector>

class CgiEnv
{
	public:
		CgiEnv();
		CgiEnv(const CgiEnv& other);
		CgiEnv& operator=(const CgiEnv& other);
		~CgiEnv();

		void	set(const std::string& key, const std::string& value);

		void	addHttpHeaders(const std::map<std::string, std::string>& headers);

		char**	getEnvp();
		size_t	size() const;

		static std::string	headerToEnvName(const std::string& headerName);

	private:
		std::map<std::string, std::string>	_vars;
		std::vector<std::string>			_flat;
		std::vector<char*>					_envp;
};

#endif
