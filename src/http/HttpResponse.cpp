#include "http/HttpResponse.hpp"

#include <ctime>
#include <sstream>

HttpResponse::HttpResponse()
	: _status(200), _statusText("OK"), _headers(), _body(), _headOnly(false)
{
}

HttpResponse::HttpResponse(const HttpResponse& other)
	: _status(other._status), _statusText(other._statusText),
	  _headers(other._headers), _body(other._body), _headOnly(other._headOnly)
{
}

HttpResponse&	HttpResponse::operator=(const HttpResponse& other)
{
	if (this != &other)
	{
		_status = other._status;
		_statusText = other._statusText;
		_headers = other._headers;
		_body = other._body;
		_headOnly = other._headOnly;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}

void	HttpResponse::setStatus(int code)
{
	setStatus(code, "");
}

void	HttpResponse::setStatus(int code, const std::string& text)
{
	_status = code;
	_statusText = text.empty() ? reasonPhrase(code) : text;
}

void	HttpResponse::setHeader(const std::string& name, const std::string& value)
{
	std::string	key = toLower(name);

	for (size_t i = 0; i < _headers.size(); ++i)
	{
		if (toLower(_headers[i].first) == key)
		{
			_headers[i].second = value;
			return;
		}
	}
	_headers.push_back(std::make_pair(name, value));
}

void	HttpResponse::setBody(const std::string& body) { _body = body; }
void	HttpResponse::setHeadOnly(bool headOnly) { _headOnly = headOnly; }

int					HttpResponse::getStatus() const { return _status; }
const std::string&	HttpResponse::getStatusText() const { return _statusText; }
const std::string&	HttpResponse::getBody() const { return _body; }
bool				HttpResponse::isHeadOnly() const { return _headOnly; }

bool	HttpResponse::hasHeader(const std::string& name) const
{
	std::string	key = toLower(name);

	for (size_t i = 0; i < _headers.size(); ++i)
		if (toLower(_headers[i].first) == key)
			return true;
	return false;
}

std::string	HttpResponse::getHeader(const std::string& name) const
{
	std::string	key = toLower(name);

	for (size_t i = 0; i < _headers.size(); ++i)
		if (toLower(_headers[i].first) == key)
			return _headers[i].second;
	return "";
}

std::string	HttpResponse::toString() const
{
	std::string	out = "HTTP/1.1 " + numToString(static_cast<size_t>(_status))
		+ " " + _statusText + "\r\n";

	bool	noBody = (_status >= 100 && _status < 200) || _status == 204 || _status == 304;

	for (size_t i = 0; i < _headers.size(); ++i)
	{
		if (toLower(_headers[i].first) == "content-length")
			continue;
		out += _headers[i].first + ": " + _headers[i].second + "\r\n";
	}
	if (!hasHeader("Server"))
		out += "Server: webserv/1.0\r\n";
	if (!hasHeader("Date"))
		out += "Date: " + httpDate() + "\r\n";
	if (!noBody)
	{
		std::string	length = (_headOnly && hasHeader("Content-Length"))
			? getHeader("Content-Length") : numToString(_body.size());
		out += "Content-Length: " + length + "\r\n";
	}
	out += "\r\n";
	if (!noBody && !_headOnly)
		out += _body;
	return out;
}

std::string	HttpResponse::reasonPhrase(int code)
{
	switch (code)
	{
		case 100: return "Continue";
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 204: return "No Content";
		case 206: return "Partial Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 417: return "Expectation Failed";
		case 431: return "Request Header Fields Too Large";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		default:  return "Unknown";
	}
}

HttpResponse	HttpResponse::makeError(int code)
{
	std::string	title = numToString(static_cast<size_t>(code)) + " " + reasonPhrase(code);
	std::string	body = "<html>\n<head><title>" + title + "</title></head>\n"
		"<body>\n<center><h1>" + title + "</h1></center>\n"
		"<hr><center>webserv</center>\n</body>\n</html>\n";
	HttpResponse	res;

	res.setStatus(code);
	res.setHeader("Content-Type", "text/html");
	res.setBody(body);
	return res;
}

HttpResponse	HttpResponse::makeRedirect(int code, const std::string& location)
{
	std::string	title = numToString(static_cast<size_t>(code)) + " " + reasonPhrase(code);
	std::string	body = "<html>\n<head><title>" + title + "</title></head>\n"
		"<body>\n<center><h1>" + title + "</h1></center>\n"
		"<hr><center>webserv</center>\n</body>\n</html>\n";
	HttpResponse	res;

	res.setStatus(code);
	res.setHeader("Location", location);
	res.setHeader("Content-Type", "text/html");
	res.setBody(body);
	return res;
}

std::string	HttpResponse::toLower(const std::string& s)
{
	std::string	r(s);

	for (size_t i = 0; i < r.size(); ++i)
		if (r[i] >= 'A' && r[i] <= 'Z')
			r[i] = static_cast<char>(r[i] + 32);
	return r;
}

std::string	HttpResponse::numToString(size_t n)
{
	std::ostringstream	oss;

	oss << n;
	return oss.str();
}

std::string	HttpResponse::httpDate()
{
	char		buf[64];
	time_t		now = time(NULL);
	struct tm*	gmt = gmtime(&now);

	if (gmt == NULL || strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt) == 0)
		return "Thu, 01 Jan 1970 00:00:00 GMT";
	return buf;
}
