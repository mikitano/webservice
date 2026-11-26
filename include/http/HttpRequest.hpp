#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <cstddef>
#include <map>
#include <string>

class Config;
class ServerConfig;
class LocationConfig;

class HttpRequest
{
	public:
		enum ParseStatus
		{
			PARSE_INCOMPLETE,
			PARSE_COMPLETE,
			PARSE_ERROR
		};

		explicit HttpRequest(const Config* config = NULL, int port = 0);
		HttpRequest(const HttpRequest& other);
		HttpRequest& operator=(const HttpRequest& other);
		~HttpRequest();

		ParseStatus		feed(const char* data, size_t len);
		std::string		takeLeftover();
		void			reset();

		bool			isComplete() const;
		bool			isHeadersComplete() const;
		bool			hasPartialData() const;
		bool			needsContinue() const;
		void			continueSent();

		const std::string&	getMethod() const;
		const std::string&	getVersion() const;
		const std::string&	getRawUri() const;
		const std::string&	getPath() const;
		const std::string&	getQuery() const;
		const std::string&	getBody() const;

		std::string			getHeader(const std::string& name) const;
		const std::map<std::string, std::string>&	getHeaders() const;

		std::string			getHost() const;
		size_t				getContentLength() const;
		bool				isChunked() const;

		bool				isValid() const;
		int					getErrorStatus() const;
		bool				isKeepAlive() const;

		const ServerConfig*		getServerConfig() const;
		const LocationConfig*	getLocation() const;

	private:
		enum State
		{
			ST_REQUEST_LINE,
			ST_HEADERS,
			ST_BODY_LENGTH,
			ST_CHUNK_SIZE,
			ST_CHUNK_DATA,
			ST_CHUNK_DATA_END,
			ST_TRAILERS,
			ST_DONE,
			ST_ERROR
		};

		void	parse();
		bool	stepRequestLine();
		bool	stepHeaders();
		bool	stepBodyLength();
		bool	stepChunkSize();
		bool	stepChunkData();
		bool	stepChunkDataEnd();
		bool	stepTrailers();
		bool	readLine(std::string& line, size_t maxLen, int errStatus);

		void	handleRequestLine(const std::string& line);
		void	handleHeaderLine(const std::string& line);
		void	finishHeaders();
		void	selectServerAndLocation();
		void	setError(int status);
		void	compact();

		static bool			urlDecode(const std::string& in, std::string& out);
		static bool			normalizePath(const std::string& in, std::string& out);
		static bool			parseDecimal(const std::string& s, size_t& out);
		static bool			parseHex(const std::string& s, size_t& out);
		static bool			isTokenChar(char c);
		static bool			hasToken(const std::string& list, const std::string& token);
		static std::string	extractHostName(const std::string& hostHeader);
		static std::string	trim(const std::string& s);
		static std::string	toLower(const std::string& s);

		const Config*		_config;
		int					_port;

		State				_state;
		int					_errorStatus;
		std::string			_buffer;
		size_t				_bufPos;

		std::string			_method;
		std::string			_version;
		std::string			_rawUri;
		std::string			_path;
		std::string			_query;
		std::map<std::string, std::string>	_headers;
		std::string			_body;

		size_t				_contentLength;
		bool				_chunked;
		size_t				_chunkRemaining;
		size_t				_maxBodySize;

		const ServerConfig*		_server;
		const LocationConfig*	_location;

		size_t				_headerCount;
		size_t				_headersBytes;
		size_t				_trailerLines;
		size_t				_emptyLines;

		bool				_hasData;
		bool				_headersDone;
		bool				_expectContinue;
		bool				_continueSent;
};

#endif
