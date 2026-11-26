#include "http/HttpRequest.hpp"

#include <algorithm>
#include <vector>

#include "config/model/Config.hpp"
#include "config/model/LocationConfig.hpp"
#include "config/model/ServerConfig.hpp"

static const size_t	MAX_REQUEST_LINE = 8192;
static const size_t	MAX_HEADER_LINE = 8192;
static const size_t	MAX_HEADERS_TOTAL = 32768;
static const size_t	MAX_HEADER_COUNT = 100;
static const size_t	MAX_CHUNK_LINE = 1024;
static const size_t	MAX_TRAILER_LINES = 100;
static const size_t	MAX_LEADING_EMPTY_LINES = 4;
static const size_t	DEFAULT_MAX_BODY = 1048576;

HttpRequest::HttpRequest(const Config* config, int port)
	: _config(config), _port(port)
{
	reset();
}

HttpRequest::HttpRequest(const HttpRequest& other)
	: _config(other._config), _port(other._port)
{
	*this = other;
}

HttpRequest&	HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		_config = other._config;
		_port = other._port;
		_state = other._state;
		_errorStatus = other._errorStatus;
		_buffer = other._buffer;
		_bufPos = other._bufPos;
		_method = other._method;
		_version = other._version;
		_rawUri = other._rawUri;
		_path = other._path;
		_query = other._query;
		_headers = other._headers;
		_body = other._body;
		_contentLength = other._contentLength;
		_chunked = other._chunked;
		_chunkRemaining = other._chunkRemaining;
		_maxBodySize = other._maxBodySize;
		_server = other._server;
		_location = other._location;
		_headerCount = other._headerCount;
		_headersBytes = other._headersBytes;
		_trailerLines = other._trailerLines;
		_emptyLines = other._emptyLines;
		_hasData = other._hasData;
		_headersDone = other._headersDone;
		_expectContinue = other._expectContinue;
		_continueSent = other._continueSent;
	}
	return *this;
}

HttpRequest::~HttpRequest() {}

void	HttpRequest::reset()
{
	_state = ST_REQUEST_LINE;
	_errorStatus = 0;
	_buffer.clear();
	_bufPos = 0;
	_method.clear();
	_version.clear();
	_rawUri.clear();
	_path.clear();
	_query.clear();
	_headers.clear();
	_body.clear();
	_contentLength = 0;
	_chunked = false;
	_chunkRemaining = 0;
	_maxBodySize = DEFAULT_MAX_BODY;
	_server = NULL;
	_location = NULL;
	_headerCount = 0;
	_headersBytes = 0;
	_trailerLines = 0;
	_emptyLines = 0;
	_hasData = false;
	_headersDone = false;
	_expectContinue = false;
	_continueSent = false;
}

HttpRequest::ParseStatus	HttpRequest::feed(const char* data, size_t len)
{
	if (_state == ST_ERROR)
		return PARSE_ERROR;
	if (len > 0)
	{
		_buffer.append(data, len);
		_hasData = true;
	}
	if (_state == ST_DONE)
		return PARSE_COMPLETE;
	parse();
	compact();
	if (_state == ST_ERROR)
		return PARSE_ERROR;
	if (_state == ST_DONE)
		return PARSE_COMPLETE;
	return PARSE_INCOMPLETE;
}

bool	HttpRequest::isComplete() const { return _state == ST_DONE; }
bool	HttpRequest::isHeadersComplete() const { return _headersDone; }

bool	HttpRequest::hasPartialData() const
{
	return _hasData && _state != ST_DONE && _state != ST_ERROR;
}

bool	HttpRequest::needsContinue() const
{
	return _expectContinue && !_continueSent
		&& _state != ST_DONE && _state != ST_ERROR;
}

void	HttpRequest::continueSent() { _continueSent = true; }

std::string	HttpRequest::takeLeftover()
{
	std::string	rest;

	if (_state != ST_DONE)
		return rest;
	compact();
	rest.swap(_buffer);
	return rest;
}

const std::string&	HttpRequest::getMethod() const { return _method; }
const std::string&	HttpRequest::getVersion() const { return _version; }
const std::string&	HttpRequest::getRawUri() const { return _rawUri; }
const std::string&	HttpRequest::getPath() const { return _path; }
const std::string&	HttpRequest::getQuery() const { return _query; }
const std::string&	HttpRequest::getBody() const { return _body; }
const std::map<std::string, std::string>&	HttpRequest::getHeaders() const { return _headers; }

std::string	HttpRequest::getHeader(const std::string& name) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(toLower(name));

	return (it == _headers.end()) ? std::string() : it->second;
}

std::string	HttpRequest::getHost() const
{
	return extractHostName(getHeader("host"));
}

size_t	HttpRequest::getContentLength() const { return _contentLength; }
bool	HttpRequest::isChunked() const { return _chunked; }
bool	HttpRequest::isValid() const { return _state != ST_ERROR; }
int		HttpRequest::getErrorStatus() const { return _errorStatus; }

const ServerConfig*		HttpRequest::getServerConfig() const { return _server; }
const LocationConfig*	HttpRequest::getLocation() const { return _location; }

bool	HttpRequest::isKeepAlive() const
{
	if (_state == ST_ERROR)
		return false;
	std::string	conn = getHeader("connection");
	bool		close = hasToken(conn, "close");
	bool		keep = hasToken(conn, "keep-alive");

	if (_version == "HTTP/1.0")
		return keep && !close;
	return !close;
}

void	HttpRequest::parse()
{
	bool	go = true;

	while (go)
	{
		switch (_state)
		{
			case ST_REQUEST_LINE:		go = stepRequestLine(); break;
			case ST_HEADERS:			go = stepHeaders(); break;
			case ST_BODY_LENGTH:		go = stepBodyLength(); break;
			case ST_CHUNK_SIZE:			go = stepChunkSize(); break;
			case ST_CHUNK_DATA:			go = stepChunkData(); break;
			case ST_CHUNK_DATA_END:		go = stepChunkDataEnd(); break;
			case ST_TRAILERS:			go = stepTrailers(); break;
			case ST_DONE:				go = false; break;
			case ST_ERROR:				go = false; break;
		}
	}
}

void	HttpRequest::setError(int status)
{
	if (_state == ST_ERROR)
		return;
	_state = ST_ERROR;
	_errorStatus = status;
	_buffer.clear();
	_bufPos = 0;
}

void	HttpRequest::compact()
{
	if (_bufPos > 0)
	{
		_buffer.erase(0, _bufPos);
		_bufPos = 0;
	}
}

bool	HttpRequest::readLine(std::string& line, size_t maxLen, int errStatus)
{
	size_t	nl = _buffer.find('\n', _bufPos);

	if (nl == std::string::npos)
	{
		if (_buffer.size() - _bufPos > maxLen)
			setError(errStatus);
		return false;
	}
	size_t	len = nl - _bufPos;
	if (len > 0 && _buffer[nl - 1] == '\r')
		--len;
	if (len > maxLen)
	{
		setError(errStatus);
		return false;
	}
	line = _buffer.substr(_bufPos, len);
	_bufPos = nl + 1;
	return true;
}

bool	HttpRequest::stepRequestLine()
{
	std::string	line;

	if (!readLine(line, MAX_REQUEST_LINE, 414))
		return false;
	if (line.empty())
	{
		if (++_emptyLines > MAX_LEADING_EMPTY_LINES)
		{
			setError(400);
			return false;
		}
		return true;
	}
	handleRequestLine(line);
	return _state != ST_ERROR;
}

void	HttpRequest::handleRequestLine(const std::string& line)
{
	size_t	sp1 = line.find(' ');
	size_t	sp2 = (sp1 == std::string::npos) ? std::string::npos : line.find(' ', sp1 + 1);

	if (sp1 == std::string::npos || sp2 == std::string::npos
		|| sp1 == 0 || sp2 == sp1 + 1 || sp2 + 1 >= line.size()
		|| line.find(' ', sp2 + 1) != std::string::npos)
	{
		setError(400);
		return;
	}
	std::string	method = line.substr(0, sp1);
	std::string	target = line.substr(sp1 + 1, sp2 - sp1 - 1);
	std::string	version = line.substr(sp2 + 1);

	for (size_t i = 0; i < method.size(); ++i)
	{
		if (method[i] < 'A' || method[i] > 'Z')
		{
			setError(400);
			return;
		}
	}

	if (version.size() != 8 || version.compare(0, 5, "HTTP/") != 0
		|| version[5] < '0' || version[5] > '9' || version[6] != '.'
		|| version[7] < '0' || version[7] > '9')
	{
		setError(400);
		return;
	}
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
	{
		setError(505);
		return;
	}

	if (target[0] != '/')
	{
		setError(400);
		return;
	}
	for (size_t i = 0; i < target.size(); ++i)
	{
		unsigned char	c = static_cast<unsigned char>(target[i]);
		if (c < 0x20 || c == 0x7F)
		{
			setError(400);
			return;
		}
	}

	std::string	pathPart = target;
	size_t		hash = pathPart.find('#');
	if (hash != std::string::npos)
		pathPart.erase(hash);
	std::string	query;
	size_t		qm = pathPart.find('?');
	if (qm != std::string::npos)
	{
		query = pathPart.substr(qm + 1);
		pathPart.erase(qm);
	}

	std::string	decoded;
	std::string	normalized;
	if (!urlDecode(pathPart, decoded) || !normalizePath(decoded, normalized))
	{
		setError(400);
		return;
	}
	_method = method;
	_version = version;
	_rawUri = target;
	_path = normalized;
	_query = query;
	_state = ST_HEADERS;
}

bool	HttpRequest::stepHeaders()
{
	std::string	line;

	if (!readLine(line, MAX_HEADER_LINE, 431))
		return false;
	_headersBytes += line.size() + 2;
	if (_headersBytes > MAX_HEADERS_TOTAL)
	{
		setError(431);
		return false;
	}
	if (line.empty())
	{
		finishHeaders();
		return _state != ST_ERROR;
	}
	if (++_headerCount > MAX_HEADER_COUNT)
	{
		setError(431);
		return false;
	}
	handleHeaderLine(line);
	return _state != ST_ERROR;
}

void	HttpRequest::handleHeaderLine(const std::string& line)
{
	if (line[0] == ' ' || line[0] == '\t')
	{
		setError(400);
		return;
	}
	size_t	colon = line.find(':');
	if (colon == std::string::npos || colon == 0)
	{
		setError(400);
		return;
	}
	std::string	name = line.substr(0, colon);
	for (size_t i = 0; i < name.size(); ++i)
	{
		if (!isTokenChar(name[i]))
		{
			setError(400);
			return;
		}
	}
	std::string	value = trim(line.substr(colon + 1));
	for (size_t i = 0; i < value.size(); ++i)
	{
		unsigned char	c = static_cast<unsigned char>(value[i]);
		if ((c < 0x20 && c != '\t') || c == 0x7F)
		{
			setError(400);
			return;
		}
	}
	std::string	key = toLower(name);
	std::map<std::string, std::string>::iterator it = _headers.find(key);
	if (it == _headers.end())
	{
		_headers[key] = value;
		return;
	}
	if (key == "host")
		setError(400);
	else if (key == "content-length")
	{
		if (it->second != value)
			setError(400);
	}
	else if (key == "cookie")
		it->second += "; " + value;
	else
		it->second += ", " + value;
}

void	HttpRequest::finishHeaders()
{
	_headersDone = true;
	if (_version == "HTTP/1.1" && _headers.find("host") == _headers.end())
	{
		setError(400);
		return;
	}
	selectServerAndLocation();

	std::map<std::string, std::string>::const_iterator te = _headers.find("transfer-encoding");
	std::map<std::string, std::string>::const_iterator cl = _headers.find("content-length");

	if (te != _headers.end() && cl != _headers.end())
	{
		setError(400);
		return;
	}
	if (te != _headers.end())
	{
		if (toLower(trim(te->second)) != "chunked")
		{
			setError(501);
			return;
		}
		_chunked = true;
	}
	else if (cl != _headers.end())
	{
		if (!parseDecimal(cl->second, _contentLength))
		{
			setError(400);
			return;
		}
		if (_contentLength > _maxBodySize)
		{
			setError(413);
			return;
		}
	}
	std::map<std::string, std::string>::const_iterator ex = _headers.find("expect");
	if (ex != _headers.end())
	{
		if (toLower(ex->second) != "100-continue")
		{
			setError(417);
			return;
		}
		if (_version == "HTTP/1.1" && (_chunked || _contentLength > 0))
			_expectContinue = true;
	}
	if (_chunked)
		_state = ST_CHUNK_SIZE;
	else if (_contentLength > 0)
		_state = ST_BODY_LENGTH;
	else
		_state = ST_DONE;
}

// Depende do B: Config::findServer(), ServerConfig::matchLocation() e
// LocationConfig::getClientMaxBodySize() (PEDIDO_PARA_B.md)
void	HttpRequest::selectServerAndLocation()
{
	if (_config == NULL)
		return;
	_server = _config->findServer(_port, getHost());
	if (_server == NULL)
		return;
	_location = _server->matchLocation(_path);
	size_t	limit = _server->getClientMaxBodySize();
	if (_location != NULL && _location->getClientMaxBodySize() > 0)
		limit = _location->getClientMaxBodySize();
	if (limit > 0)
		_maxBodySize = limit;
}

bool	HttpRequest::stepBodyLength()
{
	if (_bufPos >= _buffer.size())
		return false;
	size_t	need = _contentLength - _body.size();
	size_t	take = std::min(need, _buffer.size() - _bufPos);

	_body.append(_buffer, _bufPos, take);
	_bufPos += take;
	if (_body.size() < _contentLength)
		return false;
	_state = ST_DONE;
	return true;
}

bool	HttpRequest::stepChunkSize()
{
	std::string	line;

	if (!readLine(line, MAX_CHUNK_LINE, 400))
		return false;
	size_t	semi = line.find(';');
	if (semi != std::string::npos)
		line.erase(semi);
	size_t	size = 0;
	if (!parseHex(trim(line), size))
	{
		setError(400);
		return false;
	}
	if (size == 0)
	{
		_state = ST_TRAILERS;
		return true;
	}

	if (_body.size() > _maxBodySize || size > _maxBodySize - _body.size())
	{
		setError(413);
		return false;
	}
	_chunkRemaining = size;
	_state = ST_CHUNK_DATA;
	return true;
}

bool	HttpRequest::stepChunkData()
{
	if (_bufPos >= _buffer.size())
		return false;
	size_t	take = std::min(_chunkRemaining, _buffer.size() - _bufPos);

	_body.append(_buffer, _bufPos, take);
	_bufPos += take;
	_chunkRemaining -= take;
	if (_chunkRemaining > 0)
		return false;
	_state = ST_CHUNK_DATA_END;
	return true;
}

bool	HttpRequest::stepChunkDataEnd()
{
	std::string	line;

	if (_bufPos < _buffer.size() && _buffer[_bufPos] != '\r' && _buffer[_bufPos] != '\n')
	{
		setError(400);
		return false;
	}
	if (!readLine(line, 16, 400))
		return false;
	if (!line.empty())
	{
		setError(400);
		return false;
	}
	_state = ST_CHUNK_SIZE;
	return true;
}

bool	HttpRequest::stepTrailers()
{
	std::string	line;

	if (!readLine(line, MAX_HEADER_LINE, 431))
		return false;
	if (line.empty())
	{
		_state = ST_DONE;
		return true;
	}
	if (++_trailerLines > MAX_TRAILER_LINES)
	{
		setError(431);
		return false;
	}
	return true;
}

static int	hexValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

bool	HttpRequest::urlDecode(const std::string& in, std::string& out)
{
	out.clear();
	for (size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] != '%')
		{
			out += in[i];
			continue;
		}
		if (i + 2 >= in.size())
			return false;
		int	hi = hexValue(in[i + 1]);
		int	lo = hexValue(in[i + 2]);
		if (hi < 0 || lo < 0)
			return false;
		int	value = hi * 16 + lo;
		if (value < 0x20 || value == 0x7F)
			return false;
		out += static_cast<char>(value);
		i += 2;
	}
	return true;
}

bool	HttpRequest::normalizePath(const std::string& in, std::string& out)
{
	std::vector<std::string>	stack;
	bool						trailing = false;

	if (in.empty() || in[0] != '/')
		return false;
	if (in[in.size() - 1] == '/')
		trailing = true;
	else if (in.size() >= 2 && in.compare(in.size() - 2, 2, "/.") == 0)
		trailing = true;
	else if (in.size() >= 3 && in.compare(in.size() - 3, 3, "/..") == 0)
		trailing = true;

	size_t	pos = 1;
	while (pos <= in.size())
	{
		size_t		slash = in.find('/', pos);
		size_t		end = (slash == std::string::npos) ? in.size() : slash;
		std::string	seg = in.substr(pos, end - pos);

		if (seg == "..")
		{
			if (stack.empty())
				return false;
			stack.pop_back();
		}
		else if (!seg.empty() && seg != ".")
			stack.push_back(seg);
		if (slash == std::string::npos)
			break;
		pos = slash + 1;
	}
	out = "/";
	for (size_t i = 0; i < stack.size(); ++i)
	{
		out += stack[i];
		if (i + 1 < stack.size())
			out += "/";
	}
	if (trailing && !stack.empty())
		out += "/";
	return true;
}

bool	HttpRequest::parseDecimal(const std::string& s, size_t& out)
{
	const size_t	maxVal = static_cast<size_t>(-1);

	if (s.empty())
		return false;
	out = 0;
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] < '0' || s[i] > '9')
			return false;
		size_t	digit = static_cast<size_t>(s[i] - '0');
		if (out > (maxVal - digit) / 10)
			return false;
		out = out * 10 + digit;
	}
	return true;
}

bool	HttpRequest::parseHex(const std::string& s, size_t& out)
{
	const size_t	maxVal = static_cast<size_t>(-1);

	if (s.empty())
		return false;
	out = 0;
	for (size_t i = 0; i < s.size(); ++i)
	{
		int	v = hexValue(s[i]);
		if (v < 0)
			return false;
		if (out > (maxVal - static_cast<size_t>(v)) / 16)
			return false;
		out = out * 16 + static_cast<size_t>(v);
	}
	return true;
}

bool	HttpRequest::isTokenChar(char c)
{
	if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return true;
	switch (c)
	{
		case '!': case '#': case '$': case '%': case '&': case '\'':
		case '*': case '+': case '-': case '.': case '^': case '_':
		case '`': case '|': case '~':
			return true;
		default:
			return false;
	}
}

bool	HttpRequest::hasToken(const std::string& list, const std::string& token)
{
	size_t	pos = 0;

	while (pos <= list.size())
	{
		size_t	comma = list.find(',', pos);
		size_t	end = (comma == std::string::npos) ? list.size() : comma;

		if (toLower(trim(list.substr(pos, end - pos))) == token)
			return true;
		if (comma == std::string::npos)
			break;
		pos = comma + 1;
	}
	return false;
}

std::string	HttpRequest::extractHostName(const std::string& hostHeader)
{
	std::string	host = toLower(trim(hostHeader));

	if (!host.empty() && host[0] == '[')
	{
		size_t	close = host.find(']');
		return (close == std::string::npos) ? host : host.substr(0, close + 1);
	}
	size_t	colon = host.rfind(':');
	if (colon != std::string::npos)
		host.erase(colon);
	return host;
}

std::string	HttpRequest::trim(const std::string& s)
{
	size_t	a = 0;
	size_t	b = s.size();

	while (a < b && (s[a] == ' ' || s[a] == '\t'))
		++a;
	while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t'))
		--b;
	return s.substr(a, b - a);
}

std::string	HttpRequest::toLower(const std::string& s)
{
	std::string	r(s);

	for (size_t i = 0; i < r.size(); ++i)
		if (r[i] >= 'A' && r[i] <= 'Z')
			r[i] = static_cast<char>(r[i] + 32);
	return r;
}
