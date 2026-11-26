#include "http/RequestHandler.hpp"

#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "cgi/CgiAdapter.hpp"
#include "cgi/CgiHandler.hpp"
#include "http/MimeTypes.hpp"
#include "http/MultipartParser.hpp"

RequestHandler::RequestHandler(time_t cgiTimeout) : _cgiTimeout(cgiTimeout) {}

RequestHandler::RequestHandler(const RequestHandler& other) : _cgiTimeout(other._cgiTimeout) {}

RequestHandler&	RequestHandler::operator=(const RequestHandler& other)
{
	if (this != &other)
		_cgiTimeout = other._cgiTimeout;
	return *this;
}

RequestHandler::~RequestHandler() {}

static std::string	numToString(size_t n)
{
	std::ostringstream	oss;

	oss << n;
	return oss.str();
}

static bool	isDirectory(const std::string& path)
{
	struct stat	st;

	return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static bool	isRegularFile(const std::string& path)
{
	struct stat	st;

	return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

// REVISAR: std::remove não está na lista de funções do subject (unlink também não),
// mas o DELETE é obrigatório. Todo apagar de arquivo passa por aqui.
bool	RequestHandler::removeFile(const std::string& path)
{
	return std::remove(path.c_str()) == 0;
}

std::string	RequestHandler::joinPath(const std::string& root, const std::string& urlPath)
{
	std::string	r(root);

	while (r.size() > 1 && r[r.size() - 1] == '/')
		r.erase(r.size() - 1);
	if (urlPath.empty())
		return r;
	if (urlPath[0] != '/')
		return r + "/" + urlPath;
	if (r == "/")
		return urlPath;
	return r + urlPath;
}

std::string	RequestHandler::lastSegment(const std::string& path)
{
	std::string	p(path);

	while (!p.empty() && p[p.size() - 1] == '/')
		p.erase(p.size() - 1);
	size_t	slash = p.rfind('/');

	return (slash == std::string::npos) ? p : p.substr(slash + 1);
}

std::string	RequestHandler::urlEncode(const std::string& s)
{
	static const char	hex[] = "0123456789ABCDEF";
	std::string			out;

	for (size_t i = 0; i < s.size(); ++i)
	{
		unsigned char	c = static_cast<unsigned char>(s[i]);

		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
			|| c == '-' || c == '.' || c == '_' || c == '~' || c == '/')
			out += static_cast<char>(c);
		else
		{
			out += '%';
			out += hex[c >> 4];
			out += hex[c & 15];
		}
	}
	return out;
}

std::string	RequestHandler::htmlEscape(const std::string& s)
{
	std::string	out;

	for (size_t i = 0; i < s.size(); ++i)
	{
		switch (s[i])
		{
			case '&': out += "&amp;"; break;
			case '<': out += "&lt;"; break;
			case '>': out += "&gt;"; break;
			case '"': out += "&quot;"; break;
			default: out += s[i];
		}
	}
	return out;
}

std::string	RequestHandler::httpDate(time_t t)
{
	char		buf[64];
	struct tm*	gmt = gmtime(&t);

	if (gmt == NULL || strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt) == 0)
		return "Thu, 01 Jan 1970 00:00:00 GMT";
	return buf;
}

bool	RequestHandler::readFile(const std::string& path, std::string& out)
{
	std::ifstream	file(path.c_str(), std::ios::in | std::ios::binary);

	if (!file.is_open())
		return false;
	std::ostringstream	buffer;

	buffer << file.rdbuf();
	out = buffer.str();
	return true;
}

HttpResponse	RequestHandler::errorResponse(int status, const ServerConfig* server) const
{
	return buildError(status, server);
}

HttpResponse	RequestHandler::cgiResponse(const CgiResult& result, const ServerConfig* server) const
{
	if (result.statusCode >= 400 && result.body.empty())
		return buildError(result.statusCode, server);
	return CgiAdapter::toHttpResponse(result);
}

bool	RequestHandler::handle(const HttpRequest& req, const ClientInfo& client,
							   HttpResponse& response, CgiHandler*& cgi)
{
	const ServerConfig*		server = req.getServerConfig();
	const LocationConfig*	loc = req.getLocation();
	const std::string&		method = req.getMethod();

	cgi = NULL;

	if (server == NULL || loc == NULL)
	{
		response = buildError(404, server);
		return true;
	}

	if (loc->hasReturn())
	{
		response = buildRedirect(*loc, server);
		return true;
	}

	if (loc->getRoot().empty())
	{
		response = buildError(500, server);
		return true;
	}

	if (!loc->isMethodAllowed(method))
	{
		response = methodNotAllowed(*loc, server);
		return true;
	}

	if (CgiAdapter::isCgiRequest(req, *loc))
		return handleCgi(req, client, *loc, response, cgi);

	if (method == "GET" || method == "HEAD")
		response = handleGet(req, *loc);
	else if (method == "POST")
		response = handlePost(req, *loc);
	else if (method == "DELETE")
		response = handleDelete(req, *loc);
	else
		response = buildError(501, server);
	if (method == "HEAD")
		response.setHeadOnly(true);
	return true;
}

HttpResponse	RequestHandler::buildError(int status, const ServerConfig* server) const
{
	if (server != NULL)
	{
		std::string				page = server->getErrorPage(status);
		const LocationConfig*	pageLoc = page.empty() ? NULL : server->matchLocation(page);

		if (pageLoc != NULL && !pageLoc->getRoot().empty())
		{
			std::string	fsPath = joinPath(pageLoc->getRoot(), page);
			std::string	content;

			if (isRegularFile(fsPath) && readFile(fsPath, content))
			{
				HttpResponse	res;

				res.setStatus(status);
				res.setHeader("Content-Type", MimeTypes::fromPath(fsPath));
				res.setBody(content);
				return res;
			}
		}
	}
	return HttpResponse::makeError(status);
}

HttpResponse	RequestHandler::buildRedirect(const LocationConfig& loc, const ServerConfig* server) const
{
	int	code = loc.getReturnCode();

	if (code >= 300 && code < 400)
		return HttpResponse::makeRedirect(code, loc.getReturnUrl());
	return buildError(code, server);
}

HttpResponse	RequestHandler::methodNotAllowed(const LocationConfig& loc, const ServerConfig* server) const
{
	const std::vector<std::string>&	methods = loc.getAllowedMethods();
	std::string						allow;

	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (!allow.empty())
			allow += ", ";
		allow += methods[i];
	}
	HttpResponse	res = buildError(405, server);

	res.setHeader("Allow", allow);
	return res;
}

HttpResponse	RequestHandler::handleGet(const HttpRequest& req, const LocationConfig& loc)
{
	const ServerConfig*	server = req.getServerConfig();
	std::string			fsPath = joinPath(loc.getRoot(), req.getPath());
	struct stat			st;
	bool				headOnly = (req.getMethod() == "HEAD");

	if (stat(fsPath.c_str(), &st) != 0)
		return buildError(404, server);
	if (S_ISDIR(st.st_mode))
		return serveDirectory(req, loc, fsPath, headOnly);
	if (S_ISREG(st.st_mode))
		return serveFile(fsPath, static_cast<size_t>(st.st_size), st.st_mtime, server, headOnly);
	return buildError(403, server);
}

HttpResponse	RequestHandler::serveFile(const std::string& fsPath, size_t size, time_t mtime,
										  const ServerConfig* server, bool headOnly) const
{
	if (access(fsPath.c_str(), R_OK) != 0)
		return buildError(403, server);
	HttpResponse	res;

	res.setStatus(200);
	res.setHeader("Content-Type", MimeTypes::fromPath(fsPath));
	res.setHeader("Last-Modified", httpDate(mtime));
	if (headOnly)
	{
		res.setHeader("Content-Length", numToString(size));
		return res;
	}
	std::string	content;

	if (!readFile(fsPath, content))
		return buildError(403, server);
	res.setBody(content);
	return res;
}

HttpResponse	RequestHandler::serveDirectory(const HttpRequest& req, const LocationConfig& loc,
											   const std::string& fsPath, bool headOnly) const
{
	const ServerConfig*	server = req.getServerConfig();
	const std::string&	urlPath = req.getPath();

	if (urlPath.empty() || urlPath[urlPath.size() - 1] != '/')
	{
		std::string	target = urlEncode(urlPath) + "/";

		if (!req.getQuery().empty())
			target += "?" + req.getQuery();
		return HttpResponse::makeRedirect(301, target);
	}
	const std::vector<std::string>&	index = loc.getIndex();

	for (size_t i = 0; i < index.size(); ++i)
	{
		std::string	indexPath = joinPath(fsPath, "/" + index[i]);
		struct stat	st;

		if (!index[i].empty() && stat(indexPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
			return serveFile(indexPath, static_cast<size_t>(st.st_size), st.st_mtime, server, headOnly);
	}
	if (loc.getAutoindex())
		return autoIndex(fsPath, urlPath, server);
	return buildError(404, server);
}

HttpResponse	RequestHandler::autoIndex(const std::string& dirPath, const std::string& urlPath,
										  const ServerConfig* server) const
{
	DIR*	dir = opendir(dirPath.c_str());

	if (dir == NULL)
		return buildError(403, server);
	std::vector<std::string>	names;
	struct dirent*				entry;

	while ((entry = readdir(dir)) != NULL)
	{
		std::string	name(entry->d_name);

		if (name != "." && name != "..")
			names.push_back(name);
	}
	closedir(dir);
	std::sort(names.begin(), names.end());

	std::string	title = "Index of " + htmlEscape(urlPath);
	std::string	body = "<html>\n<head><title>" + title + "</title></head>\n"
		"<body>\n<h1>" + title + "</h1><hr>\n<pre>\n";

	if (urlPath != "/")
		body += "<a href=\"../\">../</a>\n";
	for (size_t i = 0; i < names.size(); ++i)
	{
		struct stat	st;
		std::string	full = joinPath(dirPath, "/" + names[i]);
		bool		isDir = false;
		std::string	info;

		if (stat(full.c_str(), &st) == 0)
		{
			isDir = S_ISDIR(st.st_mode);
			char		date[32];
			struct tm*	tmv = localtime(&st.st_mtime);

			if (tmv != NULL && strftime(date, sizeof(date), "%d-%b-%Y %H:%M", tmv) > 0)
				info = std::string(date) + "    " + (isDir ? "-" : numToString(static_cast<size_t>(st.st_size)));
		}
		std::string	shown = names[i] + (isDir ? "/" : "");

		body += "<a href=\"" + urlEncode(shown) + "\">" + htmlEscape(shown) + "</a>";
		if (!info.empty())
			body += "    " + info;
		body += "\n";
	}
	body += "</pre><hr>\n</body>\n</html>\n";
	HttpResponse	res;

	res.setStatus(200);
	res.setHeader("Content-Type", "text/html; charset=utf-8");
	res.setBody(body);
	return res;
}

std::string	RequestHandler::sanitizeFilename(const std::string& raw)
{
	std::string	name = raw;
	size_t		slash = name.find_last_of("/\\");

	if (slash != std::string::npos)
		name = name.substr(slash + 1);
	std::string	clean;

	for (size_t i = 0; i < name.size(); ++i)
	{
		char	c = name[i];

		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
			|| c == '.' || c == '-' || c == '_')
			clean += c;
		else
			clean += '_';
	}
	size_t	start = clean.find_first_not_of('.');

	clean = (start == std::string::npos) ? "" : clean.substr(start);
	if (clean.size() > 100)
		clean.erase(100);
	return clean;
}

std::string	RequestHandler::generatedName()
{
	static size_t	counter = 0;

	return "upload_" + numToString(static_cast<size_t>(time(NULL))) + "_" + numToString(++counter);
}

std::string	RequestHandler::saveFile(const std::string& uploadDir, const std::string& name,
									 const std::string& data)
{
	std::string	base = name;
	std::string	ext;
	size_t		dot = name.rfind('.');

	if (dot != std::string::npos && dot > 0)
	{
		base = name.substr(0, dot);
		ext = name.substr(dot);
	}
	std::string	finalName = name;

	for (size_t n = 1; n < 1000 && access(joinPath(uploadDir, "/" + finalName).c_str(), F_OK) == 0; ++n)
		finalName = base + "_" + numToString(n) + ext;
	std::string		path = joinPath(uploadDir, "/" + finalName);
	std::ofstream	out(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

	if (!out.is_open())
		return "";
	out.write(data.data(), static_cast<std::streamsize>(data.size()));
	out.close();
	if (out.fail())
	{
		removeFile(path);
		return "";
	}
	return finalName;
}

HttpResponse	RequestHandler::handlePost(const HttpRequest& req, const LocationConfig& loc)
{
	const ServerConfig*	server = req.getServerConfig();
	const std::string&	store = loc.getUploadStore();

	if (store.empty())
	{
		HttpResponse	res;

		res.setStatus(200);
		res.setHeader("Content-Type", "text/plain; charset=utf-8");
		res.setBody("Recebido: " + numToString(req.getBody().size()) + " bytes\n");
		return res;
	}
	if (!isDirectory(store) || access(store.c_str(), W_OK | X_OK) != 0)
		return buildError(500, server);

	std::vector<std::string>	saved;
	std::string					boundary;

	if (MultipartParser::extractBoundary(req.getHeader("Content-Type"), boundary))
	{
		std::vector<MultipartPart>	parts;

		if (!MultipartParser::parse(req.getBody(), boundary, parts))
			return buildError(400, server);
		for (size_t i = 0; i < parts.size(); ++i)
		{
			if (!parts[i].hasFilename || parts[i].filename.empty())
				continue;
			std::string	name = sanitizeFilename(parts[i].filename);

			if (name.empty())
				name = generatedName();
			name = saveFile(store, name, parts[i].data);
			if (name.empty())
				return buildError(500, server);
			saved.push_back(name);
		}
	}
	else
	{
		if (req.getBody().empty())
			return buildError(400, server);
		std::string	relative = req.getPath();
		std::string	prefix = loc.getPath();

		if (prefix.size() > 1 && relative.compare(0, prefix.size(), prefix) == 0)
			relative = relative.substr(prefix.size());
		std::string	name = sanitizeFilename(lastSegment(relative));

		if (name.empty())
			name = generatedName();
		name = saveFile(store, name, req.getBody());
		if (name.empty())
			return buildError(500, server);
		saved.push_back(name);
	}
	if (saved.empty())
		return buildError(400, server);

	std::string	prefix = loc.getPath();
	std::string	listing = "Salvo(s):\n";

	for (size_t i = 0; i < saved.size(); ++i)
		listing += saved[i] + "\n";
	HttpResponse	res;

	res.setStatus(201);
	res.setHeader("Content-Type", "text/plain; charset=utf-8");
	res.setHeader("Location", joinPath(prefix.empty() ? "/" : prefix, "/" + urlEncode(saved[0])));
	res.setBody(listing);
	return res;
}

HttpResponse	RequestHandler::handleDelete(const HttpRequest& req, const LocationConfig& loc)
{
	const ServerConfig*	server = req.getServerConfig();
	std::string			fsPath = joinPath(loc.getRoot(), req.getPath());
	struct stat			st;

	if (stat(fsPath.c_str(), &st) != 0)
		return buildError(404, server);
	if (!S_ISREG(st.st_mode))
		return buildError(403, server);
	if (!removeFile(fsPath))
		return buildError(403, server);
	HttpResponse	res;

	res.setStatus(204);
	return res;
}

bool	RequestHandler::handleCgi(const HttpRequest& req, const ClientInfo& client,
								  const LocationConfig& loc, HttpResponse& response,
								  CgiHandler*& cgi)
{
	CgiParams	params;
	int			error = 0;

	if (!CgiAdapter::buildParams(req, *req.getServerConfig(), loc, client, params, error))
	{
		response = buildError(error, req.getServerConfig());
		return true;
	}
	CgiHandler*	handler = new CgiHandler(params, _cgiTimeout);

	if (!handler->start())
	{
		delete handler;
		response = buildError(500, req.getServerConfig());
		return true;
	}
	cgi = handler;
	return false;
}
