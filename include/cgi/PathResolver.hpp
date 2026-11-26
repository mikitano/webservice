#ifndef PATHRESOLVER_HPP
#define PATHRESOLVER_HPP

#include <string>

struct PathResolution
{
	int			statusCode;
	std::string	scriptPath;
	std::string	scriptName;
	std::string	scriptFile;
	std::string	pathInfo;
	std::string	scriptDir;

	PathResolution() : statusCode(500) {}
};

class PathResolver
{
	public:
		static PathResolution resolve(const std::string& urlPath,
									  const std::string& root,
									  const std::string& cgiExtension);

	private:
		PathResolver();

		static bool	endsWith(const std::string& str, const std::string& suffix);
		static bool	hasDotDotSegment(const std::string& path);
};

#endif
