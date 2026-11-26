#ifndef CGIOUTPUTPARSER_HPP
#define CGIOUTPUTPARSER_HPP

#include <map>
#include <string>

struct CgiResult
{
	int									statusCode;
	std::string							statusText;
	std::map<std::string, std::string>	headers;
	std::string							body;

	CgiResult() : statusCode(200), statusText("OK") {}
};

class CgiOutputParser
{
	public:
		static bool	parse(const std::string& raw, CgiResult& result);

	private:
		CgiOutputParser();

		static bool	findHeaderEnd(const std::string& raw, size_t& pos, size_t& sepLen);
		static bool	parseHeaderLine(const std::string& line, CgiResult& result);
		static bool	parseStatusValue(const std::string& value, CgiResult& result);

		static std::string	trim(const std::string& str);
		static std::string	toLower(const std::string& str);
};

#endif
