#ifndef MULTIPARTPARSER_HPP
#define MULTIPARTPARSER_HPP

#include <string>
#include <vector>

struct MultipartPart
{
	std::string	name;
	std::string	filename;
	std::string	contentType;
	std::string	data;
	bool		hasFilename;

	MultipartPart() : hasFilename(false) {}
};

class MultipartParser
{
	public:
		static bool	extractBoundary(const std::string& contentType, std::string& boundary);
		static bool	parse(const std::string& body, const std::string& boundary,
						  std::vector<MultipartPart>& parts);

	private:
		MultipartParser();

		static bool			parseDisposition(const std::string& value, MultipartPart& part);
		static std::string	headerParam(const std::string& value, const std::string& key);
		static std::string	toLower(const std::string& s);
		static std::string	trim(const std::string& s);
};

#endif
