#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <ctime>
#include <string>

#include "cgi/CgiOutputParser.hpp"
#include "config/model/LocationConfig.hpp"
#include "config/model/ServerConfig.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "network/ClientInfo.hpp"

class CgiHandler;

class RequestHandler
{
	public:
		explicit RequestHandler(time_t cgiTimeout = 5);
		RequestHandler(const RequestHandler& other);
		RequestHandler& operator=(const RequestHandler& other);
		~RequestHandler();

		bool			handle(const HttpRequest& request, const ClientInfo& client,
							   HttpResponse& response, CgiHandler*& cgi);

		HttpResponse	errorResponse(int status, const ServerConfig* server) const;
		HttpResponse	cgiResponse(const CgiResult& result, const ServerConfig* server) const;

	private:
		HttpResponse	handleGet(const HttpRequest& req, const LocationConfig& loc);
		HttpResponse	handlePost(const HttpRequest& req, const LocationConfig& loc);
		HttpResponse	handleDelete(const HttpRequest& req, const LocationConfig& loc);
		bool			handleCgi(const HttpRequest& req, const ClientInfo& client,
								  const LocationConfig& loc, HttpResponse& response,
								  CgiHandler*& cgi);

		HttpResponse	serveFile(const std::string& fsPath, size_t size, time_t mtime,
								  const ServerConfig* server, bool headOnly) const;
		HttpResponse	serveDirectory(const HttpRequest& req, const LocationConfig& loc,
									   const std::string& fsPath, bool headOnly) const;
		HttpResponse	autoIndex(const std::string& dirPath, const std::string& urlPath,
								  const ServerConfig* server) const;

		std::string		saveFile(const std::string& uploadDir, const std::string& name,
								 const std::string& data);

		HttpResponse	buildError(int status, const ServerConfig* server) const;
		HttpResponse	buildRedirect(const LocationConfig& loc, const ServerConfig* server) const;
		HttpResponse	methodNotAllowed(const LocationConfig& loc, const ServerConfig* server) const;

		static bool			removeFile(const std::string& path);
		static std::string	joinPath(const std::string& root, const std::string& urlPath);
		static std::string	sanitizeFilename(const std::string& raw);
		static std::string	generatedName();
		static std::string	urlEncode(const std::string& s);
		static std::string	htmlEscape(const std::string& s);
		static std::string	httpDate(time_t t);
		static bool			readFile(const std::string& path, std::string& out);
		static std::string	lastSegment(const std::string& path);

		time_t	_cgiTimeout;
};

#endif
