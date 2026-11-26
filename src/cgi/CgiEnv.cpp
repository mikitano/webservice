#include "cgi/CgiEnv.hpp"

CgiEnv::CgiEnv() {}

CgiEnv::CgiEnv(const CgiEnv& other) : _vars(other._vars) {}

CgiEnv&	CgiEnv::operator=(const CgiEnv& other)
{
	if (this != &other)
	{
		_vars = other._vars;
		_flat.clear();
		_envp.clear();
	}
	return *this;
}

CgiEnv::~CgiEnv() {}

void	CgiEnv::set(const std::string& key, const std::string& value)
{
	_vars[key] = value;
}

static std::string	lower(const std::string& s)
{
	std::string	r(s);

	for (size_t i = 0; i < r.size(); ++i)
		if (r[i] >= 'A' && r[i] <= 'Z')
			r[i] = static_cast<char>(r[i] + 32);
	return r;
}

std::string	CgiEnv::headerToEnvName(const std::string& headerName)
{
	std::string	name = "HTTP_";

	for (size_t i = 0; i < headerName.size(); ++i)
	{
		char	c = headerName[i];

		if (c == '-')
			c = '_';
		else if (c >= 'a' && c <= 'z')
			c = static_cast<char>(c - 32);
		name += c;
	}
	return name;
}

void	CgiEnv::addHttpHeaders(const std::map<std::string, std::string>& headers)
{
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		it != headers.end(); ++it)
	{
		std::string	key = lower(it->first);

		if (key == "content-type" || key == "content-length"
			|| key == "transfer-encoding")
			continue;
		set(headerToEnvName(it->first), it->second);
	}
}

char**	CgiEnv::getEnvp()
{
	_flat.clear();
	_envp.clear();
	for (std::map<std::string, std::string>::const_iterator it = _vars.begin();
		it != _vars.end(); ++it)
		_flat.push_back(it->first + "=" + it->second);
	for (size_t i = 0; i < _flat.size(); ++i)
		_envp.push_back(const_cast<char*>(_flat[i].c_str()));
	_envp.push_back(NULL);
	return &_envp[0];
}

size_t	CgiEnv::size() const
{
	return _vars.size();
}
