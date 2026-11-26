#include "cgi/CgiAdapter.hpp"
#include "cgi/PathResolver.hpp"

#include <unistd.h>

static std::string	capitalizeHeader(const std::string& name)
{
	std::string	out(name);
	bool		upper = true;

	for (size_t i = 0; i < out.size(); ++i)
	{
		if (upper && out[i] >= 'a' && out[i] <= 'z')
			out[i] = static_cast<char>(out[i] - 32);
		upper = (out[i] == '-');
	}
	return out;
}

std::string	CgiAdapter::findCgiExtension(const std::string& urlPath,
										const std::map<std::string, std::string>& cgi)
{
	for (std::map<std::string, std::string>::const_iterator it = cgi.begin();
		it != cgi.end(); ++it)
	{
		const std::string&	ext = it->first;
		if (ext.empty())
			continue;
		size_t	pos = urlPath.find(ext);
		while (pos != std::string::npos)
		{
			size_t	end = pos + ext.size();

			if (end == urlPath.size() || urlPath[end] == '/')
				return ext;
			pos = urlPath.find(ext, pos + 1);
		}
	}
	return "";
}

bool	CgiAdapter::isCgiRequest(const HttpRequest& req, const LocationConfig& loc)
{
	return !findCgiExtension(req.getPath(), loc.getCgi()).empty();
}

bool	CgiAdapter::buildParams(const HttpRequest& req,
								const ServerConfig& server,
								const LocationConfig& loc,
								const ClientInfo& client,
								CgiParams& out,
								int& errorStatus)
{
	std::string	ext = findCgiExtension(req.getPath(), loc.getCgi());
	if (ext.empty())
	{
		errorStatus = 500;
		return false;
	}
	std::map<std::string, std::string>::const_iterator it = loc.getCgi().find(ext);

	if (access(it->second.c_str(), X_OK) != 0)
	{
		errorStatus = 500;
		return false;
	}

	PathResolution	r = PathResolver::resolve(req.getPath(), loc.getRoot(), ext);
	if (r.statusCode != 200)
	{
		errorStatus = r.statusCode;
		return false;
	}

	out.interpreter    = it->second;
	out.scriptPath     = r.scriptPath;
	out.scriptFile     = r.scriptFile;
	out.scriptDir      = r.scriptDir;
	out.scriptName     = r.scriptName;
	out.pathInfo       = r.pathInfo.empty() ? req.getPath() : r.pathInfo;
	out.pathTranslated = loc.getRoot() + out.pathInfo;
	out.method         = req.getMethod();
	out.queryString    = req.getQuery();
	out.requestUri     = req.getRawUri();
	out.contentType    = req.getHeader("Content-Type");
	out.body           = req.getBody();
	out.headers        = req.getHeaders();
	out.serverName     = server.getServerNames().empty()
		? req.getHost() : server.getServerNames()[0];
	out.serverPort     = server.getPort();
	out.remoteAddr     = client.getRemoteAddr();
	return true;
}

HttpResponse	CgiAdapter::toHttpResponse(const CgiResult& result)
{
	if (result.statusCode >= 400 && result.body.empty())
		return HttpResponse::makeError(result.statusCode);

	HttpResponse	res;
	res.setStatus(result.statusCode, result.statusText);

	for (std::map<std::string, std::string>::const_iterator it = result.headers.begin();
		it != result.headers.end(); ++it)
	{
		if (it->first == "status" || it->first == "content-length")
			continue;
		res.setHeader(capitalizeHeader(it->first), it->second);
	}
	res.setBody(result.body);
	return res;
}
