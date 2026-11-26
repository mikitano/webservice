#include "cgi/CgiOutputParser.hpp"

#include <cstdlib>

std::string	CgiOutputParser::toLower(const std::string& str)
{
	std::string	r(str);

	for (size_t i = 0; i < r.size(); ++i)
		if (r[i] >= 'A' && r[i] <= 'Z')
			r[i] = static_cast<char>(r[i] + 32);
	return r;
}

std::string	CgiOutputParser::trim(const std::string& str)
{
	size_t	first = str.find_first_not_of(" \t\r\n");

	if (first == std::string::npos)
		return "";
	size_t	last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, last - first + 1);
}

bool	CgiOutputParser::findHeaderEnd(const std::string& raw, size_t& pos, size_t& sepLen)
{
	size_t	crlf = raw.find("\r\n\r\n");
	size_t	lf = raw.find("\n\n");

	if (crlf == std::string::npos && lf == std::string::npos)
		return false;
	if (lf == std::string::npos || (crlf != std::string::npos && crlf < lf))
	{
		pos = crlf;
		sepLen = 4;
	}
	else
	{
		pos = lf;
		sepLen = 2;
	}
	return true;
}

bool	CgiOutputParser::parseStatusValue(const std::string& value, CgiResult& result)
{
	size_t		space = value.find(' ');
	std::string	number = (space == std::string::npos) ? value : value.substr(0, space);

	if (number.empty() || number.find_first_not_of("0123456789") != std::string::npos)
		return false;
	int	code = std::atoi(number.c_str());
	if (code < 100 || code > 599)
		return false;
	result.statusCode = code;
	result.statusText = (space == std::string::npos) ? "" : trim(value.substr(space + 1));
	return true;
}

bool	CgiOutputParser::parseHeaderLine(const std::string& line, CgiResult& result)
{
	size_t	colon = line.find(':');

	if (colon == std::string::npos || colon == 0)
		return false;
	std::string	name = toLower(trim(line.substr(0, colon)));
	std::string	value = trim(line.substr(colon + 1));

	if (name.empty())
		return false;
	if (name == "status")
		return parseStatusValue(value, result);
	result.headers[name] = value;
	return true;
}

bool	CgiOutputParser::parse(const std::string& raw, CgiResult& result)
{
	size_t	pos = 0;
	size_t	sepLen = 0;

	result = CgiResult();
	if (!findHeaderEnd(raw, pos, sepLen))
		return false;
	std::string	head = raw.substr(0, pos);
	result.body = raw.substr(pos + sepLen);
	result.statusCode = 0;

	size_t	start = 0;
	while (start <= head.size())
	{
		size_t		nl = head.find('\n', start);
		std::string	line = head.substr(start,
			(nl == std::string::npos) ? std::string::npos : nl - start);

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (!line.empty() && !parseHeaderLine(line, result))
			return false;
		if (nl == std::string::npos)
			break;
		start = nl + 1;
	}
	if (result.statusCode == 0)
	{
		if (result.headers.count("location"))
		{
			result.statusCode = 302;
			result.statusText = "Found";
		}
		else
		{
			result.statusCode = 200;
			result.statusText = "OK";
		}
	}

	if (!result.body.empty() && !result.headers.count("content-type"))
		return false;
	return true;
}
