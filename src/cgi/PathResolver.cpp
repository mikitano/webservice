#include "cgi/PathResolver.hpp"

#include <sys/stat.h>
#include <unistd.h>

bool	PathResolver::endsWith(const std::string& str, const std::string& suffix)
{
	if (suffix.size() > str.size())
		return false;
	return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool	PathResolver::hasDotDotSegment(const std::string& path)
{
	size_t	start = 0;

	while (start <= path.size())
	{
		size_t	slash = path.find('/', start);
		size_t	end = (slash == std::string::npos) ? path.size() : slash;

		if (path.compare(start, end - start, "..") == 0 && end - start == 2)
			return true;
		if (slash == std::string::npos)
			break;
		start = slash + 1;
	}
	return false;
}

PathResolution	PathResolver::resolve(const std::string& urlPath,
									  const std::string& root,
									  const std::string& cgiExtension)
{
	PathResolution	r;

	if (urlPath.empty() || urlPath[0] != '/'
		|| urlPath.find('\0') != std::string::npos)
	{
		r.statusCode = 400;
		return r;
	}
	if (hasDotDotSegment(urlPath))
	{
		r.statusCode = 403;
		return r;
	}

	size_t	scriptEnd = std::string::npos;
	size_t	start = 1;
	while (start <= urlPath.size())
	{
		size_t		slash = urlPath.find('/', start);
		size_t		end = (slash == std::string::npos) ? urlPath.size() : slash;
		std::string	segment = urlPath.substr(start, end - start);

		if (segment.size() > cgiExtension.size() && endsWith(segment, cgiExtension))
		{
			scriptEnd = end;
			break;
		}
		if (slash == std::string::npos)
			break;
		start = slash + 1;
	}
	if (scriptEnd == std::string::npos)
	{
		r.statusCode = 404;
		return r;
	}
	std::string	scriptName = urlPath.substr(0, scriptEnd);
	std::string	pathInfo = urlPath.substr(scriptEnd);

	struct stat	st;
	if (root.empty() || stat(root.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
	{
		r.statusCode = 500;
		return r;
	}

	std::string	base(root);
	while (base.size() > 1 && base[base.size() - 1] == '/')
		base.erase(base.size() - 1);
	std::string	scriptPath = (base == "/") ? scriptName : base + scriptName;

	if (stat(scriptPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
	{
		r.statusCode = 404;
		return r;
	}
	if (access(scriptPath.c_str(), R_OK) != 0)
	{
		r.statusCode = 403;
		return r;
	}

	size_t	lastSlash = scriptPath.rfind('/');
	r.scriptPath = scriptPath;
	r.scriptName = scriptName;
	r.pathInfo = pathInfo;
	r.scriptDir = (lastSlash == 0) ? "/" : scriptPath.substr(0, lastSlash);
	r.scriptFile = scriptPath.substr(lastSlash + 1);
	r.statusCode = 200;
	return r;
}
