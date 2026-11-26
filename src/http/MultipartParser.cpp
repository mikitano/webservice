#include "http/MultipartParser.hpp"

std::string	MultipartParser::toLower(const std::string& s)
{
	std::string	r(s);

	for (size_t i = 0; i < r.size(); ++i)
		if (r[i] >= 'A' && r[i] <= 'Z')
			r[i] = static_cast<char>(r[i] + 32);
	return r;
}

std::string	MultipartParser::trim(const std::string& s)
{
	size_t	a = 0;
	size_t	b = s.size();

	while (a < b && (s[a] == ' ' || s[a] == '\t'))
		++a;
	while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t'))
		--b;
	return s.substr(a, b - a);
}

bool	MultipartParser::extractBoundary(const std::string& contentType, std::string& boundary)
{
	std::string	lower = toLower(contentType);

	if (lower.compare(0, 19, "multipart/form-data") != 0)
		return false;
	size_t	pos = lower.find("boundary=");

	if (pos == std::string::npos)
		return false;
	std::string	value = contentType.substr(pos + 9);
	size_t		semi = value.find(';');

	if (semi != std::string::npos)
		value.erase(semi);
	value = trim(value);
	if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"')
		value = value.substr(1, value.size() - 2);
	if (value.empty() || value.size() > 200)
		return false;
	boundary = value;
	return true;
}

std::string	MultipartParser::headerParam(const std::string& value, const std::string& key)
{
	std::string	needle = key + "=";
	size_t		pos = 0;

	while ((pos = value.find(needle, pos)) != std::string::npos)
	{
		bool	atStart = (pos == 0) || value[pos - 1] == ' ' || value[pos - 1] == ';';

		if (!atStart)
		{
			pos += needle.size();
			continue;
		}
		size_t	start = pos + needle.size();

		if (start < value.size() && value[start] == '"')
		{
			size_t	end = value.find('"', start + 1);

			if (end == std::string::npos)
				return "";
			return value.substr(start + 1, end - start - 1);
		}
		size_t	end = value.find(';', start);

		return trim(value.substr(start, end == std::string::npos ? std::string::npos : end - start));
	}
	return "";
}

bool	MultipartParser::parseDisposition(const std::string& value, MultipartPart& part)
{
	if (toLower(value).compare(0, 9, "form-data") != 0)
		return false;
	part.name = headerParam(value, "name");
	part.filename = headerParam(value, "filename");
	part.hasFilename = (value.find("filename=") != std::string::npos);
	return true;
}

bool	MultipartParser::parse(const std::string& body, const std::string& boundary,
							   std::vector<MultipartPart>& parts)
{
	const std::string	delim = "--" + boundary;
	const std::string	sep = "\r\n" + delim;
	size_t				pos = body.find(delim);

	parts.clear();
	if (pos == std::string::npos)
		return false;

	if (pos != 0 && (pos < 2 || body.compare(pos - 2, 2, "\r\n") != 0))
		return false;
	pos += delim.size();
	for (;;)
	{
		if (body.compare(pos, 2, "--") == 0)
			return true;
		if (body.compare(pos, 2, "\r\n") != 0)
			return false;
		pos += 2;

		size_t	headEnd = body.find("\r\n\r\n", pos);

		if (headEnd == std::string::npos)
			return false;
		std::string		headers = body.substr(pos, headEnd - pos);
		size_t			dataStart = headEnd + 4;
		MultipartPart	part;
		bool			hasDisposition = false;
		size_t			ls = 0;

		for (;;)
		{
			size_t		le = headers.find("\r\n", ls);
			std::string	line = headers.substr(ls, le == std::string::npos ? std::string::npos : le - ls);
			size_t		colon = line.find(':');

			if (colon == std::string::npos)
				return false;
			std::string	name = toLower(trim(line.substr(0, colon)));
			std::string	value = trim(line.substr(colon + 1));

			if (name == "content-disposition")
			{
				if (!parseDisposition(value, part))
					return false;
				hasDisposition = true;
			}
			else if (name == "content-type")
				part.contentType = value;
			if (le == std::string::npos)
				break;
			ls = le + 2;
		}
		if (!hasDisposition)
			return false;

		size_t	next = body.find(sep, dataStart);

		if (next == std::string::npos)
			return false;
		part.data = body.substr(dataStart, next - dataStart);
		parts.push_back(part);
		pos = next + sep.size();
	}
}
