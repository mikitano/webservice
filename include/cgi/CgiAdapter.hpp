#ifndef CGIADAPTER_HPP
#define CGIADAPTER_HPP

#include <map>
#include <string>

#include "cgi/CgiHandler.hpp"
#include "config/model/LocationConfig.hpp"
#include "config/model/ServerConfig.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "network/ClientInfo.hpp"

class CgiAdapter
{
	public:
		static std::string	findCgiExtension(const std::string& urlPath,
											const std::map<std::string, std::string>& cgi);

		static bool			isCgiRequest(const HttpRequest& req, const LocationConfig& loc);

		static bool			buildParams(const HttpRequest& req,
										const ServerConfig& server,
										const LocationConfig& loc,
										const ClientInfo& client,
										CgiParams& out,
										int& errorStatus);

		static HttpResponse	toHttpResponse(const CgiResult& result);

	private:
		CgiAdapter();
};

#endif
